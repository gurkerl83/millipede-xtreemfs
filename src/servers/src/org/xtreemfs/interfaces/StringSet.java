package org.xtreemfs.interfaces;

import java.util.ArrayList;
import java.util.Iterator;
import org.xtreemfs.interfaces.utils.*;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;




public class StringSet extends ArrayList<String>
{

    public StringSet()
    { }

    public StringSet( Object from_array )
    {
        this.deserialize( from_array );
    }

    public StringSet( Object[] from_array )
    {
        this.deserialize( from_array );
    }        

    public String toString()
    {
        String to_string = new String();
        for ( Iterator<String> i = iterator(); i.hasNext(); )
            to_string += "\"" + i.next() + "\"" + ", ";
        return to_string;
    }

    
    public Object serialize() 
    {
        Object[] to_array = new Object[size()];        
        for ( int value_i = 0; value_i < size(); value_i++ )
        {
            String next_value = get( value_i );                    
            to_array[value_i] = next_value;
        }
        return to_array;
    }

    public void serialize(ONCRPCBufferWriter writer) {
        if (this.size() > org.xtreemfs.interfaces.utils.XDRUtils.MAX_ARRAY_ELEMS)
        throw new IllegalArgumentException("array is too large ("+this.size()+")");
        writer.putInt( size() );
        for ( Iterator<String> i = iterator(); i.hasNext(); )
        {
            String next_value = i.next();        
            org.xtreemfs.interfaces.utils.XDRUtils.serializeString( next_value, writer );;
        }
    }        


    public void deserialize( Object from_array )
    {
        this.deserialize( ( Object[] )from_array );
    }
        
    public void deserialize( Object[] from_array )
    {
        for ( int from_array_i = 0; from_array_i < from_array.length; from_array_i++ )
            this.add( ( String )from_array[from_array_i] );
    }        

    public void deserialize( ReusableBuffer buf ) {
        int new_size = buf.getInt();
    if (new_size > org.xtreemfs.interfaces.utils.XDRUtils.MAX_ARRAY_ELEMS)
        throw new IllegalArgumentException("array is too large ("+this.size()+")");
        for ( int i = 0; i < new_size; i++ )
        {
            String new_value; new_value = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );;
            this.add( new_value );
        }
    } 


    public int calculateSize() {
        int my_size = Integer.SIZE/8;
        for ( Iterator<String> i = iterator(); i.hasNext(); ) {
            String value = i.next();
            my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(value);
        }
        return my_size;
    }


}
