package org.xtreemfs.interfaces;

import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.utils.*;

import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.buffer.BufferPool;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;


         
public class AddressMappingSet extends ArrayList<AddressMapping>
{    
    // Serializable
    public String getTypeName() { return "xtreemfs::interfaces::AddressMappingSet"; }
    
    public void serialize(ONCRPCBufferWriter writer) {
        if (this.size() > org.xtreemfs.interfaces.utils.XDRUtils.MAX_ARRAY_ELEMS)
	    throw new IllegalArgumentException("array is too large ("+this.size()+")");
        writer.putInt( size() );
        for ( Iterator<AddressMapping> i = iterator(); i.hasNext(); )
        {
            AddressMapping next_value = i.next();        
            next_value.serialize( writer );;
        }
    }

    public void deserialize( ReusableBuffer buf ) {
        int new_size = buf.getInt();
	if (new_size > org.xtreemfs.interfaces.utils.XDRUtils.MAX_ARRAY_ELEMS)
	    throw new IllegalArgumentException("array is too large ("+this.size()+")");
        for ( int i = 0; i < new_size; i++ )
        {
            AddressMapping new_value; new_value = new org.xtreemfs.interfaces.AddressMapping(); new_value.deserialize( buf );;
            this.add( new_value );
        }
    }
    
    public int calculateSize() {
        int my_size = Integer.SIZE/8;
        for ( Iterator<AddressMapping> i = iterator(); i.hasNext(); ) {
            AddressMapping next_value = i.next();
            my_size += next_value.calculateSize();
        }
        return my_size;
    }
}

