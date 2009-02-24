package org.xtreemfs.interfaces;

import org.xtreemfs.interfaces.*;

import org.xtreemfs.interfaces.utils.*;

import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.buffer.BufferPool;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;


         
   
public class DirectoryEntry implements org.xtreemfs.interfaces.utils.Serializable
{
    public DirectoryEntry() { path = ""; stbuf = new org.xtreemfs.interfaces.stat_(); }
    public DirectoryEntry( String path, stat_ stbuf ) { this.path = path; this.stbuf = stbuf; }

    public String getPath() { return path; }
    public void setPath( String path ) { this.path = path; }
    public stat_ getStbuf() { return stbuf; }
    public void setStbuf( stat_ stbuf ) { this.stbuf = stbuf; }

    // Object
    public String toString()
    {
        return "DirectoryEntry( " + "\"" + path + "\"" + ", " + stbuf.toString() + " )";
    }    

    // Serializable
    public String getTypeName() { return "xtreemfs::interfaces::DirectoryEntry"; }    
    
    public void serialize(ONCRPCBufferWriter writer) {
        { final byte[] bytes = path.getBytes(); writer.putInt( bytes.length ); writer.put( bytes );  if (bytes.length % 4 > 0) {for (int k = 0; k < (4 - (bytes.length % 4)); k++) { writer.put((byte)0); } }}
        stbuf.serialize( writer );        
    }
    
    public void deserialize( ReusableBuffer buf )
    {
        { int path_new_length = buf.getInt(); byte[] path_new_bytes = new byte[path_new_length]; buf.get( path_new_bytes ); path = new String( path_new_bytes ); if (path_new_length % 4 > 0) {for (int k = 0; k < (4 - (path_new_length % 4)); k++) { buf.get(); } } }
        stbuf = new org.xtreemfs.interfaces.stat_(); stbuf.deserialize( buf );    
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += 4 + ( path.length() + 4 - ( path.length() % 4 ) );
        my_size += stbuf.calculateSize();
        return my_size;
    }

    private String path;
    private stat_ stbuf;

}

