/*  Copyright (c) 2008 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin,
 Barcelona Supercomputing Center - Centro Nacional de Supercomputacion
 and Consiglio Nazionale delle Ricerche.

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
 * AUTHORS: Jan Stender (ZIB), Björn Kolbeck (ZIB), Jesús Malo (BSC)
 *          Eugenio Cesario (CNR)
 */

package org.xtreemfs.common;

import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.xtreemfs.common.config.ServiceConfig;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.logging.Logging.Category;
import org.xtreemfs.common.util.NetUtils;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.dir.client.DIRClient;
import org.xtreemfs.foundation.LifeCycleThread;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.interfaces.AddressMapping;
import org.xtreemfs.interfaces.AddressMappingSet;
import org.xtreemfs.interfaces.Constants;
import org.xtreemfs.interfaces.Service;
import org.xtreemfs.interfaces.ServiceSet;
import org.xtreemfs.interfaces.utils.ONCRPCException;

/**
 * A thread that regularly sends a heartbeat signal with fresh service data to
 * the Directory Service.
 */
public class HeartbeatThread extends LifeCycleThread {
    
    /**
     * An interface that generates service data to be sent to the Directory
     * Service. Each time a heartbeat signal is sent, new service data will be
     * generated by means of invoking <tt>getServiceData()</tt>.
     */
    public interface ServiceDataGenerator {
        public ServiceSet getServiceData();
    }
    
    private static final long    UPDATE_INTERVAL = 60 * 1000; // 60s
                                                              
    private ServiceUUID          uuid;
    
    private ServiceDataGenerator serviceDataGen;
    
    private DIRClient            client;
    
    private volatile boolean     quit;
    
    private final ServiceConfig  config;
    
    private final boolean        advertiseUDPEndpoints;
    
    public HeartbeatThread(String name, DIRClient client, ServiceUUID uuid,
        ServiceDataGenerator serviceDataGen, ServiceConfig config, boolean advertiseUDPEndpoints) {
        
        super(name);
        
        this.client = client;
        this.uuid = uuid;
        this.serviceDataGen = serviceDataGen;
        this.config = config;
        this.advertiseUDPEndpoints = advertiseUDPEndpoints;
    }
    
    public synchronized void shutdown() {
        RPCResponse<String> r = null;
        try {
            if (client.clientIsAlive()) {
                
                RPCResponse r1 = client.xtreemfs_service_offline(null, uuid.toString());
                r1.get();
                r1.freeBuffers();
                
            }
        } catch (Exception ex) {
            Logging.logMessage(Logging.LEVEL_WARN, this, "could not deregister service at DIR");
            Logging.logError(Logging.LEVEL_WARN, this, ex);
        } finally {
            try {
                if (r != null)
                    r.freeBuffers();
            } catch (Throwable thr) {
            }
        }
        
        this.quit = true;
        this.interrupt();
    }
    
    public void initialize() throws IOException {
        
        List<RPCResponse> responses = new LinkedList<RPCResponse>();
        
        // initially, ...
        try {
            
            // ... for each UUID, ...
            for (Service reg : serviceDataGen.getServiceData()) {
                
                // ... remove old DS entry if necessary
                RPCResponse<ServiceSet> r1 = client.xtreemfs_service_get_by_uuid(null, reg.getUuid());
                responses.add(r1);
                long currentVersion = 0;
                ServiceSet olset = r1.get();
                if (olset.size() > 0) {
                    currentVersion = olset.get(0).getVersion();
                }
                
                reg.setVersion(currentVersion);
                RPCResponse<Long> r2 = client.xtreemfs_service_register(null, reg);
                responses.add(r2);
                r2.get();
                
                if (Logging.isDebug())
                    Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this,
                        "%s successfully registered at Directory Service", uuid);
            }
            
            // ... register the address mapping for the service
            
            AddressMappingSet endpoints = new AddressMappingSet();
            
            // check if hostname or listen.address are set
            if ("".equals(config.getHostName()) && config.getAddress() == null) {
                
                endpoints = NetUtils.getReachableEndpoints(config.getPort(),
                    config.isUsingSSL() ? Constants.ONCRPCS_SCHEME : Constants.ONCRPC_SCHEME);
                
                if (advertiseUDPEndpoints)
                    endpoints.addAll(NetUtils.getReachableEndpoints(config.getPort(),
                        Constants.ONCRPCU_SCHEME));
                
                for (AddressMapping endpoint : endpoints) {
                    endpoint.setUuid(uuid.toString());
                }
                
            } else {
                // if it is set, we should use that for UUID mapping!
                endpoints = new AddressMappingSet();
                
                // remove the leading '/' if necessary
                String host = "".equals(config.getHostName()) ? config.getAddress().toString() : config
                        .getHostName();
                if (host.startsWith("/"))
                    host = host.substring(1);
                
                final String prot = config.isUsingSSL() ? Constants.ONCRPCS_SCHEME : Constants.ONCRPC_SCHEME;
                
                // add an oncrpc/oncrpcs mapping
                endpoints.add(new AddressMapping(uuid.toString(), 0, prot, host, config.getPort(), "*", 3600,
                    prot + "://" + host + ":" + config.getPort()));
                
                if (advertiseUDPEndpoints)
                    endpoints.add(new AddressMapping(uuid.toString(), 0, Constants.ONCRPCU_SCHEME, host,
                        config.getPort(), "*", 3600, Constants.ONCRPCU_SCHEME + "://" + host + ":"
                            + config.getPort()));
                
            }
            
            if (Logging.isInfo()) {
                Logging.logMessage(Logging.LEVEL_INFO, Category.net, this,
                    "registering the following address mapping for the service:");
                for (AddressMapping mapping : endpoints)
                    Logging.logMessage(Logging.LEVEL_INFO, Category.net, this, mapping.toString());
            }
            
            // fetch the latest address mapping version from the Directory
            // Serivce
            long version = 0;
            RPCResponse<AddressMappingSet> r2 = client.xtreemfs_address_mappings_get(null, uuid.toString());
            try {
                AddressMappingSet ams = r2.get();
                
                // retrieve the version number from the address mapping
                if (ams.size() > 0) {
                    version = ams.get(0).getVersion();
                }
            } finally {
                responses.add(r2);
            }
            
            if (endpoints.size() > 0)
                endpoints.get(0).setVersion(version);
            
            // register/update the current address mapping
            RPCResponse r3 = client.xtreemfs_address_mappings_set(null, endpoints);
            try {
                r3.get();
            } finally {
                responses.add(r3);
            }
        } catch (InterruptedException ex) {
        } catch (Exception ex) {
            Logging.logMessage(Logging.LEVEL_ERROR, this,
                "an error occurred while initially contacting the Directory Service: " + ex);
            throw new IOException("cannot initialize service at XtreemFS DIR: " + ex, ex);
        } finally {
            for (RPCResponse resp : responses)
                resp.freeBuffers();
        }
    }
    
    public void run() {
        
        List<RPCResponse> responses = new LinkedList<RPCResponse>();
        Map<String, Long> verMap = new HashMap<String, Long>();
        
        notifyStarted();
        
        // periodically, ...
        while (!quit) {
            
            synchronized (this) {
                
                responses.clear();
                
                try {
                    
                    // ... for each UUID, ...
                    for (Service reg : serviceDataGen.getServiceData()) {
                        
                        // ... remove old DS entry if necessary
                        RPCResponse<ServiceSet> r1 = client.xtreemfs_service_get_by_uuid(null, reg.getUuid());
                        long currentVersion = 0;
                        responses.add(r1);
                        ServiceSet olset = r1.get();
                        if (olset.size() > 0) {
                            currentVersion = olset.get(0).getVersion();
                        }
                        
                        reg.setVersion(currentVersion);
                        RPCResponse<Long> r2 = client.xtreemfs_service_register(null, reg);
                        responses.add(r2);
                        r2.get();
                        
                        if (Logging.isDebug())
                            Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this,
                                "%s successfully updated at Directory Service", uuid);
                    }
                    
                } catch (IOException ex) {
                    Logging.logError(Logging.LEVEL_ERROR, this, ex);
                } catch (ONCRPCException ex) {
                    Logging.logError(Logging.LEVEL_ERROR, this, ex);
                } catch (InterruptedException ex) {
                    quit = true;
                    break;
                } finally {
                    for (RPCResponse resp : responses)
                        resp.freeBuffers();
                }
                
                if (quit)
                    break;
                
            }
            
            try {
                Thread.sleep(UPDATE_INTERVAL);
            } catch (InterruptedException e) {
                // ignore
            }
        }
        
        notifyStopped();
    }
}
