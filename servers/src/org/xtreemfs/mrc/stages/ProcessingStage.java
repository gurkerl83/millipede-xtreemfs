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

package org.xtreemfs.mrc.stages;

import java.util.HashMap;
import java.util.Map;

import org.xtreemfs.common.auth.AuthenticationException;
import org.xtreemfs.common.auth.UserCredentials;
import org.xtreemfs.foundation.oncrpc.server.ONCRPCRequest;
import org.xtreemfs.interfaces.Context;
import org.xtreemfs.mrc.ErrNo;
import org.xtreemfs.mrc.ErrorRecord;
import org.xtreemfs.mrc.MRCRequest;
import org.xtreemfs.mrc.MRCRequestDispatcher;
import org.xtreemfs.mrc.UserException;
import org.xtreemfs.mrc.ErrorRecord.ErrorClass;
import org.xtreemfs.mrc.operations.AddReplicaOperation;
import org.xtreemfs.mrc.operations.ChangeAccessModeOperation;
import org.xtreemfs.mrc.operations.ChangeOwnerOperation;
import org.xtreemfs.mrc.operations.CheckAccessOperation;
import org.xtreemfs.mrc.operations.CheckFileListOperation;
import org.xtreemfs.mrc.operations.CheckpointOperation;
import org.xtreemfs.mrc.operations.CreateDirOperation;
import org.xtreemfs.mrc.operations.CreateFileOperation;
import org.xtreemfs.mrc.operations.CreateLinkOperation;
import org.xtreemfs.mrc.operations.CreateSymLinkOperation;
import org.xtreemfs.mrc.operations.CreateVolumeOperation;
import org.xtreemfs.mrc.operations.DeleteOperation;
import org.xtreemfs.mrc.operations.DeleteVolumeOperation;
import org.xtreemfs.mrc.operations.DumpDBOperation;
import org.xtreemfs.mrc.operations.GetLocalVolumesOperation;
import org.xtreemfs.mrc.operations.GetXAttrOperation;
import org.xtreemfs.mrc.operations.GetXAttrsOperation;
import org.xtreemfs.mrc.operations.MRCOperation;
import org.xtreemfs.mrc.operations.MoveOperation;
import org.xtreemfs.mrc.operations.OpenOperation;
import org.xtreemfs.mrc.operations.ReadDirAndStatOperation;
import org.xtreemfs.mrc.operations.RemoveACLEntriesOperation;
import org.xtreemfs.mrc.operations.RemoveReplicaOperation;
import org.xtreemfs.mrc.operations.RenewOperation;
import org.xtreemfs.mrc.operations.RestoreDBOperation;
import org.xtreemfs.mrc.operations.RestoreFileOperation;
import org.xtreemfs.mrc.operations.SetACLEntriesOperation;
import org.xtreemfs.mrc.operations.SetXAttrOperation;
import org.xtreemfs.mrc.operations.ShutdownOperation;
import org.xtreemfs.mrc.operations.StatFSOperation;
import org.xtreemfs.mrc.operations.StatOperation;
import org.xtreemfs.mrc.operations.UpdateFileSizeOperation;
import org.xtreemfs.mrc.operations.UtimeOperation;

/**
 * 
 * @author bjko
 */
public class ProcessingStage extends MRCStage {
    
    public static final int                  STAGEOP_PARSE_AND_EXECUTE = 1;
    
    private final MRCRequestDispatcher       master;
    
    private final Map<Integer, MRCOperation> operations;
    
    public ProcessingStage(MRCRequestDispatcher master) {
        super("ProcSt");
        this.master = master;
        operations = new HashMap<Integer, MRCOperation>();
        
        installOperations();
    }
    
    public void installOperations() {
        operations.put(ShutdownOperation.OP_ID, new ShutdownOperation(master));
        // operations.put(StatusPageOperation.RPC_NAME, new
        // StatusPageOperation(master));
        operations.put(CreateVolumeOperation.OP_ID, new CreateVolumeOperation(master));
        operations.put(DeleteVolumeOperation.OP_ID, new DeleteVolumeOperation(master));
        operations.put(GetLocalVolumesOperation.OP_ID, new GetLocalVolumesOperation(master));
        operations.put(StatOperation.OP_ID, new StatOperation(master));
        operations.put(CheckAccessOperation.OP_ID, new CheckAccessOperation(master));
        operations.put(ReadDirAndStatOperation.OP_ID, new ReadDirAndStatOperation(master));
        operations.put(CreateFileOperation.OP_ID, new CreateFileOperation(master));
        operations.put(CreateDirOperation.OP_ID, new CreateDirOperation(master));
        operations.put(CreateSymLinkOperation.OP_ID, new CreateSymLinkOperation(master));
        operations.put(DeleteOperation.OP_ID, new DeleteOperation(master));
        operations.put(GetXAttrOperation.OP_ID, new GetXAttrOperation(master));
        operations.put(GetXAttrsOperation.OP_ID, new GetXAttrsOperation(master));
        operations.put(SetXAttrOperation.OP_ID, new SetXAttrOperation(master));
        operations.put(OpenOperation.OP_ID, new OpenOperation(master));
        operations.put(UpdateFileSizeOperation.OP_ID, new UpdateFileSizeOperation(master));
        operations.put(RenewOperation.OP_ID, new RenewOperation(master));
        operations.put(ChangeOwnerOperation.OP_ID, new ChangeOwnerOperation(master));
        operations.put(ChangeAccessModeOperation.OP_ID, new ChangeAccessModeOperation(master));
        operations.put(AddReplicaOperation.OP_ID, new AddReplicaOperation(master));
        operations.put(RemoveReplicaOperation.OP_ID, new RemoveReplicaOperation(master));
        operations.put(MoveOperation.OP_ID, new MoveOperation(master));
        operations.put(CreateLinkOperation.OP_ID, new CreateLinkOperation(master));
        operations.put(StatFSOperation.OP_ID, new StatFSOperation(master));
        operations.put(UtimeOperation.OP_ID, new UtimeOperation(master));
        operations.put(SetACLEntriesOperation.OP_ID, new SetACLEntriesOperation(master));
        operations.put(RemoveACLEntriesOperation.OP_ID, new RemoveACLEntriesOperation(master));
        operations.put(DumpDBOperation.OP_ID, new DumpDBOperation(master));
        operations.put(RestoreDBOperation.OP_ID, new RestoreDBOperation(master));
        operations.put(CheckFileListOperation.OP_ID, new CheckFileListOperation(master));
        operations.put(RestoreFileOperation.OP_ID, new RestoreFileOperation(master));
        operations.put(CheckpointOperation.OP_ID, new CheckpointOperation(master));
    }
    
    @Override
    protected void processMethod(StageMethod method) {
        switch (method.getStageMethod()) {
        case STAGEOP_PARSE_AND_EXECUTE:
            parseAndExecute(method);
            break;
        default:
            method.getRq().setError(
                new ErrorRecord(ErrorRecord.ErrorClass.INTERNAL_SERVER_ERROR, "unknown stage operation"));
            master.requestFinished(method.getRq());
        }
    }
    
    /**
     * Parse request and execute method
     * 
     * @param method
     *            stagemethod to execute
     */
    private void parseAndExecute(StageMethod method) {
        
        final MRCRequest rq = method.getRq();
        final ONCRPCRequest rpcRequest = rq.getRPCRequest();
        
        final MRCOperation op = operations.get(rpcRequest.getRequestHeader().getOperationNumber());
        if (op == null) {
            rq.setError(new ErrorRecord(ErrorClass.UNKNOWN_OPERATION,
                "requested operation is not available on this MRC"));
            master.requestFinished(rq);
            return;
        }
        
        // parse request arguments
        ErrorRecord error = op.parseRequestArgs(rq);
        if (error != null) {
            rq.setError(error);
            master.requestFinished(rq);
            return;
        }
        
        try {
            
            // parse the user Id from the "AUTHORIZATION" header
            Context ctx = op.getContext(rq);
            
            UserCredentials cred = null;
            try {
                cred = master.getAuthProvider().getEffectiveCredentials(ctx, rpcRequest.getChannel());
                rq.getDetails().superUser = cred.isSuperUser();
                rq.getDetails().groupIds = cred.getGroupIDs();
                rq.getDetails().userId = cred.getUserID();
            } catch (AuthenticationException ex) {
                throw new UserException(ErrNo.EPERM, ex.getMessage());
            }
            
        } catch (Exception exc) {
            
            method.getRq().setError(
                new ErrorRecord(ErrorRecord.ErrorClass.INTERNAL_SERVER_ERROR,
                    "could not initialize authentication module", exc));
            master.requestFinished(method.getRq());
            return;
        }
        
        op.startRequest(rq);
    }
    
}