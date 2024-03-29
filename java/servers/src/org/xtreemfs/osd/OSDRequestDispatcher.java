/*
 * Copyright (c) 2008-2011 by Bjoern Kolbeck, Jan Stender,
 *               Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

package org.xtreemfs.osd;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.lang.management.OperatingSystemMXBean;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.AtomicReference;

import org.xtreemfs.common.HeartbeatThread;
import org.xtreemfs.common.ServiceAvailability;
import org.xtreemfs.common.HeartbeatThread.ServiceDataGenerator;
import org.xtreemfs.common.config.PolicyContainer;
import org.xtreemfs.common.monitoring.StatusMonitor;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.common.uuids.UUIDResolver;
import org.xtreemfs.common.uuids.UnknownUUIDException;
import org.xtreemfs.common.xloc.XLocations;
import org.xtreemfs.dir.discovery.DiscoveryUtils;
import org.xtreemfs.foundation.CrashReporter;
import org.xtreemfs.foundation.LifeCycleListener;
import org.xtreemfs.foundation.SSLOptions;
import org.xtreemfs.foundation.TimeServerClient;
import org.xtreemfs.foundation.TimeSync;
import org.xtreemfs.foundation.VersionManagement;
import org.xtreemfs.foundation.SSLOptions.TrustManager;
import org.xtreemfs.foundation.checksums.ChecksumFactory;
import org.xtreemfs.foundation.checksums.provider.JavaChecksumProvider;
import org.xtreemfs.foundation.logging.Logging;
import org.xtreemfs.foundation.logging.Logging.Category;
import org.xtreemfs.foundation.pbrpc.Schemes;
import org.xtreemfs.foundation.pbrpc.client.RPCAuthentication;
import org.xtreemfs.foundation.pbrpc.client.RPCNIOSocketClient;
import org.xtreemfs.foundation.pbrpc.client.RPCResponse;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.ErrorType;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.MessageType;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.POSIXErrno;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.RPCHeader;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.RPCHeader.ErrorResponse;
import org.xtreemfs.foundation.pbrpc.server.RPCNIOSocketServer;
import org.xtreemfs.foundation.pbrpc.server.RPCServerRequest;
import org.xtreemfs.foundation.pbrpc.server.RPCServerRequestListener;
import org.xtreemfs.foundation.pbrpc.server.RPCUDPSocketServer;
import org.xtreemfs.foundation.pbrpc.utils.ErrorUtils;
import org.xtreemfs.foundation.util.FSUtils;
import org.xtreemfs.foundation.util.OutputUtils;
import org.xtreemfs.osd.operations.CheckObjectOperation;
import org.xtreemfs.osd.operations.CleanupGetResultsOperation;
import org.xtreemfs.osd.operations.CleanupGetStatusOperation;
import org.xtreemfs.osd.operations.CleanupIsRunningOperation;
import org.xtreemfs.osd.operations.CleanupStartOperation;
import org.xtreemfs.osd.operations.CleanupStopOperation;
import org.xtreemfs.osd.operations.CleanupVersionsStartOperation;
import org.xtreemfs.osd.operations.DeleteOperation;
import org.xtreemfs.osd.operations.EventCloseFile;
import org.xtreemfs.osd.operations.EventCreateFileVersion;
import org.xtreemfs.osd.operations.EventInsertPaddingObject;
import org.xtreemfs.osd.operations.EventWriteObject;
import org.xtreemfs.osd.operations.FleaseMessageOperation;
import org.xtreemfs.osd.operations.GetFileIDListOperation;
import org.xtreemfs.osd.operations.GetObjectSetOperation;
import org.xtreemfs.osd.operations.InternalGetFileSizeOperation;
import org.xtreemfs.osd.operations.InternalGetGmaxOperation;
import org.xtreemfs.osd.operations.InternalRWRFetchOperation;
import org.xtreemfs.osd.operations.InternalRWRStatusOperation;
import org.xtreemfs.osd.operations.InternalRWRTruncateOperation;
import org.xtreemfs.osd.operations.InternalRWRUpdateOperation;
import org.xtreemfs.osd.operations.InternalTruncateOperation;
import org.xtreemfs.osd.operations.LocalReadOperation;
import org.xtreemfs.osd.operations.LockAcquireOperation;
import org.xtreemfs.osd.operations.LockCheckOperation;
import org.xtreemfs.osd.operations.LockReleaseOperation;
import org.xtreemfs.osd.operations.OSDOperation;
import org.xtreemfs.osd.operations.RWRNotifyOperation;
import org.xtreemfs.osd.operations.ReadOperation;
import org.xtreemfs.osd.operations.ShutdownOperation;
import org.xtreemfs.osd.operations.TruncateOperation;
import org.xtreemfs.osd.operations.VivaldiPingOperation;
import org.xtreemfs.osd.operations.WriteOperation;
import org.xtreemfs.osd.rwre.RWReplicationStage;
import org.xtreemfs.osd.stages.DeletionStage;
import org.xtreemfs.osd.stages.PreprocStage;
import org.xtreemfs.osd.stages.ReplicationStage;
import org.xtreemfs.osd.stages.StorageStage;
import org.xtreemfs.osd.stages.VivaldiStage;
import org.xtreemfs.osd.storage.CleanupThread;
import org.xtreemfs.osd.storage.CleanupVersionsThread;
import org.xtreemfs.osd.storage.HashStorageLayout;
import org.xtreemfs.osd.storage.MetadataCache;
import org.xtreemfs.osd.storage.StorageLayout;
import org.xtreemfs.osd.vivaldi.VivaldiNode;
import org.xtreemfs.pbrpc.generatedinterfaces.DIRServiceClient;
import org.xtreemfs.pbrpc.generatedinterfaces.MRCServiceClient;
import org.xtreemfs.pbrpc.generatedinterfaces.OSDServiceClient;
import org.xtreemfs.pbrpc.generatedinterfaces.OSDServiceConstants;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.DirService;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.Service;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.ServiceDataMap;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.ServiceSet;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.ServiceType;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.globalTimeSGetResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.KeyValuePair;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.VivaldiCoordinates;

import com.google.protobuf.Message;
import com.sun.net.httpserver.BasicAuthenticator;
import com.sun.net.httpserver.HttpContext;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.xtreemfs.dir.DIRClient;
import org.xtreemfs.osd.operations.EventRWRStatus;

public class OSDRequestDispatcher implements RPCServerRequestListener, LifeCycleListener {
    
    protected final Map<Integer, OSDOperation>          operations;
    
    protected final Map<Class<?>, OSDOperation>         internalEvents;
    
    protected final HeartbeatThread                     heartbeatThread;
    
    protected final OSDConfig                           config;
    
    protected final DIRClient                           dirClient;
    
    protected final MRCServiceClient                    mrcClient;
    
    protected final OSDServiceClient                    osdClient;
    
    protected final OSDServiceClient                    osdClientForReplication;
    
    protected final RPCNIOSocketClient                  rpcClientForReplication;
    
    protected final RPCNIOSocketClient                  rpcClient;
    
    protected final RPCNIOSocketServer                  rpcServer;
    
    protected long                                      requestId;
    
    protected String                                    authString;
    
    protected final PreprocStage                        preprocStage;
    
    protected final StorageStage                        stStage;
    
    protected final DeletionStage                       delStage;
    
    protected final ReplicationStage                    replStage;
    
    protected final RPCUDPSocketServer                  udpCom;
    
    protected final HttpServer                          httpServ;
    
    protected final long                                startupTime;
    
    protected final AtomicLong                          numBytesTX, numBytesRX, numObjsTX, numObjsRX,
            numReplBytesRX, numReplObjsRX;
    
    protected final VivaldiStage                        vStage;
    
    protected final AtomicReference<VivaldiCoordinates> myCoordinates;
    
    protected final CleanupThread                       cThread;
    
    protected final CleanupVersionsThread               cvThread;
    
    protected final RWReplicationStage                  rwrStage;
    
    private List<OSDStatusListener>                     statusListener;
    
    /**
     * reachability of services
     */
    private final ServiceAvailability                   serviceAvailability;
    
    public OSDRequestDispatcher(final OSDConfig config) throws Exception {
        
        Logging.logMessage(Logging.LEVEL_INFO, this, "XtreemFS OSD version "
            + VersionManagement.RELEASE_VERSION);
        
        this.config = config;
        assert (config.getUUID() != null);
        
        if (this.config.getDirectoryService().getHostName().equals(DiscoveryUtils.AUTODISCOVER_HOSTNAME)) {
            Logging.logMessage(Logging.LEVEL_INFO, Category.net, this,
                "trying to discover local XtreemFS DIR service...");
            DirService dir = DiscoveryUtils.discoverDir(10);
            if (dir == null) {
                Logging.logMessage(Logging.LEVEL_ERROR, Category.net, this,
                    "CANNOT FIND XtreemFS DIR service via discovery broadcasts... no response");
                throw new IOException("no DIR service found via discovery broadcast");
            }
            Logging.logMessage(Logging.LEVEL_INFO, Category.net, this, "found XtreemFS DIR service at "
                + dir.getAddress() + ":" + dir.getPort());
            config.setDirectoryService(new InetSocketAddress(dir.getAddress(), dir.getPort()));
        }
        
        numBytesTX = new AtomicLong();
        numBytesRX = new AtomicLong();
        numObjsTX = new AtomicLong();
        numObjsRX = new AtomicLong();
        numReplBytesRX = new AtomicLong();
        numReplObjsRX = new AtomicLong();
        
        // initialize the checksum factory
        ChecksumFactory.getInstance().addProvider(new JavaChecksumProvider());
        
        // ---------------------
        // initialize operations
        // ---------------------
        
        // IMPORTANT: the order of operations defined in
        // 'RequestDispatcher.Operations' has to be preserved!
        operations = new HashMap<Integer, OSDOperation>();
        internalEvents = new HashMap<Class<?>, OSDOperation>();
        initializeOperations();
        
        // create directory if necessary
        File objDir = new File(config.getObjDir());
        if (!objDir.exists()) {
            if (!objDir.mkdirs())
                throw new IOException("unable to create object directory: " + objDir.getAbsolutePath());
        }
        
        // -------------------------------
        // initialize communication stages
        // -------------------------------
        
        TrustManager tm1 = null;
        TrustManager tm2 = null;
        if (config.isUsingSSL()) {
            
            PolicyContainer policyContainer = new PolicyContainer(config);
            try {
                tm1 = policyContainer.getTrustManager();
                tm2 = policyContainer.getTrustManager();
            } catch (Exception e) {
                throw new IOException(e);
            }
            
            if (Logging.isInfo() && tm1 != null)
                Logging.logMessage(Logging.LEVEL_INFO, Category.misc, this,
                    "using custom trust manager '%s'", tm1.getClass().getName());
        }
        
        SSLOptions serverSSLopts = config.isUsingSSL() ? new SSLOptions(new FileInputStream(config
                .getServiceCredsFile()), config.getServiceCredsPassphrase(), config
                .getServiceCredsContainer(), new FileInputStream(config.getTrustedCertsFile()), config
                .getTrustedCertsPassphrase(), config.getTrustedCertsContainer(), false, config
                .isGRIDSSLmode(), tm1) : null;
        
        rpcServer = new RPCNIOSocketServer(config.getPort(), config.getAddress(), this, serverSSLopts,
                config.getSocketReceiveBufferSize(), config.getMaxClientQ());
        rpcServer.setLifeCycleListener(this);
        
        final SSLOptions clientSSLopts = config.isUsingSSL() ? new SSLOptions(new FileInputStream(config
                .getServiceCredsFile()), config.getServiceCredsPassphrase(), config
                .getServiceCredsContainer(), new FileInputStream(config.getTrustedCertsFile()), config
                .getTrustedCertsPassphrase(), config.getTrustedCertsContainer(), false, config
                .isGRIDSSLmode(), tm2) : null;
        
        rpcClient = new RPCNIOSocketClient(clientSSLopts, 5000, 5 * 60 * 1000, config
                .getSocketSendBufferSize(), config.getSocketReceiveBufferSize());
        rpcClient.setLifeCycleListener(this);
        
        // replication uses its own RPCClient with a much higher timeout
        rpcClientForReplication = new RPCNIOSocketClient(clientSSLopts, 30000, 5 * 60 * 1000);
        rpcClientForReplication.setLifeCycleListener(this);
        
        // initialize ServiceAvailability
        this.serviceAvailability = new ServiceAvailability();
        
        // --------------------------
        // initialize internal stages
        // --------------------------
        
        MetadataCache metadataCache = new MetadataCache();
        StorageLayout storageLayout = null;
        if (config.getStorageLayout().equalsIgnoreCase(HashStorageLayout.class.getSimpleName())) {
            storageLayout = new HashStorageLayout(config, metadataCache);
            /*
             * } else if
             * (config.getStorageLayout().equalsIgnoreCase(SingleFileStorageLayout
             * .class.getSimpleName())) { storageLayout = new
             * SingleFileStorageLayout(config, metadataCache);
             */
        } else {
            throw new RuntimeException("unknown storage layout in config file: " + config.getStorageLayout());
        }
        
        udpCom = new RPCUDPSocketServer(config.getPort(), this);
        udpCom.setLifeCycleListener(this);
        
        preprocStage = new PreprocStage(this, metadataCache);
        preprocStage.setLifeCycleListener(this);
        
        stStage = new StorageStage(this, metadataCache, storageLayout, 1);
        stStage.setLifeCycleListener(this);
        
        delStage = new DeletionStage(this, metadataCache, storageLayout);
        delStage.setLifeCycleListener(this);
        
        replStage = new ReplicationStage(this);
        replStage.setLifeCycleListener(this);
        
        rwrStage = new RWReplicationStage(this, serverSSLopts);
        rwrStage.setLifeCycleListener(this);
        
        // ----------------------------------------
        // initialize TimeSync and Heartbeat thread
        // ----------------------------------------
        
        DIRServiceClient dirRpcClient = new DIRServiceClient(rpcClient, config.getDirectoryService());
        dirClient = new DIRClient(dirRpcClient, config.getDirectoryServices(), config.getFailoverMaxRetries(), config.getFailoverWait());
        mrcClient = new MRCServiceClient(rpcClient, null);
        osdClient = new OSDServiceClient(rpcClient, null);
        osdClientForReplication = new OSDServiceClient(rpcClientForReplication, null);

        TimeSync.initialize(dirClient, config.getRemoteTimeSync(), config.getLocalClockRenew());
        UUIDResolver.start(dirClient, 10 * 1000, 600 * 1000);
        UUIDResolver.addLocalMapping(config.getUUID(), config.getPort(), Schemes.getScheme(config
                .isUsingSSL(), config.isGRIDSSLmode()));
        UUIDResolver.addLocalMapping(config.getUUID(), config.getPort(), Schemes.SCHEME_PBRPCU);
        
        myCoordinates = new AtomicReference<VivaldiCoordinates>();
        
        ServiceDataGenerator gen = new ServiceDataGenerator() {
            public ServiceSet getServiceData() {
                
                OSDConfig config = OSDRequestDispatcher.this.config;
                String freeSpace = "0";
                
                if (config.isReportFreeSpace()) {
                    freeSpace = String.valueOf(FSUtils.getFreeSpace(config.getObjDir()));
                }
                
                String totalSpace = "-1";
                
                try {
                    File f = new File(config.getObjDir());
                    totalSpace = String.valueOf(f.getTotalSpace());
                } catch (Exception ex) {
                }
                
                OperatingSystemMXBean osb = ManagementFactory.getOperatingSystemMXBean();
                String load = String.valueOf((int) (osb.getSystemLoadAverage() * 100 / osb
                        .getAvailableProcessors()));
                
                long totalRAM = Runtime.getRuntime().maxMemory();
                long usedRAM = Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory();
                
                ServiceSet.Builder data = ServiceSet.newBuilder();
                
                ServiceDataMap.Builder dmap = ServiceDataMap.newBuilder();
                dmap.addData(KeyValuePair.newBuilder().setKey("load").setValue(load).build());
                dmap.addData(KeyValuePair.newBuilder().setKey("total").setValue(totalSpace).build());
                dmap.addData(KeyValuePair.newBuilder().setKey("free").setValue(freeSpace).build());
                dmap.addData(KeyValuePair.newBuilder().setKey("totalRAM").setValue(Long.toString(totalRAM))
                        .build());
                dmap.addData(KeyValuePair.newBuilder().setKey("usedRAM").setValue(Long.toString(usedRAM))
                        .build());
                dmap.addData(KeyValuePair.newBuilder().setKey("geoCoordinates").setValue(
                    config.getGeoCoordinates()).build());
                dmap.addData(KeyValuePair.newBuilder().setKey("proto_version").setValue(
                    Integer.toString(OSDServiceConstants.INTERFACE_ID)).build());
                VivaldiCoordinates coord = myCoordinates.get();
                if (coord != null) {
                    dmap.addData(KeyValuePair.newBuilder().setKey("vivaldi_coordinates").setValue(
                        VivaldiNode.coordinatesToString(coord)).build());
                }
                for (Entry<String, String> e : config.getCustomParams().entrySet())
                    dmap.addData(KeyValuePair.newBuilder().setKey(e.getKey()).setValue(e.getValue()));
                
                try {
                    final String address = "".equals(config.getHostName()) ? config.getAddress() == null ? config
                            .getUUID().getMappings()[0].resolvedAddr.getAddress().getHostAddress()
                        : config.getAddress().getHostAddress()
                        : config.getHostName();
                    dmap.addData(KeyValuePair.newBuilder().setKey("status_page_url").setValue(
                        "http://" + address + ":" + config.getHttpPort()));
                } catch (UnknownUUIDException ex) {
                    // should never happen
                }
                Service.Builder me = Service.newBuilder();
                me.setType(ServiceType.SERVICE_TYPE_OSD);
                me.setUuid(config.getUUID().toString());
                me.setName("OSD @ " + config.getUUID());
                me.setVersion(0);
                me.setData(dmap);
                me.setLastUpdatedS(0);
                
                data.addServices(me);
                
                return data.build();
            }
        };
        heartbeatThread = new HeartbeatThread("OSD HB Thr", dirClient, config.getUUID(), gen, config, true);
        
        httpServ = HttpServer.create(new InetSocketAddress(config.getHttpPort()), 0);
        final HttpContext ctx = httpServ.createContext("/", new HttpHandler() {
            public void handle(HttpExchange httpExchange) throws IOException {
                byte[] content;
                try {
                    if (httpExchange.getRequestURI().getPath().contains("strace")) {
                        content = OutputUtils.getThreadDump().getBytes("ascii");
                    } else if (httpExchange.getRequestURI().getPath().contains("rft")) {
                        final StringBuffer sb = new StringBuffer();
                        final AtomicReference<Map<String, Map<String, String>>> result = new AtomicReference<Map<String, Map<String, String>>>();
                        sb.append("<HTML><HEAD><TITLE>Replicated File Status List</TITLE>");
                        sb.append("<STYLE type=\"text/css\">body,table,tr,td,h1 {font-family:Arial,Helvetica,sans-serif;}</STYLE></HEAD><BODY>");
                        sb.append("<H1>List of Open Replicated Files</H1>");
                        sb.append("<TABLE border=\"1\">");
                        sb.append("<TR><TD><B>File ID</B></TD><TD><B>Status</B></TD></TR>");
                        rwrStage.getStatus(new RWReplicationStage.StatusCallback() {
                            
                            @Override
                            public void statusComplete(Map<String, Map<String, String>> status) {
                                synchronized (result) {
                                    result.set(status);
                                    result.notifyAll();
                                }
                            }
                        });
                        synchronized (result) {
                            if (result.get() == null)
                                result.wait();
                        }
                        Map<String, Map<String, String>> status = result.get();
                        for (String fileId : status.keySet()) {
                            sb.append("<TR><TD>");
                            sb.append(fileId);
                            final String role = status.get(fileId).get("role");
                            String bgcolor = "#FFFFFF";
                            if (role != null && role.equals("primary")) {
                                bgcolor = "#A3FFA3";
                            } else if (role != null && role.startsWith("backup")) {
                                bgcolor = "#FFFF66";
                            }
                            sb.append("</TD><TD style=\"background-color:"+bgcolor+"\"><TABLE border=\"0\">");
                            for (Entry<String, String> e : status.get(fileId).entrySet()) {
                                sb.append("<TR><TD>");
                                sb.append(e.getKey());
                                sb.append("</TD><TD>");
                                sb.append(e.getValue());
                                sb.append("</TD></TR>\n");
                            }
                            sb.append("</TABLE></TD></TR>\n");
                        }
                        content = sb.toString().getBytes("ascii");
                    } else {
                        content = StatusPage.getStatusPage(OSDRequestDispatcher.this).getBytes("ascii");
                    }
                    httpExchange.getResponseHeaders().add("Content-Type", "text/html; charset=UTF-8");
                    httpExchange.sendResponseHeaders(200, content.length);
                    httpExchange.getResponseBody().write(content);
                    httpExchange.getResponseBody().close();
                } catch (Throwable ex) {
                    ex.printStackTrace();
                    httpExchange.sendResponseHeaders(500, 0);
                }
                
            }
        });
        
        if (config.getAdminPassword().length() > 0) {
            ctx.setAuthenticator(new BasicAuthenticator("XtreemFS OSD " + config.getUUID()) {
                @Override
                public boolean checkCredentials(String arg0, String arg1) {
                    return (arg0.equals("admin") && arg1.equals(config.getAdminPassword()));
                }
            });
        }
        
        httpServ.start();
        
        startupTime = System.currentTimeMillis();
        
        vStage = new VivaldiStage(this);
        vStage.setLifeCycleListener(this);
        
        cThread = new CleanupThread(this, storageLayout);
        cThread.setLifeCycleListener(this);
        
        cvThread = new CleanupVersionsThread(this, storageLayout);
        cvThread.setLifeCycleListener(this);
        
        statusListener = new ArrayList<OSDStatusListener>();
        if (config.isUsingSnmp()) {
            statusListener.add(new StatusMonitor(
                    this, config.getSnmpAddress(), config.getSnmpPort(), config.getSnmpACLFile()));
            
            notifyConfigurationChange();
        }
        
        
        
        if (Logging.isDebug())
            Logging.logMessage(Logging.LEVEL_DEBUG, Category.lifecycle, this, "OSD at %s ready", this
                    .getConfig().getUUID().toString());
    }
    
    public CleanupThread getCleanupThread() {
        return cThread;
    }
    
    public CleanupVersionsThread getCleanupVersionsThread() {
        return cvThread;
    }
    
    public void start() {
        
        try {
            
            rpcServer.start();
            rpcClient.start();
            rpcClientForReplication.start();
            
            rpcServer.waitForStartup();
            rpcClient.waitForStartup();
            
            udpCom.start();
            preprocStage.start();
            delStage.start();
            stStage.start();
            replStage.start();
            vStage.start();
            cThread.start();
            cvThread.start();
            rwrStage.start();
            
            udpCom.waitForStartup();
            preprocStage.waitForStartup();
            delStage.waitForStartup();
            stStage.waitForStartup();
            vStage.waitForStartup();
            cThread.waitForStartup();
            cvThread.waitForStartup();
            rwrStage.waitForStartup();
            
            heartbeatThread.initialize();
            heartbeatThread.start();
            heartbeatThread.waitForStartup();
            
            if (Logging.isInfo())
                Logging.logMessage(Logging.LEVEL_INFO, Category.lifecycle, this,
                    "OSD RequestController and all services operational");
            
        } catch (Exception ex) {
            Logging.logMessage(Logging.LEVEL_ERROR, this, "STARTUP FAILED!");
            Logging.logError(Logging.LEVEL_ERROR, this, ex);
            System.exit(1);
        }
        
    }
    
    public void shutdown() {
        
        try {
            
        	for (OSDStatusListener listener : statusListener) {
				listener.shuttingDown();
			}
        	
            heartbeatThread.shutdown();
            heartbeatThread.waitForShutdown();
            
            UUIDResolver.shutdown();
            
            rpcServer.shutdown();
            rpcClient.shutdown();
            rpcClientForReplication.shutdown();
            
            rpcServer.waitForShutdown();
            rpcClient.waitForShutdown();
            rpcClientForReplication.waitForShutdown();
            
            serviceAvailability.shutdown();
            
            udpCom.shutdown();
            preprocStage.shutdown();
            delStage.shutdown();
            stStage.shutdown();
            replStage.shutdown();
            rwrStage.shutdown();
            vStage.shutdown();
            cThread.cleanupStop();
            cThread.shutdown();
            cvThread.cleanupStop();
            cvThread.shutdown();
            serviceAvailability.shutdown();
            
            udpCom.waitForShutdown();
            preprocStage.waitForShutdown();
            delStage.waitForShutdown();
            stStage.waitForShutdown();
            replStage.waitForShutdown();
            rwrStage.waitForShutdown();
            vStage.waitForShutdown();
            cThread.waitForShutdown();
            cvThread.waitForShutdown();
            
            httpServ.stop(0);
            
            if (Logging.isInfo())
                Logging.logMessage(Logging.LEVEL_INFO, Category.lifecycle, this,
                    "OSD and all stages terminated");
            
        } catch (Exception ex) {
            Logging.logMessage(Logging.LEVEL_ERROR, this, "shutdown failed");
            Logging.logError(Logging.LEVEL_ERROR, this, ex);
        }
    }
    
    public void asyncShutdown() {
        try {
            
            for (OSDStatusListener listener : statusListener) {
                listener.shuttingDown();
            }
            
            heartbeatThread.shutdown();
            
            UUIDResolver.shutdown();
            
            rpcServer.shutdown();
            rpcClient.shutdown();
            rpcClientForReplication.shutdown();
            
            udpCom.shutdown();
            preprocStage.shutdown();
            delStage.shutdown();
            stStage.shutdown();
            replStage.shutdown();
            rwrStage.shutdown();
            vStage.shutdown();
            cThread.cleanupStop();
            cThread.shutdown();
            cvThread.cleanupStop();
            cvThread.shutdown();
            serviceAvailability.shutdown();
            
            httpServ.stop(0);
            
            if (Logging.isInfo())
                Logging.logMessage(Logging.LEVEL_INFO, Category.lifecycle, this, "OSD and all stages terminated");
            
        } catch (Exception ex) {
            Logging.logMessage(Logging.LEVEL_ERROR, this, "shutdown failed");
            Logging.logError(Logging.LEVEL_ERROR, this, ex);
        }
    }
    
    public OSDOperation getOperation(int procId) {
        return operations.get(procId);
    }
    
    public OSDOperation getInternalEvent(Class<?> clazz) {
        return internalEvents.get(clazz);
    }
    
    public OSDConfig getConfig() {
        return config;
    }
    
    public DIRClient getDIRClient() {
        return dirClient;
    }
    
    public MRCServiceClient getMRCClient() {
        return mrcClient;
    }
    
    public OSDServiceClient getOSDClient() {
        return osdClient;
    }
    
    public OSDServiceClient getOSDClientForReplication() {
        return osdClientForReplication;
    }
    
    public RPCNIOSocketClient getRPCClient() {
        return rpcClient;
    }
    
    public void startupPerformed() {
        
    }
    
    public void shutdownPerformed() {
        
    }
    
    public void crashPerformed(Throwable cause) {
        final String report = CrashReporter
                .createCrashReport("OSD", VersionManagement.RELEASE_VERSION, cause);
        System.out.println(report);
        CrashReporter.reportXtreemFSCrash(report);
        this.shutdown();
    }
    
    /**
     * Checks if the local OSD is the head OSD in one of the given X-Locations
     * list.
     * 
     * @param xloc
     *            the X-Locations list
     * @return <texttt>true</texttt>, if the local OSD is the head OSD of the
     *         given X-Locations list; <texttt>false</texttt>, otherwise
     */
    public boolean isHeadOSD(XLocations xloc) {
        final ServiceUUID headOSD = xloc.getLocalReplica().getOSDs().get(0);
        return config.getUUID().equals(headOSD);
    }
    
    public long getFreeSpace() {
        return FSUtils.getFreeSpace(config.getObjDir());
    }
    
    public long getTotalSpace() {
        File f = new File(config.getObjDir());
        long s = f.getTotalSpace();
        return s;
    }
    
    @Override
    public void receiveRecord(RPCServerRequest rq) {
        
        // final ONCRPCRequestHeader hdr = rq.getRequestHeader();
        RPCHeader hdr = rq.getHeader();
        
        if (hdr.getMessageType() != MessageType.RPC_REQUEST) {
            rq.sendError(ErrorType.GARBAGE_ARGS, POSIXErrno.POSIX_ERROR_EIO,
                "expected RPC request message type but got " + hdr.getMessageType());
            return;
        }
        
        final RPCHeader.RequestHeader rqHdr = hdr.getRequestHeader();
        
        if (rqHdr.getInterfaceId() != OSDServiceConstants.INTERFACE_ID) {
            rq.sendError(ErrorType.INVALID_INTERFACE_ID, POSIXErrno.POSIX_ERROR_EIO,
                "invalid interface id. Maybe wrong service address/port configured?");
            return;
        }
        
        try {
            OSDRequest request = new OSDRequest(rq);
            if (Logging.isDebug())
                Logging.logMessage(Logging.LEVEL_DEBUG, Category.stage, this, "received new request: %s", rq
                        .toString());
            preprocStage.prepareRequest(request, new PreprocStage.ParseCompleteCallback() {
                
                @Override
                public void parseComplete(OSDRequest result, ErrorResponse error) {
                    if (error == null) {
                        result.getOperation().startRequest(result);
                    } else {
                        result.getRPCRequest().sendError(error);
                    }
                }
            });
        } catch (Exception ex) {
            rq.sendError(ErrorUtils.getInternalServerError(ex));
            Logging.logError(Logging.LEVEL_ERROR, this, ex);
        }
    }
    
    public int getNumClientConnections() {
        return rpcServer.getNumConnections();
    }
    
    public long getPendingRequests() {
        return rpcServer.getPendingRequests();
    }
    
    private void initializeOperations() {
        // register all ops
        OSDOperation op = new ReadOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new WriteOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new DeleteOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new TruncateOperation(this);
        operations.put(op.getProcedureId(), op);
        
        /*
         * op = new KeepFileOpenOperation(this);
         * operations.put(op.getProcedureId(),op);
         */

        op = new InternalGetGmaxOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new InternalTruncateOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new CheckObjectOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new InternalGetFileSizeOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new ShutdownOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new LocalReadOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new CleanupStartOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new CleanupIsRunningOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new CleanupStopOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new CleanupGetStatusOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new CleanupGetResultsOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new CleanupVersionsStartOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new GetObjectSetOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new LockAcquireOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new LockCheckOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new LockReleaseOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new VivaldiPingOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new FleaseMessageOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new InternalRWRUpdateOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new InternalRWRTruncateOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new InternalRWRStatusOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new InternalRWRFetchOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new GetFileIDListOperation(this);
        operations.put(op.getProcedureId(), op);
        
        op = new RWRNotifyOperation(this);
        operations.put(op.getProcedureId(), op);
        
        // --internal events here--
        
        op = new EventCloseFile(this);
        internalEvents.put(EventCloseFile.class, op);
        
        op = new EventCreateFileVersion(this);
        internalEvents.put(EventCreateFileVersion.class, op);
        
        op = new EventWriteObject(this);
        internalEvents.put(EventWriteObject.class, op);
        
        op = new EventInsertPaddingObject(this);
        internalEvents.put(EventInsertPaddingObject.class, op);
        
        op = new EventRWRStatus(this);
        internalEvents.put(EventRWRStatus.class, op);

    }
    
    public StorageStage getStorageStage() {
        return this.stStage;
    }
    
    public DeletionStage getDeletionStage() {
        return delStage;
    }
    
    public PreprocStage getPreprocStage() {
        return preprocStage;
    }
    
    public ReplicationStage getReplicationStage() {
        return replStage;
    }
    
    public void sendUDPMessage(RPCHeader header, Message message, InetSocketAddress receiver)
        throws IOException {
        udpCom.sendRequest(header, message, receiver);
    }
    
    public VivaldiStage getVivaldiStage() {
        return this.vStage;
    }
    
    public RWReplicationStage getRWReplicationStage() {
        return this.rwrStage;
    }
    
    // FIXME: implement operations for Gmax, Ping
    /*
     * @Override public void receiveUDP(UDPMessage msg) { assert
     * (msg.isRequest() || msg.isResponse());
     * 
     * try {
     * 
     * if (msg.isRequest()) { if (msg.getRequestData() instanceof
     * xtreemfs_broadcast_gmaxRequest) { xtreemfs_broadcast_gmaxRequest rq =
     * (xtreemfs_broadcast_gmaxRequest) msg.getRequestData(); if
     * (Logging.isDebug()) Logging.logMessage(Logging.LEVEL_DEBUG,
     * Category.stage, this, "received GMAX packet for: %s from %s",
     * rq.getFile_id(), msg.getAddress());
     * 
     * BufferPool.free(msg.getPayload());
     * stStage.receivedGMAX_ASYNC(rq.getFile_id(), rq.getTruncate_epoch(),
     * rq.getLast_object()); } else if (msg.getRequestData() instanceof
     * xtreemfs_pingRequest) { xtreemfs_pingRequest rq = (xtreemfs_pingRequest)
     * msg.getRequestData(); if (Logging.isDebug())
     * Logging.logMessage(Logging.LEVEL_DEBUG, Category.stage, this,
     * "received ping request from: %s", msg.getAddress());
     * 
     * vStage.receiveVivaldiMessage(msg); } } else { if (msg.getResponseData()
     * instanceof xtreemfs_pingResponse) { xtreemfs_pingResponse resp =
     * (xtreemfs_pingResponse) msg.getResponseData(); if (Logging.isDebug())
     * Logging.logMessage(Logging.LEVEL_DEBUG, Category.stage, this,
     * "received ping response from: %s", msg.getAddress());
     * 
     * vStage.receiveVivaldiMessage(msg); } } } catch (Exception ex) {
     * Logging.logError(Logging.LEVEL_DEBUG, this,ex); } }
     */

    /**
     * @return the serviceAvailability
     */
    public ServiceAvailability getServiceAvailability() {
        return serviceAvailability;
    }
    
    public void objectReceived() {
        long num = numObjsRX.incrementAndGet();
        for (OSDStatusListener listener : statusListener) {
            listener.numObjsRXChanged(num);
        }
    }
    
    public void objectReplicated() {
        long num = numReplObjsRX.incrementAndGet();
        for (OSDStatusListener listener : statusListener) {
            listener.numReplObjsRX(num);
        }
    }
    
    public void objectSent() {
        long num = numObjsTX.incrementAndGet();
        for (OSDStatusListener listener : statusListener) {
            listener.numObjsTXChanged(num);
        }
    }
    
    public void replicatedDataReceived(int numBytes) {
        long num = numReplBytesRX.addAndGet(numBytes);
        for (OSDStatusListener listener : statusListener) {
            listener.numReplBytesRXChanged(num);
        }
    }
    
    public void dataReceived(int numBytes) {
        long num = numBytesRX.addAndGet(numBytes);
        for (OSDStatusListener listener : statusListener) {
            listener.numBytesRXChanged(num);
        }
    }
    
    public void dataSent(int numBytes) {
        long num = numBytesTX.addAndGet(numBytes);
        for (OSDStatusListener listener : statusListener) {
            listener.numBytesTXChanged(num);
        }
    }
    
    public long getObjectsReceived() {
        return numObjsRX.get();
    }
    
    public long getObjectsSent() {
        return numObjsTX.get();
    }
    
    public long getBytesReceived() {
        return numBytesRX.get();
    }
    
    public long getBytesSent() {
        return numBytesTX.get();
    }
    
    public long getReplicatedObjectsReceived() {
        return numReplObjsRX.get();
    }
    
    public long getReplicatedBytesReceived() {
        return numReplBytesRX.get();
    }
    
    public void updateVivaldiCoordinates(VivaldiCoordinates newVC) {
        myCoordinates.set(newVC);
    }
    
    public String getHostName() {
        return heartbeatThread.getAdvertisedHostName();
    }
    
    public void addStatusListener(OSDStatusListener listener) {
        this.statusListener.add(listener);
    }

    public void removeStatusListener(OSDStatusListener listener) {
        this.statusListener.remove(listener);
    }
    
    /**
     * Tells all listeners when the configuration has changed.
     */
    public void notifyConfigurationChange() {
        for (OSDStatusListener listener : statusListener) {
            listener.OSDConfigChanged(this.config);
        }
    }
    
    
    /**
     * Getter for a timestamp when the heartbeatthread sent his last heartbeat
     * @return long - timestamp as returned by System.currentTimeMillis()
     */
    public long getLastHeartbeat() {
        return heartbeatThread.getLastHeartbeat();
    }
    
}
