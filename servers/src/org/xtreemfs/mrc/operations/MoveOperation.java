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

import java.net.InetSocketAddress;

import org.xtreemfs.common.Capability;
import org.xtreemfs.common.TimeSync;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.interfaces.Context;
import org.xtreemfs.interfaces.FileCredentials;
import org.xtreemfs.interfaces.FileCredentialsSet;
import org.xtreemfs.interfaces.MRCInterface.renameRequest;
import org.xtreemfs.interfaces.MRCInterface.renameResponse;
import org.xtreemfs.mrc.ErrNo;
import org.xtreemfs.mrc.ErrorRecord;
import org.xtreemfs.mrc.MRCRequest;
import org.xtreemfs.mrc.MRCRequestDispatcher;
import org.xtreemfs.mrc.UserException;
import org.xtreemfs.mrc.ErrorRecord.ErrorClass;
import org.xtreemfs.mrc.ac.FileAccessManager;
import org.xtreemfs.mrc.database.AtomicDBUpdate;
import org.xtreemfs.mrc.database.StorageManager;
import org.xtreemfs.mrc.metadata.FileMetadata;
import org.xtreemfs.mrc.utils.Converter;
import org.xtreemfs.mrc.utils.MRCHelper;
import org.xtreemfs.mrc.utils.Path;
import org.xtreemfs.mrc.utils.PathResolver;
import org.xtreemfs.mrc.utils.MRCHelper.FileType;
import org.xtreemfs.mrc.volumes.VolumeManager;
import org.xtreemfs.mrc.volumes.metadata.VolumeInfo;

/**
 * 
 * @author stender
 */
public class MoveOperation extends MRCOperation {
    
    public static final int OP_ID = 14;
    
    public MoveOperation(MRCRequestDispatcher master) {
        super(master);
    }
    
    @Override
    public void startRequest(MRCRequest rq) {
        
        try {
            
            final renameRequest rqArgs = (renameRequest) rq.getRequestArgs();
            
            final VolumeManager vMan = master.getVolumeManager();
            final FileAccessManager faMan = master.getFileAccessManager();
            
            final Path sp = new Path(rqArgs.getSource_path());
            
            final VolumeInfo volume = vMan.getVolumeByName(sp.getComp(0));
            final StorageManager sMan = vMan.getStorageManager(volume.getId());
            final PathResolver sRes = new PathResolver(sMan, sp);
            
            // check whether the path prefix is searchable
            faMan.checkSearchPermission(sMan, sRes, rq.getDetails().userId, rq.getDetails().superUser, rq
                    .getDetails().groupIds);
            
            // check whether the parent directory grants write access
            faMan.checkPermission(FileAccessManager.O_WRONLY, sMan, sRes.getParentDir(), sRes
                    .getParentsParentId(), rq.getDetails().userId, rq.getDetails().superUser,
                rq.getDetails().groupIds);
            
            final Path tp = new Path(rqArgs.getTarget_path());
            
            // check arguments
            if (sp.getCompCount() == 1)
                throw new UserException(ErrNo.ENOENT, "cannot move a volume");
            
            if (!sp.getComp(0).equals(tp.getComp(0)))
                throw new UserException(ErrNo.ENOENT, "cannot move between volumes");
            
            // check whether the file/directory exists
            sRes.checkIfFileDoesNotExist();
            
            // check whether the entry itself can be moved (this is e.g.
            // important w/ POSIX access control if the sticky bit is set)
            faMan.checkPermission(FileAccessManager.NON_POSIX_RM_MV_IN_DIR, sMan, sRes.getFile(), sRes
                    .getParentDirId(), rq.getDetails().userId, rq.getDetails().superUser,
                rq.getDetails().groupIds);
            
            FileMetadata source = sRes.getFile();
            
            // find out what the source path refers to (1 = directory, 2 = file)
            FileType sourceType = source.isDirectory() ? FileType.dir : FileType.file;
            
            final PathResolver tRes = new PathResolver(sMan, tp);
            
            FileMetadata targetParentDir = tRes.getParentDir();
            if (targetParentDir == null || !targetParentDir.isDirectory())
                throw new UserException(ErrNo.ENOTDIR, "'" + tp.getComps(0, tp.getCompCount() - 2)
                    + "' is not a directory");
            
            FileMetadata target = tRes.getFile();
            
            // find out what the target path refers to (0 = does not exist, 1 =
            // directory, 2 = file)
            FileType targetType = tp.getCompCount() == 1 ? FileType.dir : target == null ? FileType.nexists
                : target.isDirectory() ? FileType.dir : FileType.file;
            
            // if both the old and the new directory point to the same
            // entity, do nothing
            if (sp.equals(tp)) {
                finishRequest(rq);
                return;
            }
            
            // check whether the path prefix of the target file is
            // searchable
            faMan.checkSearchPermission(sMan, tRes, rq.getDetails().userId, rq.getDetails().superUser, rq
                    .getDetails().groupIds);
            
            // check whether the parent directory of the target file grants
            // write access
            faMan.checkPermission(FileAccessManager.O_WRONLY, sMan, tRes.getParentDir(), tRes
                    .getParentsParentId(), rq.getDetails().userId, rq.getDetails().superUser,
                rq.getDetails().groupIds);
            
            AtomicDBUpdate update = sMan.createAtomicDBUpdate(master, rq);
            FileCredentialsSet creds = new FileCredentialsSet();
            
            switch (sourceType) {
            
            // source is a directory
            case dir: {
                
                // check whether the target is a subdirectory of the
                // source directory; if so, throw an exception
                if (tp.isSubDirOf(sp))
                    throw new UserException(ErrNo.EINVAL, "cannot move '" + sp
                        + "' to one of its own subdirectories");
                
                switch (targetType) {
                
                case nexists: // target does not exist
                {
                    // relink the metadata object to the parent directory of
                    // the target path and remove the former link
                    short newLinkCount = sMan.delete(sRes.getParentDirId(), sRes.getFileName(), update);
                    source.setLinkCount(newLinkCount);
                    source.setCtime((int) (TimeSync.getGlobalTime() / 1000));
                    sMan.link(source, tRes.getParentDirId(), tRes.getFileName(), update);
                    
                    break;
                }
                    
                case dir: // target is a directory
                {
                    
                    // check whether the target directory may be overwritten; if
                    // so, delete it
                    faMan.checkPermission(FileAccessManager.NON_POSIX_DELETE, sMan, target, tRes
                            .getParentDirId(), rq.getDetails().userId, rq.getDetails().superUser, rq
                            .getDetails().groupIds);
                    
                    if (sMan.getChildren(target.getId()).hasNext())
                        throw new UserException(ErrNo.ENOTEMPTY, "target directory '" + tRes
                            + "' is not empty");
                    else
                        sMan.delete(tRes.getParentDirId(), tRes.getFileName(), update);
                    
                    // relink the metadata object to the parent directory of
                    // the target path and remove the former link
                    short newLinkCount = sMan.delete(sRes.getParentDirId(), sRes.getFileName(), update);
                    source.setLinkCount(newLinkCount);
                    source.setCtime((int) (TimeSync.getGlobalTime() / 1000));
                    sMan.link(source, tRes.getParentDirId(), tRes.getFileName(), update);
                    
                    break;
                }
                    
                case file: // target is a file
                    throw new UserException(ErrNo.ENOTDIR, "cannot rename directory '" + sRes + "' to file '"
                        + tRes + "'");
                    
                }
                
                break;
                
            }
                
                // source is a file
            case file: {
                
                switch (targetType) {
                
                case nexists: // target does not exist
                {
                    
                    // relink the metadata object to the parent directory of
                    // the target path and remove the former link
                    short newLinkCount = sMan.delete(sRes.getParentDirId(), sRes.getFileName(), update);
                    source.setLinkCount(newLinkCount);
                    source.setCtime((int) (TimeSync.getGlobalTime() / 1000));
                    sMan.link(source, tRes.getParentDirId(), tRes.getFileName(), update);
                    
                    break;
                }
                    
                case dir: // target is a directory
                {
                    throw new UserException(ErrNo.EISDIR, "cannot rename file '" + sRes + "' to directory '"
                        + tRes + "'");
                }
                    
                case file: // target is a file
                {
                    
                    // unless there is still another link to the target file,
                    // i.e. the target file must not be deleted yet, create a
                    // 'delete' capability and include the XCapability and
                    // XLocationsList headers in the response
                    if (target.getLinkCount() == 1) {
                        
                        // create a deletion capability for the file
                        Capability cap = new Capability(volume.getId() + ":" + target.getId(),
                            FileAccessManager.NON_POSIX_DELETE, Integer.MAX_VALUE, ((InetSocketAddress) rq
                                    .getRPCRequest().getClientIdentity()).getAddress().getHostAddress(),
                            target.getEpoch(), master.getConfig().getCapabilitySecret());
                        
                        creds.add(new FileCredentials(Converter.xLocListToXLocSet(target.getXLocList()), cap
                                .getXCap()));
                    }
                    
                    // delete the target
                    sMan.delete(tRes.getParentDirId(), tRes.getFileName(), update);
                    
                    // relink the metadata object to the parent directory of
                    // the target path and remove the former link
                    short newLinkCount = sMan.delete(sRes.getParentDirId(), sRes.getFileName(), update);
                    source.setLinkCount(newLinkCount);
                    source.setCtime((int) (TimeSync.getGlobalTime() / 1000));
                    sMan.link(source, tRes.getParentDirId(), tRes.getFileName(), update);
                    
                    break;
                }
                    
                }
                
            }
            }
            
            // update POSIX timestamps of source and target parent directories
            MRCHelper.updateFileTimes(sRes.getParentsParentId(), sRes.getParentDir(), false, true, true,
                sMan, update);
            MRCHelper.updateFileTimes(tRes.getParentsParentId(), tRes.getParentDir(), false, true, true,
                sMan, update);
            
            // set the response
            rq.setResponse(new renameResponse(creds));
            
            update.execute();
            
        } catch (UserException exc) {
            Logging.logMessage(Logging.LEVEL_TRACE, this, exc);
            finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, exc.getErrno(), exc.getMessage(),
                exc));
        } catch (Exception exc) {
            finishRequest(rq, new ErrorRecord(ErrorClass.INTERNAL_SERVER_ERROR, "an error has occurred", exc));
        }
    }
    
    public Context getContext(MRCRequest rq) {
        return ((renameRequest) rq.getRequestArgs()).getContext();
    }
    
}