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

import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.interfaces.Context;
import org.xtreemfs.interfaces.MRCInterface.getxattrRequest;
import org.xtreemfs.interfaces.MRCInterface.getxattrResponse;
import org.xtreemfs.mrc.ErrorRecord;
import org.xtreemfs.mrc.MRCRequest;
import org.xtreemfs.mrc.MRCRequestDispatcher;
import org.xtreemfs.mrc.UserException;
import org.xtreemfs.mrc.ErrorRecord.ErrorClass;
import org.xtreemfs.mrc.ac.FileAccessManager;
import org.xtreemfs.mrc.database.StorageManager;
import org.xtreemfs.mrc.metadata.FileMetadata;
import org.xtreemfs.mrc.utils.MRCHelper;
import org.xtreemfs.mrc.utils.Path;
import org.xtreemfs.mrc.utils.PathResolver;
import org.xtreemfs.mrc.volumes.VolumeManager;
import org.xtreemfs.mrc.volumes.metadata.VolumeInfo;

/**
 * 
 * @author stender
 */
public class GetXAttrOperation extends MRCOperation {
    
    public static final int OP_ID = 6;
    
    public GetXAttrOperation(MRCRequestDispatcher master) {
        super(master);
    }
    
    @Override
    public void startRequest(MRCRequest rq) {
        
        try {
            
            final getxattrRequest rqArgs = (getxattrRequest) rq.getRequestArgs();
            
            final VolumeManager vMan = master.getVolumeManager();
            final FileAccessManager faMan = master.getFileAccessManager();
            
            Path p = new Path(rqArgs.getPath());
            
            VolumeInfo volume = vMan.getVolumeByName(p.getComp(0));
            StorageManager sMan = vMan.getStorageManager(volume.getId());
            PathResolver res = new PathResolver(sMan, p);
            
            // check whether the path prefix is searchable
            faMan.checkSearchPermission(sMan, res, rq.getDetails().userId, rq.getDetails().superUser, rq
                    .getDetails().groupIds);
            
            // check whether file exists
            res.checkIfFileDoesNotExist();
            
            // retrieve and prepare the metadata to return
            FileMetadata file = res.getFile();
            
            // if the file refers to a symbolic link, resolve the link
            String target = sMan.getSoftlinkTarget(file.getId());
            if (target != null) {
                rqArgs.setPath(target);
                p = new Path(rqArgs.getPath());
                
                // if the local MRC is not responsible, send a redirect
                if (!vMan.hasVolume(p.getComp(0))) {
                    finishRequest(rq, new ErrorRecord(ErrorClass.REDIRECT, target));
                    return;
                }
                
                volume = vMan.getVolumeByName(p.getComp(0));
                sMan = vMan.getStorageManager(volume.getId());
                res = new PathResolver(sMan, p);
                file = res.getFile();
            }
            
            String value = null;
            if (rqArgs.getName().startsWith("xtreemfs."))
                value = MRCHelper.getSysAttrValue(master.getConfig(), sMan, master.getOSDStatusManager(),
                    volume, res.toString(), file, rqArgs.getName().substring(9));
            else {
                
                // first, try to fetch an individual user attribute
                value = sMan.getXAttr(file.getId(), rq.getDetails().userId, rqArgs.getName());
                
                // if no such attribute exists, try to fetch a global attribute
                if (value == null)
                    value = sMan.getXAttr(file.getId(), StorageManager.GLOBAL_ID, rqArgs.getName());
                
                if (value == null)
                    value = "";
            }
            
            // set the response
            rq.setResponse(new getxattrResponse(value));
            finishRequest(rq);
            
        } catch (UserException exc) {
            Logging.logMessage(Logging.LEVEL_TRACE, this, exc);
            finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, exc.getErrno(), exc.getMessage(),
                exc));
        } catch (Exception exc) {
            finishRequest(rq, new ErrorRecord(ErrorClass.INTERNAL_SERVER_ERROR, "an error has occurred", exc));
        }
    }
    
    public Context getContext(MRCRequest rq) {
        return ((getxattrRequest) rq.getRequestArgs()).getContext();
    }
    
}