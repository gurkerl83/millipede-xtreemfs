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

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.foundation.json.JSONException;
import org.xtreemfs.foundation.json.JSONParser;
import org.xtreemfs.mrc.brain.UserException;
import org.xtreemfs.new_mrc.ErrorRecord;
import org.xtreemfs.new_mrc.MRCRequest;
import org.xtreemfs.new_mrc.MRCRequestDispatcher;
import org.xtreemfs.new_mrc.ErrorRecord.ErrorClass;
import org.xtreemfs.new_mrc.ac.FileAccessManager;
import org.xtreemfs.new_mrc.dbaccess.AtomicDBUpdate;
import org.xtreemfs.new_mrc.dbaccess.StorageManager;
import org.xtreemfs.new_mrc.metadata.FileMetadata;
import org.xtreemfs.new_mrc.volumes.VolumeManager;
import org.xtreemfs.new_mrc.volumes.metadata.VolumeInfo;

/**
 * 
 * @author stender
 */
public class ReadDirOperation extends MRCOperation {
    
    static class Args {
        public String path;
    }
    
    public static final String RPC_NAME = "readDir";
    
    public ReadDirOperation(MRCRequestDispatcher master) {
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
            
            final VolumeManager vMan = master.getVolumeManager();
            final FileAccessManager faMan = master.getFileAccessManager();
            
            final Path p = new Path(rqArgs.path);
            
            final VolumeInfo volume = vMan.getVolumeByName(p.getComp(0));
            final StorageManager sMan = vMan.getStorageManager(volume.getId());
            final PathResolver res = new PathResolver(sMan, p);
            
            // check whether the path prefix is searchable
            faMan.checkSearchPermission(sMan, res.getPathPrefix(), rq.getDetails().userId, rq
                    .getDetails().superUser, rq.getDetails().groupIds);
            
            // check whether file exists
            res.checkIfFileDoesNotExist();
            
            FileMetadata file = res.getFile();
            
            // check whether the directory grants read access
            faMan.checkPermission(FileAccessManager.READ_ACCESS, sMan, file, res.getParentDirId(),
                rq.getDetails().userId, rq.getDetails().superUser, rq.getDetails().groupIds);
            
            AtomicDBUpdate update = null;
            // if required, update POSIX timestamps
            if (!master.getConfig().isNoAtime()) {
                update = sMan.createAtomicDBUpdate(master, rq);
                MRCOpHelper.updateFileTimes(res.getParentDirId(), file, true, false, false, sMan,
                    update);
            }
            
            List<Object> dirList = new LinkedList<Object>();
            Iterator<FileMetadata> it = sMan.getChildren(res.getFile().getId());
            while (it.hasNext())
                dirList.add(it.next().getFileName());
            
            rq.setData(ReusableBuffer.wrap(JSONParser.writeJSON(dirList).getBytes()));
            if (update != null)
                update.execute();
            else
                finishRequest(rq);
            
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
            
            args.path = (String) arguments.get(0);
            
            if (arguments.size() == 1)
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