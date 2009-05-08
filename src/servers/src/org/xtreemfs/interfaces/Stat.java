package org.xtreemfs.interfaces;

import java.util.HashMap;
import org.xtreemfs.interfaces.utils.*;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;




public class Stat implements org.xtreemfs.interfaces.utils.Serializable
{
    public Stat() { mode = 0; nlink = 0; uid = 0; gid = 0; unused_dev = 0; size = 0; atime_ns = 0; mtime_ns = 0; ctime_ns = 0; user_id = ""; group_id = ""; file_id = ""; link_target = ""; truncate_epoch = 0; attributes = 0; }
    public Stat( int mode, int nlink, int uid, int gid, int unused_dev, long size, long atime_ns, long mtime_ns, long ctime_ns, String user_id, String group_id, String file_id, String link_target, int truncate_epoch, int attributes ) { this.mode = mode; this.nlink = nlink; this.uid = uid; this.gid = gid; this.unused_dev = unused_dev; this.size = size; this.atime_ns = atime_ns; this.mtime_ns = mtime_ns; this.ctime_ns = ctime_ns; this.user_id = user_id; this.group_id = group_id; this.file_id = file_id; this.link_target = link_target; this.truncate_epoch = truncate_epoch; this.attributes = attributes; }
    public Stat( Object from_hash_map ) { mode = 0; nlink = 0; uid = 0; gid = 0; unused_dev = 0; size = 0; atime_ns = 0; mtime_ns = 0; ctime_ns = 0; user_id = ""; group_id = ""; file_id = ""; link_target = ""; truncate_epoch = 0; attributes = 0; this.deserialize( from_hash_map ); }
    public Stat( Object[] from_array ) { mode = 0; nlink = 0; uid = 0; gid = 0; unused_dev = 0; size = 0; atime_ns = 0; mtime_ns = 0; ctime_ns = 0; user_id = ""; group_id = ""; file_id = ""; link_target = ""; truncate_epoch = 0; attributes = 0;this.deserialize( from_array ); }

    public int getMode() { return mode; }
    public void setMode( int mode ) { this.mode = mode; }
    public int getNlink() { return nlink; }
    public void setNlink( int nlink ) { this.nlink = nlink; }
    public int getUid() { return uid; }
    public void setUid( int uid ) { this.uid = uid; }
    public int getGid() { return gid; }
    public void setGid( int gid ) { this.gid = gid; }
    public int getUnused_dev() { return unused_dev; }
    public void setUnused_dev( int unused_dev ) { this.unused_dev = unused_dev; }
    public long getSize() { return size; }
    public void setSize( long size ) { this.size = size; }
    public long getAtime_ns() { return atime_ns; }
    public void setAtime_ns( long atime_ns ) { this.atime_ns = atime_ns; }
    public long getMtime_ns() { return mtime_ns; }
    public void setMtime_ns( long mtime_ns ) { this.mtime_ns = mtime_ns; }
    public long getCtime_ns() { return ctime_ns; }
    public void setCtime_ns( long ctime_ns ) { this.ctime_ns = ctime_ns; }
    public String getUser_id() { return user_id; }
    public void setUser_id( String user_id ) { this.user_id = user_id; }
    public String getGroup_id() { return group_id; }
    public void setGroup_id( String group_id ) { this.group_id = group_id; }
    public String getFile_id() { return file_id; }
    public void setFile_id( String file_id ) { this.file_id = file_id; }
    public String getLink_target() { return link_target; }
    public void setLink_target( String link_target ) { this.link_target = link_target; }
    public int getTruncate_epoch() { return truncate_epoch; }
    public void setTruncate_epoch( int truncate_epoch ) { this.truncate_epoch = truncate_epoch; }
    public int getAttributes() { return attributes; }
    public void setAttributes( int attributes ) { this.attributes = attributes; }

    public String getTypeName() { return "org::xtreemfs::interfaces::Stat"; }    
    public long getTypeId() { return 0; }

    public String toString()
    {
        return "Stat( " + Integer.toString( mode ) + ", " + Integer.toString( nlink ) + ", " + Integer.toString( uid ) + ", " + Integer.toString( gid ) + ", " + Integer.toString( unused_dev ) + ", " + Long.toString( size ) + ", " + Long.toString( atime_ns ) + ", " + Long.toString( mtime_ns ) + ", " + Long.toString( ctime_ns ) + ", " + "\"" + user_id + "\"" + ", " + "\"" + group_id + "\"" + ", " + "\"" + file_id + "\"" + ", " + "\"" + link_target + "\"" + ", " + Integer.toString( truncate_epoch ) + ", " + Integer.toString( attributes ) + " )";
    }


    public void deserialize( Object from_hash_map )
    {
        this.deserialize( ( HashMap<String, Object> )from_hash_map );
    }
        
    public void deserialize( HashMap<String, Object> from_hash_map )
    {
        this.mode = ( ( Integer )from_hash_map.get( "mode" ) ).intValue();
        this.nlink = ( ( Integer )from_hash_map.get( "nlink" ) ).intValue();
        this.uid = ( ( Integer )from_hash_map.get( "uid" ) ).intValue();
        this.gid = ( ( Integer )from_hash_map.get( "gid" ) ).intValue();
        this.unused_dev = ( ( Integer )from_hash_map.get( "unused_dev" ) ).intValue();
        this.size = ( ( Long )from_hash_map.get( "size" ) ).longValue();
        this.atime_ns = ( ( Long )from_hash_map.get( "atime_ns" ) ).longValue();
        this.mtime_ns = ( ( Long )from_hash_map.get( "mtime_ns" ) ).longValue();
        this.ctime_ns = ( ( Long )from_hash_map.get( "ctime_ns" ) ).longValue();
        this.user_id = ( String )from_hash_map.get( "user_id" );
        this.group_id = ( String )from_hash_map.get( "group_id" );
        this.file_id = ( String )from_hash_map.get( "file_id" );
        this.link_target = ( String )from_hash_map.get( "link_target" );
        this.truncate_epoch = ( ( Integer )from_hash_map.get( "truncate_epoch" ) ).intValue();
        this.attributes = ( ( Integer )from_hash_map.get( "attributes" ) ).intValue();
    }
    
    public void deserialize( Object[] from_array )
    {
        this.mode = ( ( Integer )from_array[0] ).intValue();
        this.nlink = ( ( Integer )from_array[1] ).intValue();
        this.uid = ( ( Integer )from_array[2] ).intValue();
        this.gid = ( ( Integer )from_array[3] ).intValue();
        this.unused_dev = ( ( Integer )from_array[4] ).intValue();
        this.size = ( ( Long )from_array[5] ).longValue();
        this.atime_ns = ( ( Long )from_array[6] ).longValue();
        this.mtime_ns = ( ( Long )from_array[7] ).longValue();
        this.ctime_ns = ( ( Long )from_array[8] ).longValue();
        this.user_id = ( String )from_array[9];
        this.group_id = ( String )from_array[10];
        this.file_id = ( String )from_array[11];
        this.link_target = ( String )from_array[12];
        this.truncate_epoch = ( ( Integer )from_array[13] ).intValue();
        this.attributes = ( ( Integer )from_array[14] ).intValue();        
    }

    public void deserialize( ReusableBuffer buf )
    {
        mode = buf.getInt();
        nlink = buf.getInt();
        uid = buf.getInt();
        gid = buf.getInt();
        unused_dev = buf.getInt();
        size = buf.getLong();
        atime_ns = buf.getLong();
        mtime_ns = buf.getLong();
        ctime_ns = buf.getLong();
        user_id = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );
        group_id = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );
        file_id = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );
        link_target = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );
        truncate_epoch = buf.getInt();
        attributes = buf.getInt();
    }

    public Object serialize()
    {
        HashMap<String, Object> to_hash_map = new HashMap<String, Object>();
        to_hash_map.put( "mode", new Integer( mode ) );
        to_hash_map.put( "nlink", new Integer( nlink ) );
        to_hash_map.put( "uid", new Integer( uid ) );
        to_hash_map.put( "gid", new Integer( gid ) );
        to_hash_map.put( "unused_dev", new Integer( unused_dev ) );
        to_hash_map.put( "size", new Long( size ) );
        to_hash_map.put( "atime_ns", new Long( atime_ns ) );
        to_hash_map.put( "mtime_ns", new Long( mtime_ns ) );
        to_hash_map.put( "ctime_ns", new Long( ctime_ns ) );
        to_hash_map.put( "user_id", user_id );
        to_hash_map.put( "group_id", group_id );
        to_hash_map.put( "file_id", file_id );
        to_hash_map.put( "link_target", link_target );
        to_hash_map.put( "truncate_epoch", new Integer( truncate_epoch ) );
        to_hash_map.put( "attributes", new Integer( attributes ) );
        return to_hash_map;        
    }

    public void serialize( ONCRPCBufferWriter writer ) 
    {
        writer.putInt( mode );
        writer.putInt( nlink );
        writer.putInt( uid );
        writer.putInt( gid );
        writer.putInt( unused_dev );
        writer.putLong( size );
        writer.putLong( atime_ns );
        writer.putLong( mtime_ns );
        writer.putLong( ctime_ns );
        org.xtreemfs.interfaces.utils.XDRUtils.serializeString( user_id, writer );
        org.xtreemfs.interfaces.utils.XDRUtils.serializeString( group_id, writer );
        org.xtreemfs.interfaces.utils.XDRUtils.serializeString( file_id, writer );
        org.xtreemfs.interfaces.utils.XDRUtils.serializeString( link_target, writer );
        writer.putInt( truncate_epoch );
        writer.putInt( attributes );
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += ( Integer.SIZE / 8 );
        my_size += ( Integer.SIZE / 8 );
        my_size += ( Integer.SIZE / 8 );
        my_size += ( Integer.SIZE / 8 );
        my_size += ( Integer.SIZE / 8 );
        my_size += ( Long.SIZE / 8 );
        my_size += ( Long.SIZE / 8 );
        my_size += ( Long.SIZE / 8 );
        my_size += ( Long.SIZE / 8 );
        my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(user_id);
        my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(group_id);
        my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(file_id);
        my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(link_target);
        my_size += ( Integer.SIZE / 8 );
        my_size += ( Integer.SIZE / 8 );
        return my_size;
    }


    private int mode;
    private int nlink;
    private int uid;
    private int gid;
    private int unused_dev;
    private long size;
    private long atime_ns;
    private long mtime_ns;
    private long ctime_ns;
    private String user_id;
    private String group_id;
    private String file_id;
    private String link_target;
    private int truncate_epoch;
    private int attributes;

}
