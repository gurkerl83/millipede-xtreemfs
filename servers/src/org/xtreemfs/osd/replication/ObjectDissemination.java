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
 * AUTHORS: Christian Lorenz (ZIB)
 */
package org.xtreemfs.osd.replication;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.xtreemfs.common.Capability;
import org.xtreemfs.common.buffer.BufferPool;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.common.uuids.UnknownUUIDException;
import org.xtreemfs.common.xloc.StripingPolicyImpl;
import org.xtreemfs.common.xloc.XLocations;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.foundation.oncrpc.client.RPCResponseAvailableListener;
import org.xtreemfs.interfaces.FileCredentials;
import org.xtreemfs.interfaces.InternalReadLocalResponse;
import org.xtreemfs.interfaces.ObjectData;
import org.xtreemfs.interfaces.utils.ONCRPCException;
import org.xtreemfs.osd.OSDRequestDispatcher;
import org.xtreemfs.osd.client.OSDClient;
import org.xtreemfs.osd.operations.EventWriteObject;
import org.xtreemfs.osd.operations.OSDOperation;
import org.xtreemfs.osd.replication.TransferStrategy.NextRequest;
import org.xtreemfs.osd.stages.ReplicationStage.FetchObjectCallback;
import org.xtreemfs.osd.stages.Stage.StageRequest;
import org.xtreemfs.osd.storage.CowPolicy;
import org.xtreemfs.osd.storage.ObjectInformation;
import org.xtreemfs.osd.storage.ObjectInformation.ObjectStatus;

/**
 * Handles the fetching of replicas.
 * 15.09.2008
 * 
 * @author clorenz
 */
public class ObjectDissemination {
    private OSDRequestDispatcher master;

    /**
     * Encapsulates important infos about a file
     * 01.04.2009
     */
    private static class FileInfo {
        private static class ObjectInfo {
            List<StageRequest> waitingRequests;
            List<ServiceUUID> availableOSDs;
            ServiceUUID lastOSD;

            public ObjectInfo(List<ServiceUUID> osds) {
                this.waitingRequests = new ArrayList<StageRequest>();
                this.availableOSDs = osds;
            }
        }

        String fileID;
        final TransferStrategy strategy;
        XLocations xLoc;
        Capability cap;
        CowPolicy cow;
        /**
         * key: objectNo
         */
        private HashMap<Long, ObjectInfo> filesInProgress;

        public FileInfo(String fileID, TransferStrategy strategy, XLocations xLoc, Capability cap,
                CowPolicy cow) {
            this.fileID = fileID;
            this.strategy = strategy;
            this.xLoc = xLoc;
            this.cap = cap;
            this.cow = cow;
            this.filesInProgress = new HashMap<Long, ObjectInfo>();
        }

        /**
         * @see java.util.HashMap#containsKey(java.lang.Object)
         */
        public boolean containsKey(Long arg0) {
            return filesInProgress.containsKey(arg0);
        }

        /**
         * @see java.util.HashMap#get(java.lang.Object)
         */
        public ObjectInfo get(Long arg0) {
            return filesInProgress.get(arg0);
        }

        /**
         * @see java.util.ArrayList#add(java.lang.Object)
         */
        public ObjectInfo add(Long arg0, StageRequest rq, List<ServiceUUID> usedOSDs) {
            ObjectInfo info = new ObjectInfo(usedOSDs);
            info.waitingRequests.add(rq);
            filesInProgress.put(arg0, info);
            return info;
        }

        /**
         * @see java.util.HashMap#isEmpty()
         */
        public boolean isEmpty() {
            return filesInProgress.isEmpty();
        }

        /**
         * @see java.util.HashMap#remove(java.lang.Object)
         */
        public ObjectInfo remove(Long arg0) {
            return filesInProgress.remove(arg0);
        }

        /**
         * @see java.util.HashMap#size()
         */
        public int size() {
            return filesInProgress.size();
        }
    }

    /**
     * objects of these files are currently or in future downloading
     * key: fileID
     */
    private HashMap<String, FileInfo> filesInProgress;

    public ObjectDissemination(OSDRequestDispatcher master) {
        this.master = master;

        this.filesInProgress = new HashMap<String, FileInfo>();
    }

    /**
     * saves the request and fetches the object
     */
    public void fetchObject(String fileID, long objectNo, XLocations xLoc, Capability capability,
            CowPolicy cow, final StageRequest rq) {
        FileInfo fileInfo = this.filesInProgress.get(fileID);
        if (fileInfo == null) { // file not in progress
            // create a new strategy
            TransferStrategy strategy = new SimpleStrategy(fileID, capability, xLoc, xLoc.getXLocSet()
                    .getRead_only_file_size());
            fileInfo = new FileInfo(fileID, strategy, xLoc, capability, cow);
            this.filesInProgress.put(fileID, fileInfo);

            if (Logging.isDebug())
                Logging.logMessage(Logging.LEVEL_DEBUG, this, "start replication for file " + fileID);
        }

        // keep in mind current request and initialize available OSDs for object
        fileInfo.add(objectNo, rq, xLoc.getOSDsForObject(objectNo));
        fileInfo.strategy.addPreferredObject(objectNo);
        if (Logging.isDebug())
            Logging.logMessage(Logging.LEVEL_DEBUG, this, "FETCHING OBJECT: " + fileID + ":" + objectNo);

        prepareRequest(fileInfo, objectNo);
    }

    /**
     * Prepares the request by using the associated TransferStrategy.
     */
    private void prepareRequest(FileInfo fileInfo, long objectNo) {
        TransferStrategy strategy = fileInfo.strategy;
        strategy.selectNextOSD(objectNo);
        NextRequest next = strategy.getNext();

        if (next != null) { // there is something to fetch
            fileInfo.get(objectNo).lastOSD = next.osd;
            sendFetchObjectRequest(next.objectNo, next.osd, fileInfo.fileID, fileInfo.cap, fileInfo.xLoc);
        } else {
            // FIXME: use correct Exception
            sendError(fileInfo, objectNo, new Exception());
            
            if (filesInProgress.isEmpty())
                fileCompleted(fileInfo.fileID);
        }
    }

    /**
     * Sends a RPC for reading the object on another OSD.
     */
    private void sendFetchObjectRequest(final long objectNo, ServiceUUID osd, final String fileID,
            Capability capability, XLocations xLoc) {
        try {
            OSDClient client = master.getOSDClient();
            // FIXME: change this, if using different striping policies
            RPCResponse<InternalReadLocalResponse> response = client.internal_read_local(osd.getAddress(),
                    fileID, new FileCredentials(xLoc.getXLocSet(), capability.getXCap()), objectNo, 0, 0,
                    xLoc.getLocalReplica().getStripingPolicy().getStripeSizeForObject(objectNo));

            response.registerListener(new RPCResponseAvailableListener<InternalReadLocalResponse>() {
                @Override
                public void responseAvailable(RPCResponse<InternalReadLocalResponse> r) {
                    try {
                        ObjectData data = r.get().getData();
                        master.getReplicationStage().InternalObjectFetched(fileID, objectNo, data);
                    } catch (ONCRPCException e) {
                        // TODO Auto-generated catch block
                        master.getReplicationStage().InternalObjectFetched(fileID, objectNo, null);
                        e.printStackTrace();
                    } catch (IOException e) {
                        // TODO Auto-generated catch block
                        master.getReplicationStage().InternalObjectFetched(fileID, objectNo, null);
                        e.printStackTrace();
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } finally {
                        r.freeBuffers();
                    }
                }
            });
        } catch (UnknownUUIDException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * creates the responses and updates the maps
     */
    public void objectFetched(String fileID, long objectNo, ObjectData data) {
        FileInfo fileInfo = filesInProgress.get(fileID);

        if (!data.getInvalid_checksum_on_osd()) {
            // correct checksum
            sendResponses(fileInfo, objectNo, data.getData(), ObjectStatus.EXISTS);

            // write data to disk
            OSDOperation writeObjectEvent = master.getInternalEvent(EventWriteObject.class);
            // NOTE: "original" buffer is used
            writeObjectEvent.startInternalEvent(new Object[] { fileID, objectNo, data.getData(), fileInfo.xLoc,
                    fileInfo.cow });

            // delete object in maps/lists
            fileInfo.strategy.removeOSDListForObject(objectNo);
            fileInfo.strategy.removePreferredObject(objectNo);
            fileInfo.remove(objectNo);
            if (Logging.isDebug())
                Logging.logMessage(Logging.LEVEL_DEBUG, this, "object fetched " + fileID + ":" + objectNo);

            if (filesInProgress.isEmpty())
                fileCompleted(fileID);
        } else {
            // try next replica
            fileInfo.strategy.addPreferredObject(objectNo); // TODO: preferred or only requested?
            prepareRequest(fileInfo, objectNo);
            
            // free buffer
            BufferPool.free(data.getData());
        }
    }

    /**
     * Checks if it is a hole. Otherwise it tries to use another OSD for fetching.
     */
    public void objectNotFetched(String fileID, long objectNo) {
        FileInfo fileInfo = filesInProgress.get(fileID);

        // lastOSD could not help => remove it from list for this object
        fileInfo.strategy.removeOSDForObject(objectNo, fileInfo.get(objectNo).lastOSD);

        // check if it is a hole
        if (!fileInfo.strategy.isOSDAvailableForObject(objectNo)) { // => hole or error; we assume it is a
                                                                    // hole
            if (Logging.isDebug())
                Logging.logMessage(Logging.LEVEL_DEBUG, this, "object " + fileID + ":" + objectNo
                        + " could not be fetched; must be a hole.");

            sendResponses(fileInfo, objectNo, null, ObjectStatus.PADDING_OBJECT);

            if (filesInProgress.isEmpty())
                fileCompleted(fileID);

            // TODO: remember on this OSD that this object is a hole
        } else {
            // try next replica
            fileInfo.strategy.addPreferredObject(objectNo); // TODO: preferred or only requested?
            prepareRequest(fileInfo, objectNo);
        }
    }

    /**
     * Stops replication for this file.
     * @param fileID
     * @param fileInfo
     */
    private void fileCompleted(String fileID) {
        // if the last requested object was fetched for this file => remove from map
        filesInProgress.remove(fileID);
        if (Logging.isDebug())
            Logging.logMessage(Logging.LEVEL_DEBUG, this, "stop replicating file " + fileID);
    }

    /**
     * sends the responses to all belonging clients 
     */
    private void sendResponses(FileInfo fileInfo, long objectNo, ReusableBuffer data, ObjectStatus status) {
        List<StageRequest> reqs = fileInfo.get(objectNo).waitingRequests;
        StripingPolicyImpl sp = fileInfo.xLoc.getLocalReplica().getStripingPolicy();
        // responses
        for (StageRequest rq : reqs) {
            ObjectInformation objectInfo;
            // create returning ObjectInformation
            if (status == ObjectStatus.EXISTS) {
                objectInfo = new ObjectInformation(status, data.createViewBuffer(), sp
                        .getStripeSizeForObject(objectNo));
            } else {
                objectInfo = new ObjectInformation(status, null, sp.getStripeSizeForObject(objectNo));
            }
            // FIXME: filesize not from strategy
            objectInfo.setGlobalLastObjectNo(sp.getObjectNoForOffset(fileInfo.strategy.getFilesize() - 1));

            final FetchObjectCallback callback = (FetchObjectCallback) rq.getCallback();
            callback.fetchComplete(objectInfo, null);
        }
    }
    
    /**
     * sends an error to all belonging clients 
     */
    private void sendError(FileInfo fileInfo, long objectNo, Exception error) {
        List<StageRequest> reqs = fileInfo.get(objectNo).waitingRequests;
        // responses
        for (StageRequest rq : reqs) {
            final FetchObjectCallback callback = (FetchObjectCallback) rq.getCallback();
            callback.fetchComplete(null, error);
        }
    }
}
