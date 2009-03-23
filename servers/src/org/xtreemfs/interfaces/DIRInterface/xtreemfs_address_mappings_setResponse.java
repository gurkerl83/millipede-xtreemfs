package org.xtreemfs.interfaces.DIRInterface;

import org.xtreemfs.interfaces.*;
import java.util.HashMap;
import org.xtreemfs.interfaces.utils.*;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;




public class xtreemfs_address_mappings_setResponse implements org.xtreemfs.interfaces.utils.Response
{
    public xtreemfs_address_mappings_setResponse() { returnValue = 0; }
    public xtreemfs_address_mappings_setResponse( long returnValue ) { this.returnValue = returnValue; }
    public xtreemfs_address_mappings_setResponse( Object from_hash_map ) { returnValue = 0; this.deserialize( from_hash_map ); }
    public xtreemfs_address_mappings_setResponse( Object[] from_array ) { returnValue = 0;this.deserialize( from_array ); }

    public long getReturnValue() { return returnValue; }
    public void setReturnValue( long returnValue ) { this.returnValue = returnValue; }

    public String getTypeName() { return "org::xtreemfs::interfaces::DIRInterface::xtreemfs_address_mappings_setResponse"; }    
    public long getTypeId() { return 2; }

    public String toString()
    {
        return "xtreemfs_address_mappings_setResponse( " + Long.toString( returnValue ) + " )";
    }


    public void deserialize( Object from_hash_map )
    {
        this.deserialize( ( HashMap<String, Object> )from_hash_map );
    }
        
    public void deserialize( HashMap<String, Object> from_hash_map )
    {
        this.returnValue = ( ( Long )from_hash_map.get( "returnValue" ) ).longValue();
    }
    
    public void deserialize( Object[] from_array )
    {
        this.returnValue = ( ( Long )from_array[0] ).longValue();        
    }

    public void deserialize( ReusableBuffer buf )
    {
        returnValue = buf.getLong();
    }

    public Object serialize()
    {
        HashMap<String, Object> to_hash_map = new HashMap<String, Object>();
        to_hash_map.put( "returnValue", new Long( returnValue ) );
        return to_hash_map;        
    }

    public void serialize( ONCRPCBufferWriter writer ) 
    {
        writer.putLong( returnValue );
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += ( Long.SIZE / 8 );
        return my_size;
    }

    // Response
    public int getOperationNumber() { return 2; }


    private long returnValue;

}
