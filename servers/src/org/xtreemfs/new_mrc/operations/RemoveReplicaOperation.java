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

package org.xtreemfs.new_mrc.operations;

import java.util.List;
import java.util.Map;

import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.foundation.json.JSONException;
import org.xtreemfs.foundation.json.JSONParser;
import org.xtreemfs.mrc.brain.ErrNo;
import org.xtreemfs.mrc.brain.UserException;
import org.xtreemfs.new_mrc.ErrorRecord;
import org.xtreemfs.new_mrc.MRCRequest;
import org.xtreemfs.new_mrc.MRCRequestDispatcher;
import org.xtreemfs.new_mrc.ErrorRecord.ErrorClass;
import org.xtreemfs.new_mrc.ac.FileAccessManager;
import org.xtreemfs.new_mrc.dbaccess.AtomicDBUpdate;
import org.xtreemfs.new_mrc.dbaccess.StorageManager;
import org.xtreemfs.new_mrc.metadata.FileMetadata;
import org.xtreemfs.new_mrc.metadata.XLoc;
import org.xtreemfs.new_mrc.metadata.XLocList;
import org.xtreemfs.new_mrc.volumes.VolumeManager;
import org.xtreemfs.new_mrc.volumes.metadata.VolumeInfo;

/**
 * 
 * @author stender
 */
public class RemoveReplicaOperation extends MRCOperation {
    
    static class Args {
        
        public String              fileId;
        
        public Map<String, Object> stripingPolicy;
        
        public List<Object>        osdList;
        
    }
    
    public static final String RPC_NAME = "removeReplica";
    
    public RemoveReplicaOperation(MRCRequestDispatcher master) {
        super(master);
    }
    
    @Override
    public boolean hasArguments() {
        return true;
    }
    
    @Override
    public boolean isAuthRequired() {
        return true;
    }
    
    @Override
    public void startRequest(MRCRequest rq) {
        
        try {
            
            Args rqArgs = (Args) rq.getRequestArgs();
            
            final FileAccessManager faMan = master.getFileAccessManager();
            final VolumeManager vMan = master.getVolumeManager();
            
            // parse volume and file ID from global file ID
            long fileId = 0;
            String volumeId = null;
            try {
                String globalFileId = rqArgs.fileId;
                int i = globalFileId.indexOf(':');
                volumeId = rqArgs.fileId.substring(0, i);
                fileId = Long.parseLong(rqArgs.fileId.substring(i + 1));
            } catch (Exception exc) {
                throw new UserException("invalid global file ID: " + rqArgs.fileId
                    + "; expected pattern: <volume_ID>:<local_file_ID>");
            }
            
            StorageManager sMan = vMan.getStorageManager(volumeId);
            
            // resolve the file ID to parent ID + file Name
            Object[] parentAndFileName = sMan.getParentIdAndFileName(fileId);
            long parentId = (Long) parentAndFileName[0];
            String fileName = (String) parentAndFileName[1];
            
            // retrieve the file metadata
            FileMetadata file = sMan.getMetadata(parentId, fileName);
            if (file == null)
                throw new UserException(ErrNo.ENOENT, "file '" + fileId + "' does not exist");
            
            // if the file refers to a symbolic link, resolve the link
            String target = sMan.getSoftlinkTarget(file.getId());
            if (target != null) {
                String path = target;
                Path p = new Path(path);
                
                // if the local MRC is not responsible, send a redirect
                if (!vMan.hasVolume(p.getComp(0))) {
                    finishRequest(rq, new ErrorRecord(ErrorClass.REDIRECT, target));
                    return;
                }
                
                VolumeInfo volume = vMan.getVolumeByName(p.getComp(0));
                sMan = vMan.getStorageManager(volume.getId());
                PathResolver res = new PathResolver(sMan, p);
                file = res.getFile();
                parentId = res.getParentDirId();
                fileName = res.getFileName();
            }
            
            if (file.isDirectory())
                throw new UserException(ErrNo.EPERM, "replicas may only be removed from files");
            
            // check whether privileged permissions are granted for removing
            // replicas
            faMan.checkPrivilegedPermissions(volumeId, file.getId(), rq.getDetails().userId, rq
                    .getDetails().superUser, rq.getDetails().groupIds);
            
            if (!file.isReadOnly())
                throw new UserException(ErrNo.EPERM,
                    "the file has to be made read-only before adding replicas");
            
            XLocList xLocList = file.getXLocList();
            
            // find and remove the replica from the X-Locations list
            int i = 0;
            for (; i < xLocList.getReplicaCount(); i++) {
                
                XLoc replica = xLocList.getReplica(i);
                
                // compare the first elements from the lists; since an OSD may
                // only occur once in each X-Locations list, it is not necessary
                // to go through the entire list
                if (replica.getOSD(0).equals(rqArgs.osdList.get(0)))
                    break;
            }
            
            // create and assign a new X-Locations list that excludes the
            // replica to remove
            XLoc[] newReplList = new XLoc[xLocList.getReplicaCount() - 1];
            for (int j = 0, count = 0; j < xLocList.getReplicaCount(); j++)
                if (j != i)
                    newReplList[count++] = xLocList.getReplica(j);
            xLocList = sMan.createXLocList(newReplList, xLocList.getVersion() + 1);
            file.setXLocList(xLocList);
            
            AtomicDBUpdate update = sMan.createAtomicDBUpdate(master, rq);
            
            // update the X-Locations list
            sMan.setMetadata(parentId, fileName, file, FileMetadata.XLOC_METADATA, update);
            
            // update POSIX timestamps
            MRCOpHelper.updateFileTimes(parentId, file, false, true, false, sMan, update);
            
            // FIXME: this line is needed due to a BUG in the client which
            // expects some useless return value
            rq.setData(ReusableBuffer.wrap(JSONParser.writeJSON(null).getBytes()));
            
            update.execute();
            
        } catch (UserException exc) {
            Logging.logMessage(Logging.LEVEL_TRACE, this, exc);
            finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, exc.getErrno(), exc
                    .getMessage(), exc));
        } catch (Exception exc) {
            finishRequest(rq, new ErrorRecord(ErrorClass.INTERNAL_SERVER_ERROR,
                "an error has occurred", exc));
        }
    }
    
    @Override
    public ErrorRecord parseRPCBody(MRCRequest rq, List<Object> arguments) {
        
        Args args = new Args();
        
        try {
            
            args.fileId = (String) arguments.get(0);
            args.stripingPolicy = (Map<String, Object>) arguments.get(1);
            args.osdList = (List<Object>) arguments.get(2);
            
            if (arguments.size() == 3)
                return null;
            
            throw new Exception();
            
        } catch (Exception exc) {
            try {
                return new ErrorRecord(ErrorClass.BAD_REQUEST, "invalid arguments for operation '"
                    + getClass().getSimpleName() + "': " + JSONParser.writeJSON(arguments));
            } catch (JSONException je) {
                Logging.logMessage(Logging.LEVEL_ERROR, this, exc);
                return new ErrorRecord(ErrorClass.BAD_REQUEST, "invalid arguments for operation '"
                    + getClass().getSimpleName() + "'");
            }
        } finally {
            rq.setRequestArgs(args);
        }
    }
    
}
