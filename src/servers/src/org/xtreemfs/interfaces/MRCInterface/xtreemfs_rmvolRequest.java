package org.xtreemfs.interfaces.MRCInterface;

import org.xtreemfs.interfaces.*;
import java.util.HashMap;
import org.xtreemfs.interfaces.utils.*;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;




public class xtreemfs_rmvolRequest implements org.xtreemfs.interfaces.utils.Request
{
    public static final int TAG = 1244;

    
    public xtreemfs_rmvolRequest() { volume_name = ""; }
    public xtreemfs_rmvolRequest( String volume_name ) { this.volume_name = volume_name; }
    public xtreemfs_rmvolRequest( Object from_hash_map ) { volume_name = ""; this.deserialize( from_hash_map ); }
    public xtreemfs_rmvolRequest( Object[] from_array ) { volume_name = "";this.deserialize( from_array ); }

    public String getVolume_name() { return volume_name; }
    public void setVolume_name( String volume_name ) { this.volume_name = volume_name; }

    // Object
    public String toString()
    {
        return "xtreemfs_rmvolRequest( " + "\"" + volume_name + "\"" + " )";
    }

    // Serializable
    public int getTag() { return 1244; }
    public String getTypeName() { return "org::xtreemfs::interfaces::MRCInterface::xtreemfs_rmvolRequest"; }

    public void deserialize( Object from_hash_map )
    {
        this.deserialize( ( HashMap<String, Object> )from_hash_map );
    }
        
    public void deserialize( HashMap<String, Object> from_hash_map )
    {
        this.volume_name = ( String )from_hash_map.get( "volume_name" );
    }
    
    public void deserialize( Object[] from_array )
    {
        this.volume_name = ( String )from_array[0];        
    }

    public void deserialize( ReusableBuffer buf )
    {
        volume_name = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );
    }

    public Object serialize()
    {
        HashMap<String, Object> to_hash_map = new HashMap<String, Object>();
        to_hash_map.put( "volume_name", volume_name );
        return to_hash_map;        
    }

    public void serialize( ONCRPCBufferWriter writer ) 
    {
        org.xtreemfs.interfaces.utils.XDRUtils.serializeString( volume_name, writer );
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(volume_name);
        return my_size;
    }

    // Request
    public Response createDefaultResponse() { return new xtreemfs_rmvolResponse(); }


    private String volume_name;    

}

