/*  Copyright (c) 2009 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

    This file is part of XtreemFS. XtreemFS is part of XtreemOS, a Linux-based
    Grid Operating System, see <http://www.xtreemos.eu> for more details.
    The XtreemOS project has been developed with the financial support of the
    European Commission's IST program under contract #FP6-033576.

    XtreemFS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 2 of the License, or (at your option)
    any later version.

    XtreemFS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with XtreemFS. If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * AUTHORS: Björn Kolbeck (ZIB)
 */
package org.xtreemfs.dir;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import org.xtreemfs.babudb.BabuDB;
import org.xtreemfs.babudb.BabuDBException;
import org.xtreemfs.babudb.BabuDBFactory;
import org.xtreemfs.babudb.log.DiskLogger.SyncMode;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.logging.Logging.Category;
import org.xtreemfs.dir.operations.DIROperation;
import org.xtreemfs.dir.operations.DeleteAddressMappingOperation;
import org.xtreemfs.dir.operations.DeregisterServiceOperation;
import org.xtreemfs.dir.operations.GetAddressMappingOperation;
import org.xtreemfs.dir.operations.GetGlobalTimeOperation;
import org.xtreemfs.dir.operations.GetServiceByNameOperation;
import org.xtreemfs.dir.operations.GetServiceByUuidOperation;
import org.xtreemfs.dir.operations.GetServicesByTypeOperation;
import org.xtreemfs.dir.operations.RegisterServiceOperation;
import org.xtreemfs.dir.operations.ServiceOfflineOperation;
import org.xtreemfs.dir.operations.SetAddressMappingOperation;
import org.xtreemfs.foundation.ErrNo;
import org.xtreemfs.foundation.LifeCycleListener;
import org.xtreemfs.foundation.LifeCycleThread;
import org.xtreemfs.foundation.SSLOptions;
import org.xtreemfs.foundation.oncrpc.server.ONCRPCRequest;
import org.xtreemfs.foundation.oncrpc.server.RPCNIOSocketServer;
import org.xtreemfs.foundation.oncrpc.server.RPCServerRequestListener;
import org.xtreemfs.interfaces.DIRInterface.DIRInterface;
import org.xtreemfs.interfaces.Exceptions.ProtocolException;
import org.xtreemfs.interfaces.utils.ONCRPCRequestHeader;
import org.xtreemfs.interfaces.utils.ONCRPCResponseHeader;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.xtreemfs.foundation.CrashReporter;

/**
 * 
 * @author bjko
 */
public class DIRRequestDispatcher extends LifeCycleThread implements RPCServerRequestListener,
    LifeCycleListener {
    
    public final static String                 VERSION           = "1.0.0 (v1.0 RC1)";
    
    /**
     * index for address mappings, stores uuid -> AddressMappingSet
     */
    public static final int                    INDEX_ID_ADDRMAPS = 0;
    
    /**
     * index for service registries, stores uuid -> ServiceRegistry
     */
    public static final int                    INDEX_ID_SERVREG  = 1;
    
    private final HttpServer                   httpServ;
    
    private int                                numRequests;
    
    private final Map<Integer, DIROperation>   registry;
    
    private final RPCNIOSocketServer           server;
    
    private final BlockingQueue<ONCRPCRequest> queue;
    
    private volatile boolean                   quit;
    
    private final BabuDB                       database;
    
    public static final String                 DB_NAME           = "dirdb";
    
    public DIRRequestDispatcher(final DIRConfig config) throws IOException, BabuDBException {
        super("DIR RqDisp");
        registry = new HashMap();
        
        // start up babudb
        database = BabuDBFactory.getBabuDB(config.getDbDir(), config.getDbDir(), 0, 1024 * 1024 * 16, 60 * 5,
            SyncMode.FSYNC, 200, 500);
        
        initializeDatabase();
        
        registerOperations();
        
        // start the server
        
        SSLOptions sslOptions = null;
        if (config.isUsingSSL()) {
            sslOptions = new SSLOptions(new FileInputStream(config.getServiceCredsFile()), config
                    .getServiceCredsPassphrase(), config.getServiceCredsContainer(), new FileInputStream(
                config.getTrustedCertsFile()), config.getTrustedCertsPassphrase(), config
                    .getTrustedCertsContainer(), false);
        }
        
        queue = new LinkedBlockingQueue();
        quit = false;
        
        server = new RPCNIOSocketServer(config.getPort(), null, this, sslOptions);
        
        httpServ = HttpServer.create(new InetSocketAddress(config.getHttpPort()), 0);
        httpServ.createContext("/", new HttpHandler() {
            public void handle(HttpExchange httpExchange) throws IOException {
                byte[] content;
                try {
                    content = StatusPage.getStatusPage(DIRRequestDispatcher.this, config).getBytes("ascii");
                    httpExchange.sendResponseHeaders(200, content.length);
                    httpExchange.getResponseBody().write(content);
                    httpExchange.getResponseBody().close();
                } catch (BabuDBException ex) {
                    ex.printStackTrace();
                    httpExchange.sendResponseHeaders(500, 0);
                }
                
            }
        });
        httpServ.start();
        
        numRequests = 0;
    }
    
    @Override
    public void run() {
        try {
            notifyStarted();
            while (!quit) {
                final ONCRPCRequest rq = queue.take();
                processRequest(rq);
            }
        } catch (InterruptedException ex) {
            quit = true;
        } catch (Exception ex) {
            notifyCrashed(ex);
        }
        notifyStopped();
    }
    
    public void startup() throws Exception {
        this.start();
        
        server.start();
        server.waitForStartup();
    }
    
    public void shutdown() throws Exception {
        httpServ.stop(0);
        server.shutdown();
        server.waitForShutdown();
        database.shutdown();
        
        this.quit = true;
        this.interrupt();
        this.waitForShutdown();
    }
    
    private void initializeDatabase() {
        try {
            database.createDatabase("dirdb", 3);
        } catch (BabuDBException ex) {
            // database already created
        }
        
    }
    
    private void registerOperations() {
        
        DIROperation op;
        op = new GetGlobalTimeOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new GetAddressMappingOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new SetAddressMappingOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new DeleteAddressMappingOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new RegisterServiceOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new DeregisterServiceOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new GetServiceByUuidOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new GetServicesByTypeOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new GetServiceByNameOperation(this);
        registry.put(op.getProcedureId(), op);
        
        op = new ServiceOfflineOperation(this);
        registry.put(op.getProcedureId(), op);
    }
    
    public BabuDB getDatabase() {
        return this.database;
    }
    
    @Override
    public void receiveRecord(ONCRPCRequest rq) {
        if (Logging.isDebug())
            Logging.logMessage(Logging.LEVEL_DEBUG, Category.stage, this, "received new request: %s", rq
                    .toString());
        this.queue.add(rq);
    }
    
    public void processRequest(ONCRPCRequest rq) {
        final ONCRPCRequestHeader hdr = rq.getRequestHeader();
        
        if (hdr.getInterfaceVersion() != DIRInterface.getVersion()) {
            rq.sendProtocolException(new ProtocolException(ONCRPCResponseHeader.ACCEPT_STAT_PROG_MISMATCH,
                ErrNo.EINVAL, "invalid version requested"));
            return;
        }
        
        // everything ok, find the right operation
        DIROperation op = registry.get(hdr.getOperationNumber());
        if (op == null) {
            rq.sendProtocolException(new ProtocolException(ONCRPCResponseHeader.ACCEPT_STAT_PROC_UNAVAIL,
                ErrNo.EINVAL, "requested operation is not available on this DIR"));
            return;
        }
        
        DIRRequest dirRq = new DIRRequest(rq);
        try {
            op.parseRPCMessage(dirRq);
        } catch (Throwable ex) {
            ex.printStackTrace();
            rq.sendGarbageArgs(ex.toString());
            return;
        }
        try {
            numRequests++;
            op.startRequest(dirRq);
        } catch (Throwable ex) {
            ex.printStackTrace();
            rq.sendInternalServerError(ex);
            return;
        }
    }
    
    @Override
    public void startupPerformed() {
    }
    
    @Override
    public void shutdownPerformed() {
    }
    
    @Override
    public void crashPerformed(Throwable cause) {
        CrashReporter.reportXtreemFSCrash("DIR", this.VERSION, cause);
        try {
            shutdown();
        } catch (Exception e) {
            Logging.logError(Logging.LEVEL_ERROR, this, e);
        }
    }
    
    public long getNumRequests() {
        return server.getPendingRequests();
    }
    
    public int getNumConnections() {
        return server.getNumConnections();
    }
    
}