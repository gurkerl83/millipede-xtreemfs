/*  Copyright (c) 2009 Barcelona Supercomputing Center - Centro Nacional
    de Supercomputacion and Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

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
package org.xtreemfs.osd.operations;

import java.io.IOException;

import org.xtreemfs.common.Capability;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.common.xloc.XLocations;
import org.xtreemfs.foundation.buffer.ReusableBuffer;
import org.xtreemfs.foundation.oncrpc.utils.XDRUnmarshaller;
import org.xtreemfs.interfaces.ObjectList;
import org.xtreemfs.interfaces.OSDInterface.xtreemfs_internal_get_object_setRequest;
import org.xtreemfs.interfaces.OSDInterface.xtreemfs_internal_get_object_setResponse;
import org.xtreemfs.interfaces.utils.ONCRPCException;
import org.xtreemfs.osd.OSDRequest;
import org.xtreemfs.osd.OSDRequestDispatcher;
import org.xtreemfs.osd.replication.ObjectSet;
import org.xtreemfs.osd.stages.StorageStage.GetObjectListCallback;

/**
 *
 * <br>15.06.2009
 */
public class GetObjectSetOperation extends OSDOperation {
    final int procId;

    final String sharedSecret;

    final ServiceUUID localUUID;

    public GetObjectSetOperation(OSDRequestDispatcher master) {
        super(master);
        xtreemfs_internal_get_object_setRequest rq = new xtreemfs_internal_get_object_setRequest();
        procId = xtreemfs_internal_get_object_setRequest.TAG;
        sharedSecret = master.getConfig().getCapabilitySecret();
        localUUID = master.getConfig().getUUID();
    }

    @Override
    public int getProcedureId() {
        return procId;
    }

    @Override
    public void startRequest(final OSDRequest rq) {
        final xtreemfs_internal_get_object_setRequest args = (xtreemfs_internal_get_object_setRequest) rq
                .getRequestArgs();

//        System.out.println("rq: " + args);

        master.getStorageStage().getObjectSet(args.getFile_id(), rq.getLocationList().getLocalReplica().getStripingPolicy(), rq,
                new GetObjectListCallback() {
                    @Override
                    public void getObjectSetComplete(ObjectSet result, Exception error) {
                        postReadObjectList(rq, args, result, error);
                    }
                });
    }

    public void postReadObjectList(final OSDRequest rq, xtreemfs_internal_get_object_setRequest args,
            ObjectSet result, Exception error) {
        if (error != null) {
            if (error instanceof ONCRPCException) {
                rq.sendException((ONCRPCException) error);
            } else {
                rq.sendInternalServerError(error);
            }
        } else {
            // serialize objectSet
            ReusableBuffer objectSetBuffer = null;
            byte[] serialized;
            try {
                serialized = result.getSerializedBitSet();
                objectSetBuffer = ReusableBuffer.wrap(serialized);

                ObjectList objList = new ObjectList(objectSetBuffer, result.getStripeWidth(),
                        result.getFirstObjectNo());
                sendResponse(rq, objList);
            } catch (IOException e) {
                rq.sendInternalServerError(e);
            }
        }
    }

    public void sendResponse(OSDRequest rq, ObjectList objectList) {
        xtreemfs_internal_get_object_setResponse response = new xtreemfs_internal_get_object_setResponse(objectList);
        rq.sendSuccess(response);
    }

    @Override
    public yidl.runtime.Object parseRPCMessage(ReusableBuffer data, OSDRequest rq) throws Exception {
        xtreemfs_internal_get_object_setRequest rpcrq = new xtreemfs_internal_get_object_setRequest();
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
