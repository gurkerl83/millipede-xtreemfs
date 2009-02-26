/*  Copyright (c) 2008 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

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

package org.xtreemfs.new_mrc;

import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.lang.management.OperatingSystemMXBean;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import org.xtreemfs.common.HeartbeatThread;
import org.xtreemfs.common.TimeSync;
import org.xtreemfs.common.VersionManagement;
import org.xtreemfs.common.HeartbeatThread.ServiceDataGenerator;
import org.xtreemfs.common.auth.AuthenticationProvider;
import org.xtreemfs.common.auth.NullAuthProvider;
import org.xtreemfs.common.buffer.BufferPool;
import org.xtreemfs.common.clients.RPCClient;
import org.xtreemfs.common.clients.dir.DIRClient;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.util.OutputUtils;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.common.uuids.UUIDResolver;
import org.xtreemfs.foundation.LifeCycleListener;
import org.xtreemfs.foundation.json.JSONException;
import org.xtreemfs.foundation.pinky.HTTPHeaders;
import org.xtreemfs.foundation.pinky.HTTPUtils;
import org.xtreemfs.foundation.pinky.PinkyRequest;
import org.xtreemfs.foundation.pinky.PinkyRequestListener;
import org.xtreemfs.foundation.pinky.PipelinedPinky;
import org.xtreemfs.foundation.pinky.SSLOptions;
import org.xtreemfs.foundation.pinky.HTTPUtils.DATA_TYPE;
import org.xtreemfs.foundation.speedy.MultiSpeedy;
import org.xtreemfs.new_mrc.ac.FileAccessManager;
import org.xtreemfs.new_mrc.database.DBAccessResultListener;
import org.xtreemfs.new_mrc.database.DatabaseException;
import org.xtreemfs.new_mrc.metadata.StripingPolicy;
import org.xtreemfs.new_mrc.operations.StatusPageOperation;
import org.xtreemfs.new_mrc.operations.StatusPageOperation.Vars;
import org.xtreemfs.new_mrc.osdselection.OSDStatusManager;
import org.xtreemfs.new_mrc.stages.ProcessingStage;
import org.xtreemfs.new_mrc.utils.Converter;
import org.xtreemfs.new_mrc.utils.MRCOpHelper;
import org.xtreemfs.new_mrc.volumes.BabuDBVolumeManager;
import org.xtreemfs.new_mrc.volumes.VolumeManager;
import org.xtreemfs.new_mrc.volumes.metadata.VolumeInfo;

/**
 * 
 * @author bjko
 */
public class MRCRequestDispatcher implements PinkyRequestListener, LifeCycleListener,
    DBAccessResultListener {
    
    private final PipelinedPinky         pinkyStage;
    
    private final MultiSpeedy            speedyStage;
    
    private final DIRClient              dirClient;
    
    private final ProcessingStage        procStage;
    
    /**
     * auth string used for talking to the directory service
     */
    private final String                 authString;
    
    private final OSDStatusManager       osdMonitor;
    
    private final PolicyContainer        policyContainer;
    
    private final AuthenticationProvider authProvider;
    
    private final MRCConfig              config;
    
    private final HeartbeatThread        heartbeatThread;
    
    private final VolumeManager          volumeManager;
    
    private final FileAccessManager      fileAccessManager;
    
    public MRCRequestDispatcher(final MRCConfig config) throws IOException, JSONException,
        ClassNotFoundException, IllegalAccessException, InstantiationException, DatabaseException {
        
        this.config = config;
        
        // generate an authorization string for Directory Service operations
        authString = NullAuthProvider.createAuthString(config.getUUID().toString(), config
                .getUUID().toString());
        
        Logging.logMessage(Logging.LEVEL_DEBUG, this, "use SSL=" + config.isUsingSSL());
        
        speedyStage = config.isUsingSSL() ? new MultiSpeedy(new SSLOptions(config
                .getServiceCredsFile(), config.getServiceCredsPassphrase(), config
                .getServiceCredsContainer(), config.getTrustedCertsFile(), config
                .getTrustedCertsPassphrase(), config.getTrustedCertsContainer(), false))
            : new MultiSpeedy();
        speedyStage.setLifeCycleListener(this);
        
        pinkyStage = config.isUsingSSL() ? new PipelinedPinky(config.getPort(),
            config.getAddress(), this, new SSLOptions(config.getServiceCredsFile(), config
                    .getServiceCredsPassphrase(), config.getServiceCredsContainer(), config
                    .getTrustedCertsFile(), config.getTrustedCertsPassphrase(), config
                    .getTrustedCertsContainer(), false)) : new PipelinedPinky(config.getPort(),
            config.getAddress(), this);
        pinkyStage.setLifeCycleListener(this);
        
        dirClient = new DIRClient(speedyStage, config.getDirectoryService());
        
        policyContainer = new PolicyContainer(config);
        authProvider = policyContainer.getAuthenticationProvider();
        authProvider.initialize(config.isUsingSSL());
        if (Logging.isInfo())
            Logging.logMessage(Logging.LEVEL_INFO, this, "using authentication provider '"
                + authProvider.getClass().getName() + "'");
        
        osdMonitor = new OSDStatusManager(config, dirClient, policyContainer, authString);
        osdMonitor.setLifeCycleListener(this);
        
        procStage = new ProcessingStage(this);
        
        volumeManager = new BabuDBVolumeManager(this);
        fileAccessManager = new FileAccessManager(volumeManager, policyContainer);
        
        ServiceDataGenerator gen = new ServiceDataGenerator() {
            public Map<String, Map<String, Object>> getServiceData() {
                
                String uuid = config.getUUID().toString();
                
                OperatingSystemMXBean osb = ManagementFactory.getOperatingSystemMXBean();
                String load = String.valueOf((int) (osb.getSystemLoadAverage() * 100 / osb
                        .getAvailableProcessors()));
                
                long totalRAM = Runtime.getRuntime().maxMemory();
                long usedRAM = Runtime.getRuntime().totalMemory()
                    - Runtime.getRuntime().freeMemory();
                
                // get service data
                Map<String, Map<String, Object>> map = new HashMap<String, Map<String, Object>>();
                map.put(uuid, RPCClient.generateMap("type", "MRC", "load", load, "prot_versions",
                    VersionManagement.getSupportedProtVersAsString(), "totalRAM", Long
                            .toString(totalRAM), "usedRAM", Long.toString(usedRAM),
                    "geoCoordinates", config.getGeoCoordinates()));
                
                try {
                    for (VolumeInfo vol : volumeManager.getVolumes()) {
                        Map<String, Object> dsVolumeInfo = MRCOpHelper.createDSVolumeInfo(vol,
                            osdMonitor, uuid);
                        map.put(vol.getId(), dsVolumeInfo);
                    }
                } catch (DatabaseException exc) {
                    Logging.logMessage(Logging.LEVEL_ERROR, this, exc);
                }
                
                return map;
            }
            
        };
        
        heartbeatThread = new HeartbeatThread("MRC Heartbeat Thread", dirClient, config.getUUID(),
            gen, authString, config);
    }
    
    public void startup() throws Exception {
        
        TimeSync.initialize(dirClient, config.getRemoteTimeSync(), config.getLocalClockRenew(),
            authString);
        
        UUIDResolver.start(dirClient, 10 * 1000, 600 * 1000);
        UUIDResolver.addLocalMapping(config.getUUID(), config.getPort(), config.isUsingSSL());
        
        speedyStage.start();
        speedyStage.waitForStartup();
        osdMonitor.start();
        osdMonitor.waitForStartup();
        
        procStage.start();
        procStage.waitForStartup();
        
        volumeManager.init();
        volumeManager.addVolumeChangeListener(osdMonitor);
        
        heartbeatThread.start();
        
        pinkyStage.start();
        pinkyStage.waitForStartup();
        
        Logging.logMessage(Logging.LEVEL_INFO, this, "MRC operational, listening on port "
            + config.getPort());
    }
    
    public void shutdown() throws Exception {
        pinkyStage.shutdown();
        pinkyStage.waitForShutdown();
        
        heartbeatThread.shutdown();
        
        osdMonitor.shutdown();
        osdMonitor.waitForShutdown();
        
        procStage.shutdown();
        procStage.waitForShutdown();
        
        UUIDResolver.shutdown();
        speedyStage.shutdown();
        speedyStage.waitForShutdown();
        
        volumeManager.shutdown();
        
        TimeSync.getInstance().shutdown();
    }
    
    public void requestFinished(MRCRequest request) {
        // send response back to client, if a pinky request is present
        assert (request != null);
        
        final PinkyRequest pr = request.getPinkyRequest();
        assert (pr != null);
        
        if (request.getError() != null) {
            final ErrorRecord error = request.getError();
            switch (error.getErrorClass()) {
            case REDIRECT: {
                Logging.logMessage(Logging.LEVEL_ERROR, this, error.getErrorMessage()
                    + " / request: " + request);
                
                HTTPHeaders headers = new HTTPHeaders();
                headers.addHeader(HTTPHeaders.HDR_LOCATION + ":" + error.getErrorMessage());
                pr.setResponse(HTTPUtils.SC_SEE_OTHER, null, DATA_TYPE.JSON, headers);
                break;
            }
            case INTERNAL_SERVER_ERROR: {
                Logging.logMessage(Logging.LEVEL_ERROR, this, error.getErrorMessage()
                    + " / request: " + request);
                Logging.logMessage(Logging.LEVEL_ERROR, this,error.getThrowable());
                pr.setResponse(HTTPUtils.SC_SERVER_ERROR, error.getErrorMessage() + "\n\n");
                break;
            }
            case USER_EXCEPTION: {
                pr.setResponse(HTTPUtils.SC_USER_EXCEPTION, error.toJSON());
                break;
            }
            case BAD_REQUEST: {
                pr.setResponse(HTTPUtils.SC_BAD_REQUEST, error.getErrorMessage());
                break;
            }
            default: {
                pr.setResponse(HTTPUtils.SC_SERVER_ERROR, "an unknown error type was returned: "
                    + error);
                break;
            }
            }
            // we don't need that any more
            if (request.getData() != null)
                BufferPool.free(request.getData());
        } else {
            if (request.getDataType() == null)
                request.setDataType(DATA_TYPE.JSON);
            
            pr.setResponse(HTTPUtils.SC_OKAY, request.getData(), request.getDataType(), request
                    .getAdditionalResponseHeaders());
        }
        pinkyStage.sendResponse(pr);
    }
    
    public Map<StatusPageOperation.Vars, String> getStatusInformation() {
        HashMap<StatusPageOperation.Vars, String> data = new HashMap();
        
        data.put(Vars.AVAILPROCS, String.valueOf(Runtime.getRuntime().availableProcessors()));
        data.put(Vars.BPSTATS, BufferPool.getStatus());
        data.put(Vars.DEBUG, Integer.toString(config.getDebugLevel()));
        data.put(Vars.DIRURL, "http://" + config.getDirectoryService().getHostName() + ":"
            + config.getDirectoryService().getPort());
        data.put(Vars.GLOBALRESYNC, Long.toString(TimeSync.getTimeSyncInterval()));
        
        final long globalTime = TimeSync.getGlobalTime();
        final long localTime = TimeSync.getLocalSystemTime();
        data.put(Vars.GLOBALTIME, new Date(globalTime).toString() + " (" + globalTime + ")");
        data.put(Vars.LOCALTIME, new Date(localTime).toString() + " (" + localTime + ")");
        data.put(Vars.LOCALRESYNC, Long.toString(TimeSync.getLocalRenewInterval()));
        
        data.put(Vars.PORT, Integer.toString(config.getPort()));
        
        data.put(Vars.UUID, config.getUUID().toString());
        data.put(Vars.UUIDCACHE, UUIDResolver.getCache());
        
        long freeMem = Runtime.getRuntime().freeMemory();
        String span = "<span>";
        if (freeMem < 1024 * 1024 * 32) {
            span = "<span class=\"levelWARN\">";
        } else if (freeMem < 1024 * 1024 * 2) {
            span = "<span class=\"levelERROR\">";
        }
        data.put(Vars.MEMSTAT, span + OutputUtils.formatBytes(freeMem) + " / "
            + OutputUtils.formatBytes(Runtime.getRuntime().maxMemory()) + " / "
            + OutputUtils.formatBytes(Runtime.getRuntime().totalMemory()) + "</span>");
        
        // TODO: add request statistics
        // StringBuffer rqTableBuf = new StringBuffer();
        // long totalRequests = 0;
        // for (String req : brainStage._statMap.keySet()) {
        //
        // long count = brainStage._statMap.get(req);
        // totalRequests += count;
        //
        // rqTableBuf.append("<tr><td align=\"left\">'");
        // rqTableBuf.append(req);
        // rqTableBuf.append("'</td><td>");
        // rqTableBuf.append(count);
        // rqTableBuf.append("</td></tr>");
        // }
        
        // add volume statistics
        try {
            StringBuffer volTableBuf = new StringBuffer();
            Collection<VolumeInfo> vols = volumeManager.getVolumes();
            for (VolumeInfo v : vols) {
                
                Map<String, Map<String, Object>> osdList = osdMonitor.getUsableOSDs(v.getId());
                
                volTableBuf.append("<tr><td align=\"left\">");
                volTableBuf.append(v.getName());
                volTableBuf
                        .append("</td><td><table border=\"0\" cellpadding=\"0\"><tr><td class=\"subtitle\">selectable OSDs</td><td align=\"right\">");
                Iterator<String> it = osdList.keySet().iterator();
                while (it.hasNext()) {
                    final ServiceUUID osdUUID = new ServiceUUID(it.next());
                    volTableBuf.append("<a href=\"");
                    volTableBuf.append(osdUUID.toURL());
                    volTableBuf.append("\">");
                    volTableBuf.append(osdUUID);
                    volTableBuf.append("</a>");
                    if (it.hasNext())
                        volTableBuf.append(", ");
                }
                
                StripingPolicy defaultSP = volumeManager.getStorageManager(v.getId())
                        .getDefaultStripingPolicy(1);
                
                volTableBuf.append("</td></tr><tr><td class=\"subtitle\">striping policy</td><td>");
                volTableBuf.append(Converter.stripingPolicyToString(defaultSP));
                volTableBuf.append("</td></tr><tr><td class=\"subtitle\">access policy</td><td>");
                volTableBuf.append(v.getAcPolicyId());
                volTableBuf.append("</td></tr><tr><td class=\"subtitle\">osd policy</td><td>");
                volTableBuf.append(v.getOsdPolicyId());
                volTableBuf.append("</td></tr></table></td></tr>");
            }
            
            data.put(Vars.VOLUMES, volTableBuf.toString());
        } catch (Exception exc) {
            data.put(Vars.VOLUMES,
                "<tr><td align=\"left\">could not retrieve volume info due to an server internal error: "
                    + exc + "</td></tr>");
        }
        
        return data;
    }
    
    public void receiveRequest(PinkyRequest theRequest) {
        // no callback, special stage which executes the operatios
        procStage.enqueueOperation(new MRCRequest(theRequest),
            ProcessingStage.STAGEOP_PARSE_AND_EXECUTE, null);
    }
    
    public VolumeManager getVolumeManager() {
        return volumeManager;
    }
    
    public FileAccessManager getFileAccessManager() {
        return fileAccessManager;
    }
    
    public AuthenticationProvider getAuthProvider() {
        return authProvider;
    }
    
    public OSDStatusManager getOSDStatusManager() {
        return osdMonitor;
    }
    
    public DIRClient getDirClient() {
        return dirClient;
    }
    
    public String getAuthString() {
        return authString;
    }
    
    public MRCConfig getConfig() {
        return config;
    }
    
    public void startupPerformed() {
    }
    
    public void shutdownPerformed() {
    }
    
    public void crashPerformed() {
        Logging.logMessage(Logging.LEVEL_ERROR, this,
            "A component crashed. Shutting down MRC, trying to checkpoint database");
        try {
            shutdown();
        } catch (Exception e) {
            Logging.logMessage(Logging.LEVEL_ERROR, this, e);
            System.exit(1);
        }
    }
    
    @Override
    public void insertFinished(Object context) {
        requestFinished((MRCRequest) context);
    }
    
    @Override
    public void lookupFinished(Object context, byte[] value) {
        
    }
    
    @Override
    public void prefixLookupFinished(Object context, Iterator<Entry<byte[], byte[]>> iterator) {
        
    }
    
    @Override
    public void userDefinedLookupFinished(Object context, Object result) {
        
    }
    
    @Override
    public void requestFailed(Object context, Throwable error) {
        
    }
    
}
