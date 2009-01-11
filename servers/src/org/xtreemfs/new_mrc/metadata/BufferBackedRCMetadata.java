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

package org.xtreemfs.new_mrc.metadata;

import java.nio.ByteBuffer;

public class BufferBackedRCMetadata {
    
    public static final byte   TYPE_ID                  = 1;
    
    protected static final int RC_TYPE                  = 0;
    
    protected static final int RC_ID                    = 1;
    
    protected static final int RC_PERMS                 = 9;
    
    protected static final int RC_LINKCOUNT             = 11;
    
    protected static final int RC_EPOCH                 = 13;
    
    protected static final int RC_ISSEPOCH              = 17;
    
    protected static final int RC_READONLY              = 21;
    
    protected static final int RC_DIR_OWNER_OFFSET      = 11;
    
    protected static final int RC_DIR_GROUP_OFFSET      = 13;
    
    protected static final int DIR_VAR_LEN_PART_OFFSET  = 15;
    
    protected static final int RC_FILE_OWNER_OFFSET     = 22;
    
    protected static final int RC_FILE_GROUP_OFFSET     = 24;
    
    protected static final int FILE_VAR_LEN_PART_OFFSET = 26;
    
    private final short        ownerOffset;
    
    private final short        groupOffset;
    
    private final boolean      directory;
    
    private final ByteBuffer   keyBuf;
    
    private final ByteBuffer   valBuf;
    
    /**
     * Creates a new buffer-backed metadata object from a key and a value
     * buffer.
     * 
     * @param keyBuf
     *            the database key
     * @param valBuf
     *            the database value
     */
    public BufferBackedRCMetadata(byte[] keyBuf, byte[] valBuf) {
        
        // assign the key and value
        this.keyBuf = ByteBuffer.wrap(keyBuf);
        this.valBuf = ByteBuffer.wrap(valBuf);
        
        directory = valBuf[0] == 1;
        ownerOffset = this.valBuf.getShort(directory ? RC_DIR_OWNER_OFFSET : RC_FILE_OWNER_OFFSET);
        groupOffset = this.valBuf.getShort(directory ? RC_DIR_GROUP_OFFSET : RC_FILE_GROUP_OFFSET);
    }
    
    /**
     * Creates a new buffer-backed file metadata object.
     * 
     * @param parentId
     * @param fileName
     * @param ownerId
     * @param groupId
     * @param fileId
     * @param perms
     * @param linkCount
     * @param epoch
     * @param issEpoch
     * @param readOnly
     */
    public BufferBackedRCMetadata(long parentId, String fileName, String ownerId, String groupId,
        long fileId, short perms, short linkCount, int epoch, int issEpoch, boolean readOnly,
        short collCount) {
        
        // assign the key
        keyBuf = generateKeyBuf(parentId, fileName, TYPE_ID, collCount);
        
        // assign the value
        byte[] fnBytes = fileName.getBytes();
        byte[] oBytes = ownerId.getBytes();
        byte[] gBytes = groupId.getBytes();
        
        final int bufSize = FILE_VAR_LEN_PART_OFFSET + fnBytes.length + oBytes.length
            + gBytes.length;
        ownerOffset = (short) (bufSize - oBytes.length - gBytes.length);
        groupOffset = (short) (bufSize - gBytes.length);
        
        valBuf = ByteBuffer.wrap(new byte[bufSize]);
        valBuf.put((byte) 0).putLong(fileId).putShort(perms).putShort(linkCount).putInt(epoch)
                .putInt(issEpoch).put((byte) (readOnly ? 1 : 0)).putShort(ownerOffset).putShort(
                    groupOffset).put(fnBytes).put(oBytes).put(gBytes);
        
        directory = false;
    }
    
    /**
     * Constructor for creating a new directory metadata object.
     * 
     * @param parentId
     * @param dirName
     * @param ownerId
     * @param groupId
     * @param fileId
     * @param perms
     */
    public BufferBackedRCMetadata(long parentId, String dirName, String ownerId, String groupId,
        long fileId, short perms, short collCount) {
        
        // assign the key
        keyBuf = generateKeyBuf(parentId, dirName, TYPE_ID, collCount);
        
        // assign the value
        byte[] fnBytes = dirName.getBytes();
        byte[] oBytes = ownerId.getBytes();
        byte[] gBytes = groupId.getBytes();
        
        final int bufSize = DIR_VAR_LEN_PART_OFFSET + fnBytes.length + oBytes.length
            + gBytes.length;
        ownerOffset = (short) (bufSize - oBytes.length - gBytes.length);
        groupOffset = (short) (bufSize - gBytes.length);
        
        valBuf = ByteBuffer.wrap(new byte[bufSize]);
        valBuf.put((byte) 1).putLong(fileId).putShort(perms).putShort(ownerOffset).putShort(
            groupOffset).put(fnBytes).put(oBytes).put(gBytes);
        
        directory = true;
    }
    
    public long getId() {
        return valBuf.getLong(RC_ID);
    }
    
    public byte getType() {
        return valBuf.get();
    }
    
    public int getEpoch() {
        return valBuf.getInt(RC_EPOCH);
    }
    
    public int getIssuedEpoch() {
        return valBuf.getInt(RC_ISSEPOCH);
    }
    
    public short getLinkCount() {
        return valBuf.getShort(RC_LINKCOUNT);
    }
    
    public short getPerms() {
        return valBuf.getShort(RC_PERMS);
    }
    
    public boolean isReadOnly() {
        return valBuf.get(RC_READONLY) != 0;
    }
    
    public void setId(long id) {
        valBuf.putLong(RC_ID, id);
    }
    
    public void setEpoch(int epoch) {
        valBuf.putInt(RC_EPOCH, epoch);
    }
    
    public void setIssuedEpoch(int issuedEpoch) {
        valBuf.putInt(RC_ISSEPOCH, issuedEpoch);
    }
    
    public void setLinkCount(short linkCount) {
        valBuf.putShort(RC_LINKCOUNT, linkCount);
    }
    
    public void setPerms(short perms) {
        valBuf.putShort(RC_PERMS, perms);
    }
    
    public void setReadOnly(boolean readOnly) {
        valBuf.put(RC_READONLY, (byte) (readOnly ? 1 : 0));
    }
    
    public String getFileName() {
        int index = directory ? DIR_VAR_LEN_PART_OFFSET : FILE_VAR_LEN_PART_OFFSET;
        int length = ownerOffset - index;
        return new String(valBuf.array(), index, length);
    }
    
    public String getOwnerId() {
        int index = ownerOffset;
        int length = groupOffset - index;
        return new String(valBuf.array(), index, length);
    }
    
    public String getOwningGroupId() {
        int index = groupOffset;
        int length = valBuf.limit() - index;
        return new String(valBuf.array(), index, length);
    }
    
    public boolean isDirectory() {
        return directory;
    }
    
    // public void setXLocList(BufferBackedXLocList xloc) {
    // assert (!directory) : "cannot assign locations list to directory";
    // keyBufs[XLOC_METADATA] = generateKeyBuf(parentId, fileName,
    // XLOC_METADATA);
    // valBufs[XLOC_METADATA] = ByteBuffer.wrap(xloc.getBuffer());
    // }
    
    public byte[] getKey() {
        return keyBuf.array();
    }
    
    public byte[] getValue() {
        return valBuf.array();
    }
    
    private ByteBuffer generateKeyBuf(long parentId, String fileName, int type, short collCount) {
        
        byte[] tmp = new byte[collCount == 0 ? 13 : 15];
        ByteBuffer buf = ByteBuffer.wrap(tmp);
        buf.putLong(parentId).putInt(fileName.hashCode()).put((byte) type);
        if (collCount != 0)
            buf.putShort(collCount);
        
        return buf;
    }
}