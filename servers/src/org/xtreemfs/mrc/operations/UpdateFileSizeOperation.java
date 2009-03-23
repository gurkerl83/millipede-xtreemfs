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
 * AUTHORS: Jan Stender (ZIB)
 */

package org.xtreemfs.mrc.operations;

import org.xtreemfs.common.Capability;
import org.xtreemfs.common.TimeSync;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.interfaces.NewFileSize;
import org.xtreemfs.interfaces.NewFileSizeSet;
import org.xtreemfs.interfaces.MRCInterface.xtreemfs_update_file_sizeRequest;
import org.xtreemfs.interfaces.MRCInterface.xtreemfs_update_file_sizeResponse;
import org.xtreemfs.mrc.ErrNo;
import org.xtreemfs.mrc.ErrorRecord;
import org.xtreemfs.mrc.MRCRequest;
import org.xtreemfs.mrc.MRCRequestDispatcher;
import org.xtreemfs.mrc.UserException;
import org.xtreemfs.mrc.ErrorRecord.ErrorClass;
import org.xtreemfs.mrc.database.AtomicDBUpdate;
import org.xtreemfs.mrc.database.StorageManager;
import org.xtreemfs.mrc.metadata.FileMetadata;

/**
 * 
 * @author stender
 */
public class UpdateFileSizeOperation extends MRCOperation {
    
    public static final int OP_ID = 29;
    
    public UpdateFileSizeOperation(MRCRequestDispatcher master) {
        super(master);
    }
    
    @Override
    public void startRequest(MRCRequest rq) {
        
        try {
            
            final xtreemfs_update_file_sizeRequest rqArgs = (xtreemfs_update_file_sizeRequest) rq
                    .getRequestArgs();
            
            Capability cap = new Capability(rqArgs.getXcap(), master.getConfig().getCapabilitySecret());
            
            // check whether the capability has a valid signature
            if (!cap.hasValidSignature())
                throw new UserException(cap + " does not have a valid signature");
            
            // check whether the capability has expired
            if (cap.hasExpired())
                throw new UserException(cap + " has expired");
            
            // parse volume and file ID from global file ID
            long fileId = 0;
            String volumeId = null;
            try {
                String globalFileId = cap.getFileId();
                int i = globalFileId.indexOf(':');
                volumeId = globalFileId.substring(0, i);
                fileId = Long.parseLong(globalFileId.substring(i + 1));
            } catch (Exception exc) {
                throw new UserException("invalid global file ID: " + cap.getFileId()
                    + "; expected pattern: <volume_ID>:<local_file_ID>");
            }
            StorageManager sMan = master.getVolumeManager().getStorageManager(volumeId);
            
            FileMetadata file = sMan.getMetadata(fileId);
            if (file == null)
                throw new UserException(ErrNo.ENOENT, "file '" + fileId + "' does not exist");
            
            NewFileSizeSet newFSSet = rqArgs.getOsd_write_response().getNew_file_size();
            
            if (newFSSet.isEmpty())
                throw new UserException(ErrNo.EINVAL, "invalid file size: empty");
            
            NewFileSize newFS = newFSSet.get(0);
            long newFileSize = newFS.getSize_in_bytes();
            int epochNo = newFS.getTruncate_epoch();
            
            AtomicDBUpdate update = sMan.createAtomicDBUpdate(master, rq);
            
            // only accept valid file size updates
            if (epochNo >= file.getEpoch()) {
                
                // accept any file size in a new epoch but only larger file
                // sizes in
                // the current epoch
                if (epochNo > file.getEpoch() || newFileSize > file.getSize()) {
                    
                    int time = (int) (TimeSync.getGlobalTime() / 1000);
                    
                    file.setSize(newFileSize);
                    file.setEpoch(epochNo);
                    file.setCtime(time);
                    file.setMtime(time);
                    
                    sMan.setMetadata(file, FileMetadata.FC_METADATA, update);
                    sMan.setMetadata(file, FileMetadata.RC_METADATA, update);
                }
            }

            else {
                if (epochNo < file.getEpoch())
                    if (Logging.isDebug()) {
                        Logging.logMessage(Logging.LEVEL_DEBUG, this,
                            "received file size update w/ invalid epoch: " + epochNo + ", current epoch="
                                + file.getEpoch());
                    } else if (Logging.isDebug()) {
                        Logging.logMessage(Logging.LEVEL_DEBUG, this,
                            "received update for outdated file size: " + newFileSize + ", current file size="
                                + file.getSize());
                    }
            }
            
            // set the response
            rq.setResponse(new xtreemfs_update_file_sizeResponse());
            
            update.execute();
            
        } catch (UserException exc) {
            Logging.logMessage(Logging.LEVEL_TRACE, this, exc);
            finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, exc.getErrno(), exc.getMessage(),
                exc));
        } catch (Exception exc) {
            finishRequest(rq, new ErrorRecord(ErrorClass.INTERNAL_SERVER_ERROR, "an error has occurred", exc));
        }
    }
}
