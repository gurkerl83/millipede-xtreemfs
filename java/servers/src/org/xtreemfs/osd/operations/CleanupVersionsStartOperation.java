/*
 * Copyright (c) 2009-2011 by Jan Stender,
 *               Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

package org.xtreemfs.osd.operations;

import org.xtreemfs.foundation.buffer.ReusableBuffer;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.Auth;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.ErrorType;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.POSIXErrno;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.UserCredentials;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.RPCHeader.ErrorResponse;
import org.xtreemfs.osd.ErrorCodes;
import org.xtreemfs.osd.OSDRequest;
import org.xtreemfs.osd.OSDRequestDispatcher;
import org.xtreemfs.pbrpc.generatedinterfaces.OSDServiceConstants;
import org.xtreemfs.pbrpc.generatedinterfaces.OSD.xtreemfs_cleanup_startRequest;

public final class CleanupVersionsStartOperation extends OSDOperation {

    public CleanupVersionsStartOperation(OSDRequestDispatcher master) {
        super(master);
    }

    @Override
    public int getProcedureId() {
        return OSDServiceConstants.PROC_ID_XTREEMFS_CLEANUP_VERSIONS_START;
    }
    
    @Override
    public void startRequest(final OSDRequest rq) {

        Auth authData = rq.getRPCRequest().getHeader().getRequestHeader().getAuthData();
        if (!authData.hasAuthPasswd() || authData.getAuthPasswd().equals(master.getConfig().getAdminPassword())) {
            rq.sendError(ErrorType.ERRNO, POSIXErrno.POSIX_ERROR_EACCES, "this operation requires an admin password");
            return;
        }
        master.getCleanupVersionsThread().cleanupStart(rq.getRPCRequest().getHeader().getRequestHeader().getUserCreds());
        rq.sendSuccess(null,null);
    }

    @Override
    public ErrorResponse parseRPCMessage(OSDRequest rq) {
        rq.setFileId("");

        return null;
    }

    @Override
    public boolean requiresCapability() {
        return false;
    }

    @Override
    public void startInternalEvent(Object[] args) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    

}