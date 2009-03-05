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
package org.xtreemfs.mrc.database.babudb;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;

import org.xtreemfs.babudb.BabuDB;
import org.xtreemfs.babudb.BabuDBException;
import org.xtreemfs.common.TimeSync;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.mrc.database.AtomicDBUpdate;
import org.xtreemfs.mrc.database.DBAccessResultListener;
import org.xtreemfs.mrc.database.DatabaseException;
import org.xtreemfs.mrc.database.StorageManager;
import org.xtreemfs.mrc.database.DatabaseException.ExceptionType;
import org.xtreemfs.mrc.database.babudb.BabuDBStorageHelper.ACLIterator;
import org.xtreemfs.mrc.database.babudb.BabuDBStorageHelper.ChildrenIterator;
import org.xtreemfs.mrc.database.babudb.BabuDBStorageHelper.XAttrIterator;
import org.xtreemfs.mrc.metadata.ACLEntry;
import org.xtreemfs.mrc.metadata.BufferBackedACLEntry;
import org.xtreemfs.mrc.metadata.BufferBackedFileMetadata;
import org.xtreemfs.mrc.metadata.BufferBackedStripingPolicy;
import org.xtreemfs.mrc.metadata.BufferBackedXAttr;
import org.xtreemfs.mrc.metadata.BufferBackedXLoc;
import org.xtreemfs.mrc.metadata.BufferBackedXLocList;
import org.xtreemfs.mrc.metadata.FileMetadata;
import org.xtreemfs.mrc.metadata.StripingPolicy;
import org.xtreemfs.mrc.metadata.XAttr;
import org.xtreemfs.mrc.metadata.XLoc;
import org.xtreemfs.mrc.metadata.XLocList;
import org.xtreemfs.mrc.utils.Converter;
import org.xtreemfs.mrc.utils.DBAdminHelper;
import org.xtreemfs.mrc.utils.Path;

public class BabuDBStorageManager implements StorageManager {
    
    public static final int     FILE_INDEX            = 0;
    
    public static final int     XATTRS_INDEX          = 1;
    
    public static final int     ACL_INDEX             = 2;
    
    public static final int     FILE_ID_INDEX         = 3;
    
    public static final int     LAST_ID_INDEX         = 4;
    
    public static final byte[]  LAST_ID_KEY           = { '*' };
    
    private static final String DEFAULT_SP_ATTR_NAME  = "sp";
    
    private static final String LINK_TARGET_ATTR_NAME = "lt";
    
    private BabuDB              database;
    
    private String              dbName;
    
    private String              volumeName;
    
    public BabuDBStorageManager(BabuDB database, String volumeName, String volumeId) {
        
        this.database = database;
        this.dbName = volumeId;
        this.volumeName = volumeName;
        try {
            // first, try to create a new database; if it already exists, an
            // exception will be thrown
            database.createDatabase(dbName, 5);
            
        } catch (BabuDBException e) {
            // database already exists
            Logging.logMessage(Logging.LEVEL_TRACE, this, "database '" + dbName + "' loaded");
        }
    }
    
    @Override
    public AtomicDBUpdate createAtomicDBUpdate(DBAccessResultListener listener, Object context)
        throws DatabaseException {
        try {
            return new AtomicBabuDBUpdate(database, dbName, listener == null ? null
                : new BabuDBRequestListenerWrapper(listener), context);
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public ACLEntry createACLEntry(long fileId, String entity, short rights) {
        return new BufferBackedACLEntry(fileId, entity, rights);
    }
    
    @Override
    public XLoc createXLoc(StripingPolicy stripingPolicy, String[] osds) {
        assert (stripingPolicy instanceof BufferBackedStripingPolicy);
        return new BufferBackedXLoc((BufferBackedStripingPolicy) stripingPolicy, osds);
    }
    
    @Override
    public XLocList createXLocList(XLoc[] replicas, int version) {
        BufferBackedXLoc[] tmp = new BufferBackedXLoc[replicas.length];
        for (int i = 0; i < replicas.length; i++)
            tmp[i] = (BufferBackedXLoc) replicas[i];
        return new BufferBackedXLocList(tmp, version);
    }
    
    @Override
    public StripingPolicy createStripingPolicy(String pattern, int stripeSize, int width) {
        return new BufferBackedStripingPolicy(pattern, stripeSize, width);
    }
    
    @Override
    public XAttr createXAttr(long fileId, String owner, String key, String value) {
        return new BufferBackedXAttr(fileId, owner, key, value, (short) 0);
    }
    
    @Override
    public void init(String ownerId, String owningGroupId, int perms, ACLEntry[] acl,
        org.xtreemfs.interfaces.StripingPolicy rootDirDefSp, AtomicDBUpdate update) throws DatabaseException {
        
        // atime, ctime, mtime
        int time = (int) (TimeSync.getGlobalTime() / 1000);
        
        // create the root directory; the name is the database name
        createDir(1, 0, volumeName, time, time, time, ownerId, owningGroupId, perms, 0, update);
        setLastFileId(1, update);
        
        // set the default striping policy
        if (rootDirDefSp != null)
            setDefaultStripingPolicy(1, rootDirDefSp, update);
        
        if (acl != null)
            for (ACLEntry entry : acl)
                setACLEntry(1L, entry.getEntity(), entry.getRights(), update);
    }
    
    @Override
    public long getNextFileId() throws DatabaseException {
        
        try {
            // get the file ID assigned to the last created file or
            // directory
            byte[] idBytes = BabuDBStorageHelper.getLastAssignedFileId(database, dbName);
            
            // calculate the new file ID
            ByteBuffer tmp = ByteBuffer.wrap(idBytes);
            long id = tmp.getLong(0) + 1;
            
            return id;
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public void setLastFileId(long fileId, AtomicDBUpdate update) throws DatabaseException {
        
        byte[] idBytes = new byte[8];
        ByteBuffer.wrap(idBytes).putLong(0, fileId);
        
        update.addUpdate(LAST_ID_INDEX, LAST_ID_KEY, idBytes);
    }
    
    @Override
    public FileMetadata createDir(long fileId, long parentId, String fileName, int atime, int ctime,
        int mtime, String userId, String groupId, int perms, long w32Attrs, AtomicDBUpdate update)
        throws DatabaseException {
        
        try {
            
            // get the next collision number
            short collCount = BabuDBStorageHelper.findNextFreeFileCollisionNumber(database, dbName, parentId,
                fileName);
            
            // if the file exists already, throw an exception
            if (collCount == -1)
                throw new DatabaseException(ExceptionType.FILE_EXISTS);
            
            // create metadata
            BufferBackedFileMetadata fileMetadata = new BufferBackedFileMetadata(parentId, fileName, userId,
                groupId, fileId, atime, ctime, mtime, perms, w32Attrs, (short) 1, collCount);
            
            // update main metadata in the file index
            update.addUpdate(FILE_INDEX, fileMetadata.getFCMetadataKey(), fileMetadata.getFCMetadataValue());
            update.addUpdate(FILE_INDEX, fileMetadata.getRCMetadata().getKey(), fileMetadata.getRCMetadata()
                    .getValue());
            
            // add an entry to the file ID index
            update.addUpdate(FILE_ID_INDEX, BabuDBStorageHelper.createFileIdIndexKey(fileId, (byte) 3),
                BabuDBStorageHelper.createFileIdIndexValue(parentId, fileName));
            
            return fileMetadata;
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public FileMetadata createFile(long fileId, long parentId, String fileName, int atime, int ctime,
        int mtime, String userId, String groupId, int perms, long w32Attrs, long size, boolean readOnly,
        int epoch, int issEpoch, AtomicDBUpdate update) throws DatabaseException {
        
        try {
            
            // get the next collision number
            short collCount = BabuDBStorageHelper.findNextFreeFileCollisionNumber(database, dbName, parentId,
                fileName);
            
            // if the file exists already, throw an exception
            if (collCount == -1)
                throw new DatabaseException(ExceptionType.FILE_EXISTS);
            
            // create metadata
            BufferBackedFileMetadata fileMetadata = new BufferBackedFileMetadata(parentId, fileName, userId,
                groupId, fileId, atime, ctime, mtime, size, perms, w32Attrs, (short) 1, epoch, issEpoch,
                readOnly, collCount);
            
            // update main metadata in the file index
            update.addUpdate(FILE_INDEX, fileMetadata.getFCMetadataKey(), fileMetadata.getFCMetadataValue());
            update.addUpdate(FILE_INDEX, fileMetadata.getRCMetadata().getKey(), fileMetadata.getRCMetadata()
                    .getValue());
            
            // add an entry to the file ID index
            update.addUpdate(FILE_ID_INDEX, BabuDBStorageHelper.createFileIdIndexKey(fileId, (byte) 3),
                BabuDBStorageHelper.createFileIdIndexValue(parentId, fileName));
            
            return fileMetadata;
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
        
    }
    
    @Override
    public FileMetadata createSymLink(long fileId, long parentId, String fileName, int atime, int ctime,
        int mtime, String userId, String groupId, String ref, AtomicDBUpdate update) throws DatabaseException {
        
        try {
            
            // get the next collision number
            short collCount = BabuDBStorageHelper.findNextFreeFileCollisionNumber(database, dbName, parentId,
                fileName);
            
            // if the file exists already, throw an exception
            if (collCount == -1)
                throw new DatabaseException(ExceptionType.FILE_EXISTS);
            
            // create metadata
            BufferBackedFileMetadata fileMetadata = new BufferBackedFileMetadata(parentId, fileName, userId,
                groupId, fileId, atime, ctime, mtime, ref.length(), 0777, 0, (short) 1, 0, 0, false,
                collCount);
            
            // create link target (XAttr)
            BufferBackedXAttr lt = new BufferBackedXAttr(fileId, SYSTEM_UID, LINK_TARGET_ATTR_NAME, ref,
                (short) 0);
            update.addUpdate(XATTRS_INDEX, lt.getKeyBuf(), lt.getValBuf());
            
            // update main metadata in the file index
            update.addUpdate(FILE_INDEX, fileMetadata.getFCMetadataKey(), fileMetadata.getFCMetadataValue());
            update.addUpdate(FILE_INDEX, fileMetadata.getRCMetadata().getKey(), fileMetadata.getRCMetadata()
                    .getValue());
            
            // add an entry to the file ID index
            update.addUpdate(FILE_ID_INDEX, BabuDBStorageHelper.createFileIdIndexKey(fileId, (byte) 3),
                BabuDBStorageHelper.createFileIdIndexValue(parentId, fileName));
            
            return fileMetadata;
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
        
    }
    
    @Override
    public short delete(final long parentId, final String fileName, final AtomicDBUpdate update)
        throws DatabaseException {
        
        try {
            
            // check whether the file refers to a hard link
            BufferBackedFileMetadata file = BabuDBStorageHelper.getMetadata(database, dbName, parentId,
                fileName);
            
            boolean lastLink = file.getLinkCount() == 1;
            
            // decrement the link count
            file.setLinkCount((short) (file.getLinkCount() - 1));
            
            // get all keys to delete
            List<byte[]>[] fileKeys = BabuDBStorageHelper.getKeysToDelete(database, dbName, parentId,
                fileName, lastLink);
            
            // if there are links remaining after the deletion, update
            // the link count
            if (!lastLink)
                update.addUpdate(FILE_ID_INDEX, BabuDBStorageHelper.createFileIdIndexKey(file.getId(),
                    FileMetadata.RC_METADATA), file.getRCMetadata().getValue());
            
            // add delete requests for each key
            for (int i = 0; i < fileKeys.length; i++)
                if (fileKeys[i] != null)
                    for (byte[] key : fileKeys[i])
                        update.addUpdate(i, key, null);
            
            return file.getLinkCount();
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public Iterator<ACLEntry> getACL(long fileId) throws DatabaseException {
        
        try {
            
            byte[] prefix = BabuDBStorageHelper.createACLPrefixKey(fileId, null);
            Iterator<Entry<byte[], byte[]>> it = database.directPrefixLookup(dbName, ACL_INDEX, prefix);
            
            return new ACLIterator(it);
            
        } catch (Exception exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public ACLEntry getACLEntry(long fileId, String entity) throws DatabaseException {
        
        try {
            
            byte[] key = BabuDBStorageHelper.createACLPrefixKey(fileId, entity);
            byte[] value = database.directLookup(dbName, ACL_INDEX, key);
            
            return value == null ? null : new BufferBackedACLEntry(key, value);
            
        } catch (Exception exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public Iterator<FileMetadata> getChildren(long parentId) throws DatabaseException {
        
        try {
            
            byte[] prefix = BabuDBStorageHelper.createFilePrefixKey(parentId);
            Iterator<Entry<byte[], byte[]>> it = database.directPrefixLookup(dbName, FILE_INDEX, prefix);
            
            return new ChildrenIterator(database, dbName, it);
            
        } catch (Exception exc) {
            throw new DatabaseException(exc);
        }
        
    }
    
    @Override
    public StripingPolicy getDefaultStripingPolicy(long fileId) throws DatabaseException {
        
        try {
            String spString = getXAttr(fileId, SYSTEM_UID, DEFAULT_SP_ATTR_NAME);
            if (spString == null)
                return null;
            
            return Converter.stringToStripingPolicy(this, spString);
            
        } catch (DatabaseException exc) {
            throw exc;
        } catch (Exception exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public String getVolumeId() {
        return this.dbName;
    }
    
    @Override
    public String getVolumeName() {
        return this.volumeName;
    }
    
    @Override
    public FileMetadata getMetadata(long fileId) throws DatabaseException {
        
        try {
            
            // create the key for the file ID index lookup
            byte[] key = BabuDBStorageHelper.createFileIdIndexKey(fileId, (byte) -1);
            ByteBuffer.wrap(key).putLong(fileId);
            
            byte[][] valBufs = new byte[BufferBackedFileMetadata.NUM_BUFFERS][];
            
            // retrieve the metadata from the link index
            Iterator<Entry<byte[], byte[]>> it = database.directPrefixLookup(dbName,
                BabuDBStorageManager.FILE_ID_INDEX, key);
            
            while (it.hasNext()) {
                
                Entry<byte[], byte[]> curr = it.next();
                
                int type = BabuDBStorageHelper.getType(curr.getKey(), BabuDBStorageManager.FILE_ID_INDEX);
                
                // if the value is a back link, resolve it
                if (type == 3) {
                    
                    long parentId = ByteBuffer.wrap(curr.getValue()).getLong();
                    String fileName = new String(curr.getValue(), 8, curr.getValue().length - 8);
                    
                    return getMetadata(parentId, fileName);
                }
                
                valBufs[type] = curr.getValue();
            }
            
            // if not metadata was found for the file ID, return null
            if (valBufs[FileMetadata.RC_METADATA] == null)
                return null;
            
            // otherwise, a hard link target is contained in the index; create a
            // new metadata object in this case
            return new BufferBackedFileMetadata(null, valBufs, BabuDBStorageManager.FILE_ID_INDEX);
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public FileMetadata getMetadata(final long parentId, final String fileName) throws DatabaseException {
        
        try {
            return BabuDBStorageHelper.getMetadata(database, dbName, parentId, fileName);
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public String getSoftlinkTarget(long fileId) throws DatabaseException {
        
        try {
            return getXAttr(fileId, SYSTEM_UID, LINK_TARGET_ATTR_NAME);
        } catch (DatabaseException exc) {
            throw exc;
        } catch (Exception exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public String getXAttr(long fileId, String uid, String key) throws DatabaseException {
        
        try {
            
            // peform a prefix lookup
            byte[] prefix = BabuDBStorageHelper.createXAttrPrefixKey(fileId, uid, key);
            Iterator<Entry<byte[], byte[]>> it = database.directPrefixLookup(dbName, XATTRS_INDEX, prefix);
            
            // check whether the entry is the correct one
            while (it.hasNext()) {
                
                Entry<byte[], byte[]> curr = it.next();
                BufferBackedXAttr xattr = new BufferBackedXAttr(curr.getKey(), curr.getValue());
                if (uid.equals(xattr.getOwner()) && key.equals(xattr.getKey()))
                    return xattr.getValue();
            }
            
            return null;
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public Iterator<XAttr> getXAttrs(long fileId) throws DatabaseException {
        
        try {
            
            // peform a prefix lookup
            byte[] prefix = BabuDBStorageHelper.createXAttrPrefixKey(fileId, null, null);
            Iterator<Entry<byte[], byte[]>> it = database.directPrefixLookup(dbName, XATTRS_INDEX, prefix);
            
            return new XAttrIterator(it, null);
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public Iterator<XAttr> getXAttrs(long fileId, String uid) throws DatabaseException {
        
        try {
            
            // peform a prefix lookup
            byte[] prefix = BabuDBStorageHelper.createXAttrPrefixKey(fileId, uid, null);
            Iterator<Entry<byte[], byte[]>> it = database.directPrefixLookup(dbName, XATTRS_INDEX, prefix);
            
            return new XAttrIterator(it, uid);
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
        
    }
    
    @Override
    public void link(final FileMetadata metadata, final long newParentId, final String newFileName,
        final AtomicDBUpdate update) throws DatabaseException {
        
        try {
            // get the link source
            BufferBackedFileMetadata md = (BufferBackedFileMetadata) metadata;
            
            // increment the link count
            short links = metadata.getLinkCount();
            md.setLinkCount((short) (links + 1));
            
            // insert the whole metadata of the original file in the file ID
            // index
            update.addUpdate(FILE_ID_INDEX, BabuDBStorageHelper.createFileIdIndexKey(metadata.getId(),
                FileMetadata.FC_METADATA), md.getFCMetadataValue());
            update.addUpdate(FILE_ID_INDEX, BabuDBStorageHelper.createFileIdIndexKey(metadata.getId(),
                FileMetadata.RC_METADATA), md.getRCMetadata().getValue());
            update.addUpdate(FILE_ID_INDEX, BabuDBStorageHelper.createFileIdIndexKey(metadata.getId(),
                FileMetadata.XLOC_METADATA), md.getXLocListValue());
            update.addUpdate(FILE_ID_INDEX, BabuDBStorageHelper.createFileIdIndexKey(metadata.getId(),
                (byte) 3), null);
            
            // if the metadata was retrieved from the file index and hasn't
            // been deleted before (i.e. links == 0), ensure that the original
            // file in the file index now points to the file ID index, and
            // remove
            // the FC and XLoc metadata entries
            if (links != 0 && md.getIndexId() == FILE_INDEX) {
                
                update.addUpdate(FILE_INDEX, md.getRCMetadata().getKey(), BabuDBStorageHelper
                        .createLinkTarget(metadata.getId(), metadata.getFileName()));
                update.addUpdate(FILE_INDEX, md.getFCMetadataKey(), null);
                if (md.getXLocListKey() != null)
                    update.addUpdate(FILE_INDEX, md.getXLocListKey(), null);
            }
            
            // create an entry for the new link to the metadata in the file
            // index
            
            // retrieve the next free collision number for the new link
            short collCount = BabuDBStorageHelper.findNextFreeFileCollisionNumber(database, dbName,
                newParentId, newFileName);
            
            // if a file with the same name exists already, replace it
            if (collCount == -1)
                collCount = BabuDBStorageHelper.findFileCollisionNumber(database, dbName, newParentId,
                    newFileName);
            
            update.addUpdate(FILE_INDEX, BabuDBStorageHelper.createFileKey(newParentId, newFileName,
                FileMetadata.RC_METADATA, collCount), BabuDBStorageHelper.createLinkTarget(metadata.getId(),
                newFileName));
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
        
    }
    
    @Override
    public FileMetadata[] resolvePath(final Path path) throws DatabaseException {
        
        try {
            FileMetadata[] md = new FileMetadata[path.getCompCount()];
            
            long parentId = 0;
            for (int i = 0; i < md.length; i++) {
                md[i] = BabuDBStorageHelper.getMetadata(database, dbName, parentId, path.getComp(i));
                if (md[i] == null || i < md.length - 1 && !md[i].isDirectory()) {
                    md[i] = null;
                    return md;
                }
                parentId = md[i].getId();
            }
            
            return md;
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
    }
    
    @Override
    public void setACLEntry(long fileId, String entity, Short rights, AtomicDBUpdate update)
        throws DatabaseException {
        
        BufferBackedACLEntry entry = new BufferBackedACLEntry(fileId, entity, rights == null ? 0 : rights);
        update.addUpdate(ACL_INDEX, entry.getKeyBuf(), rights == null ? null : entry.getValBuf());
    }
    
    @Override
    public void setDefaultStripingPolicy(long fileId, org.xtreemfs.interfaces.StripingPolicy defaultSp,
        AtomicDBUpdate update) throws DatabaseException {
        
        setXAttr(fileId, SYSTEM_UID, DEFAULT_SP_ATTR_NAME, Converter.stripingPolicyToString(defaultSp),
            update);
    }
    
    @Override
    public void setMetadata(FileMetadata metadata, byte type, AtomicDBUpdate update) throws DatabaseException {
        
        assert (metadata instanceof BufferBackedFileMetadata);
        BufferBackedFileMetadata md = (BufferBackedFileMetadata) metadata;
        
        int index = md.getIndexId();
        if (type == -1)
            for (byte i = 0; i < BufferBackedFileMetadata.NUM_BUFFERS; i++) {
                update.addUpdate(index, index == FILE_ID_INDEX ? BabuDBStorageHelper.createFileIdIndexKey(
                    metadata.getId(), i) : md.getKeyBuffer(i), md.getValueBuffer(i));
            }
        
        else
            update.addUpdate(index, index == FILE_ID_INDEX ? BabuDBStorageHelper.createFileIdIndexKey(
                metadata.getId(), type) : md.getKeyBuffer(type), md.getValueBuffer(type));
    }
    
    @Override
    public void setXAttr(long fileId, String uid, String key, String value, AtomicDBUpdate update)
        throws DatabaseException {
        
        try {
            short collNumber = BabuDBStorageHelper.findXAttrCollisionNumber(database, dbName, fileId, uid,
                key);
            
            BufferBackedXAttr xattr = new BufferBackedXAttr(fileId, uid, key, value, collNumber);
            update.addUpdate(XATTRS_INDEX, xattr.getKeyBuf(), value == null ? null : xattr.getValBuf());
            
        } catch (BabuDBException exc) {
            throw new DatabaseException(exc);
        }
        
    }
    
    // public void dump() throws BabuDBException {
    //        
    // System.out.println("FILE_ID_INDEX");
    //        
    // Iterator<Entry<byte[], byte[]>> it = database.directPrefixLookup(dbName,
    // FILE_ID_INDEX,
    // new byte[0]);
    // while (it.hasNext()) {
    // Entry<byte[], byte[]> next = it.next();
    // System.out.println(Arrays.toString(next.getKey()) + " = "
    // + Arrays.toString(next.getValue()));
    // }
    //        
    // System.out.println("\nFILE_INDEX");
    //        
    // it = database.directPrefixLookup(dbName, FILE_INDEX, new byte[0]);
    // while (it.hasNext()) {
    // Entry<byte[], byte[]> next = it.next();
    // System.out.println(Arrays.toString(next.getKey()) + " = "
    // + Arrays.toString(next.getValue()));
    // }
    // }
    
    public void dumpDB(BufferedWriter xmlWriter) throws DatabaseException, IOException {
        DBAdminHelper.dumpVolume(xmlWriter, this);
    }
    
}

//
// public long getDBFileSize() {
// return backend.getDBFileSize();
// }
//
// public long getNumberOfFiles() {
// return backend.getNumberOfFiles();
// }
//
// public long getNumberOfDirs() {
// return backend.getNumberOfDirs();
// }
//
// /**
// *
// * @param fileID
// * @return true, if the file with the given ID exists, false otherwise
// * @throws DatabaseException
// */
// public boolean exists(String fileID) throws DatabaseException {
// try {
// return (getFileEntity(Long.parseLong(fileID)) != null);
// } catch (NumberFormatException e) {
// throw new
// DatabaseException("StorageManager.exists(fileID) : wrong fileID-format");
// }
// }