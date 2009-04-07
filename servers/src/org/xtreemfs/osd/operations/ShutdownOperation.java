/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.xtreemfs.osd.operations;

import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.interfaces.DIRInterface.xtreemfs_address_mappings_getRequest;
import org.xtreemfs.interfaces.OSDInterface.xtreemfs_shutdownRequest;
import org.xtreemfs.interfaces.OSDInterface.xtreemfs_shutdownResponse;
import org.xtreemfs.interfaces.UserCredentials;
import org.xtreemfs.interfaces.utils.Serializable;
import org.xtreemfs.osd.ErrorCodes;
import org.xtreemfs.osd.OSDRequest;
import org.xtreemfs.osd.OSDRequestDispatcher;

/**
 *
 * @author bjko
 */
public class ShutdownOperation extends OSDOperation {

    private final int procId;

    public ShutdownOperation(OSDRequestDispatcher master) {
        super(master);
        xtreemfs_shutdownRequest tmp = new xtreemfs_shutdownRequest();
        procId = tmp.getOperationNumber();
    }

    @Override
    public int getProcedureId() {
        return procId;
    }

    @Override
    public void startRequest(OSDRequest rq) {

        //check password
        xtreemfs_shutdownRequest args = (xtreemfs_shutdownRequest)rq.getRequestArgs();

        UserCredentials uc = rq.getRPCRequest().getUserCredentials();

        if (uc.getPassword().equals(master.getConfig().getAdminPassword())) {
            //shutdown
            try {
                rq.sendSuccess(new xtreemfs_shutdownResponse());
                Thread.sleep(100);
                master.asyncShutdown();
            } catch (Throwable thr) {
                Logging.logMessage(Logging.LEVEL_ERROR, this,"exception during shutdown: "+thr);
            }
        } else {
            rq.sendOSDException(ErrorCodes.AUTH_FAILED, "password is not valid");
        }
    }

    @Override
    public void startInternalEvent(Object[] args) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public Serializable parseRPCMessage(ReusableBuffer data, OSDRequest rq) throws Exception {
        xtreemfs_shutdownRequest rpcrq = new xtreemfs_shutdownRequest();
        rpcrq.deserialize(data);

        return rpcrq;
    }

    @Override
    public boolean requiresCapability() {
        return false;
    }

}