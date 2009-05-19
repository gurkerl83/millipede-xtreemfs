package org.xtreemfs.interfaces.DIRInterface;

import org.xtreemfs.interfaces.*;
import java.util.HashMap;
import org.xtreemfs.interfaces.utils.*;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;




public class xtreemfs_address_mappings_setRequest implements org.xtreemfs.interfaces.utils.Request
{
    public static final int TAG = 1102;

    
    public xtreemfs_address_mappings_setRequest() { address_mappings = new AddressMappingSet(); }
    public xtreemfs_address_mappings_setRequest( AddressMappingSet address_mappings ) { this.address_mappings = address_mappings; }
    public xtreemfs_address_mappings_setRequest( Object from_hash_map ) { address_mappings = new AddressMappingSet(); this.deserialize( from_hash_map ); }
    public xtreemfs_address_mappings_setRequest( Object[] from_array ) { address_mappings = new AddressMappingSet();this.deserialize( from_array ); }

    public AddressMappingSet getAddress_mappings() { return address_mappings; }
    public void setAddress_mappings( AddressMappingSet address_mappings ) { this.address_mappings = address_mappings; }

    // Object
    public String toString()
    {
        return "xtreemfs_address_mappings_setRequest( " + address_mappings.toString() + " )";
    }

    // Serializable
    public int getTag() { return 1102; }
    public String getTypeName() { return "org::xtreemfs::interfaces::DIRInterface::xtreemfs_address_mappings_setRequest"; }

    public void deserialize( Object from_hash_map )
    {
        this.deserialize( ( HashMap<String, Object> )from_hash_map );
    }
        
    public void deserialize( HashMap<String, Object> from_hash_map )
    {
        this.address_mappings.deserialize( ( Object[] )from_hash_map.get( "address_mappings" ) );
    }
    
    public void deserialize( Object[] from_array )
    {
        this.address_mappings.deserialize( ( Object[] )from_array[0] );        
    }

    public void deserialize( ReusableBuffer buf )
    {
        address_mappings = new AddressMappingSet(); address_mappings.deserialize( buf );
    }

    public Object serialize()
    {
        HashMap<String, Object> to_hash_map = new HashMap<String, Object>();
        to_hash_map.put( "address_mappings", address_mappings.serialize() );
        return to_hash_map;        
    }

    public void serialize( ONCRPCBufferWriter writer ) 
    {
        address_mappings.serialize( writer );
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += address_mappings.calculateSize();
        return my_size;
    }

    // Request
    public Response createDefaultResponse() { return new xtreemfs_address_mappings_setResponse(); }


    private AddressMappingSet address_mappings;    

}

