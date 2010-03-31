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
package org.xtreemfs.osd.operations;

import java.util.List;
import org.xtreemfs.common.Capability;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.common.uuids.UnknownUUIDException;
import org.xtreemfs.common.xloc.XLocations;
import org.xtreemfs.foundation.buffer.BufferPool;
import org.xtreemfs.foundation.buffer.ReusableBuffer;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.foundation.oncrpc.utils.XDRUnmarshaller;
import org.xtreemfs.interfaces.OSDInterface.OSDException;
import org.xtreemfs.interfaces.InternalGmax;
import org.xtreemfs.interfaces.SnapConfig;
import org.xtreemfs.interfaces.OSDInterface.xtreemfs_check_objectRequest;
import org.xtreemfs.interfaces.OSDInterface.xtreemfs_check_objectResponse;
import org.xtreemfs.interfaces.ObjectData;
import org.xtreemfs.interfaces.utils.ONCRPCException;
import org.xtreemfs.osd.ErrorCodes;
import org.xtreemfs.osd.OSDRequest;
import org.xtreemfs.osd.OSDRequestDispatcher;
import org.xtreemfs.osd.stages.StorageStage.ReadObjectCallback;
import org.xtreemfs.osd.storage.ObjectInformation;
import org.xtreemfs.osd.storage.StorageLayout;

public final class CheckObjectOperation extends OSDOperation {

    final int procId;

    final String sharedSecret;

    final ServiceUUID localUUID;

    public CheckObjectOperation(OSDRequestDispatcher master) {
        super(master);
        procId = xtreemfs_check_objectRequest.TAG;
        sharedSecret = master.getConfig().getCapabilitySecret();
        localUUID = master.getConfig().getUUID();
    }

    @Override
    public int getProcedureId() {
        return procId;
    }

    @Override
    public void startRequest(final OSDRequest rq) {
        final xtreemfs_check_objectRequest args = (xtreemfs_check_objectRequest) rq.getRequestArgs();

        if (args.getObject_number() < 0) {
            rq.sendException(new OSDException(ErrorCodes.INVALID_PARAMS, "object number must be >= 0", ""));
            return;
        }
        
        master.getStorageStage().readObject(args.getFile_id(), args.getObject_number(), rq.getLocationList().getLocalReplica().getStripingPolicy(),
                0,StorageLayout.FULL_OBJECT_LENGTH, rq.getCapability().getSnapConfig() == SnapConfig.SNAP_CONFIG_ACCESS_SNAP ? rq.getCapability()
                    .getSnapTimestamp() : 0, rq, new ReadObjectCallback() {

            @Override
            public void readComplete(ObjectInformation result, Exception error) {
                step2(rq, args, result, error);
            }
        });
    }

    public void step2(final OSDRequest rq, xtreemfs_check_objectRequest args, ObjectInformation result, Exception error) {
        if (error != null) {
            if (error instanceof ONCRPCException) {
                rq.sendException((ONCRPCException) error);
            } else {
                rq.sendInternalServerError(error);
            }
        } else {
            if (rq.getLocationList().getLocalReplica().getOSDs().size() == 1) {
                //non-striped case
                nonStripedCheckObject(rq, args, result);
            } else {
                //striped read
                stripedCheckObject(rq, args, result);
            }

        }

    }

    private void nonStripedCheckObject(OSDRequest rq, xtreemfs_check_objectRequest args, ObjectInformation result) {

        final boolean isLastObjectOrEOF = result.getLastLocalObjectNo() <= args.getObject_number();
        readFinish(rq, args, result, isLastObjectOrEOF);
    }

    private void stripedCheckObject(final OSDRequest rq, final xtreemfs_check_objectRequest args, final ObjectInformation result) {
        ObjectData data;
        final long objNo = args.getObject_number();
        final long lastKnownObject = Math.max(result.getLastLocalObjectNo(), result.getGlobalLastObjectNo());
        final boolean isLastObjectLocallyKnown = lastKnownObject <= objNo;
        //check if GMAX must be fetched to determin EOF
        if ((objNo > lastKnownObject) ||
                (objNo == lastKnownObject) && (result.getData() != null) && (result.getData().remaining() < result.getStripeSize())) {
            try {
                final List<ServiceUUID> osds = rq.getLocationList().getLocalReplica().getOSDs();
                final RPCResponse[] gmaxRPCs = new RPCResponse[osds.size() - 1];
                int cnt = 0;
                for (ServiceUUID osd : osds) {
                    if (!osd.equals(localUUID)) {
                        gmaxRPCs[cnt++] = master.getOSDClient().internal_get_gmax(osd.getAddress(), args.getFile_id(), args.getFile_credentials());
                    }
                }
                this.waitForResponses(gmaxRPCs, new ResponsesListener() {

                    @Override
                    public void responsesAvailable() {
                        stripedCheckObjectAnalyzeGmax(rq, args, result, gmaxRPCs);
                    }
                });
            } catch (UnknownUUIDException ex) {
                rq.sendInternalServerError(ex);
                return;
            }
        } else {
            readFinish(rq, args, result, isLastObjectLocallyKnown);
        }
    }

    private void stripedCheckObjectAnalyzeGmax(final OSDRequest rq, final xtreemfs_check_objectRequest args,
            final ObjectInformation result, RPCResponse[] gmaxRPCs) {
        long maxObjNo = -1;
        long maxTruncate = -1;

        try {
            for (int i = 0; i < gmaxRPCs.length; i++) {
                InternalGmax gmax = (InternalGmax) gmaxRPCs[i].get();
                if ((gmax.getLast_object_id() > maxObjNo) && (gmax.getEpoch() >= maxTruncate)) {
                    //found new max
                    maxObjNo = gmax.getLast_object_id();
                    maxTruncate = gmax.getEpoch();
                }
            }
            final boolean isLastObjectLocallyKnown = maxObjNo <= args.getObject_number();
            readFinish(rq, args, result, isLastObjectLocallyKnown);
            //and update gmax locally
            master.getStorageStage().receivedGMAX_ASYNC(args.getFile_id(), maxTruncate, maxObjNo);
        } catch (Exception ex) {
            rq.sendInternalServerError(ex);
        } finally {
            for (RPCResponse r : gmaxRPCs)
                r.freeBuffers();
        }

    }

    private void readFinish(OSDRequest rq, xtreemfs_check_objectRequest args, ObjectInformation result, boolean isLastObjectOrEOF) {

        ObjectData data;
        data = result.getObjectData(isLastObjectOrEOF,0,result.getStripeSize());
        if (data.getData() != null) {
            data.setZero_padding(data.getZero_padding()+data.getData().remaining());
            BufferPool.free(data.getData());
            data.setData(null);
        }
        sendResponse(rq, data);
    }

    public void sendResponse(OSDRequest rq, ObjectData result) {
        xtreemfs_check_objectResponse response = new xtreemfs_check_objectResponse(result);
        rq.sendSuccess(response);
    }

    @Override
    public yidl.runtime.Object parseRPCMessage(ReusableBuffer data, OSDRequest rq) throws Exception {
        xtreemfs_check_objectRequest rpcrq = new xtreemfs_check_objectRequest();
        rpcrq.unmarshal(new XDRUnmarshaller(data));

        rq.setFileId(rpcrq.getFile_id());
        rq.setCapability(new Capability(rpcrq.getFile_credentials().getXcap(), sharedSecret));
        rq.setLocationList(new XLocations(rpcrq.getFile_credentials().getXlocs(), localUUID));

        return rpcrq;
    }

    @Override
    public boolean requiresCapability() {
        return true;
    }

    @Override
    public void startInternalEvent(Object[] args) {
        throw new UnsupportedOperationException("Not supported yet.");
    }
}