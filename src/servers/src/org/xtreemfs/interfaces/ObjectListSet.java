package org.xtreemfs.interfaces;

import java.util.ArrayList;
import java.util.Iterator;
import org.xtreemfs.interfaces.utils.*;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;




public class ObjectListSet extends ArrayList<ObjectList>
{

    public ObjectListSet()
    { }

    public ObjectListSet( Object from_array )
    {
        this.deserialize( from_array );
    }

    public ObjectListSet( Object[] from_array )
    {
        this.deserialize( from_array );
    }        

    public String toString()
    {
        String to_string = new String();
        for ( Iterator<ObjectList> i = iterator(); i.hasNext(); )
            to_string += i.next().toString() + ", ";
        return to_string;
    }

    
    public Object serialize() 
    {
        Object[] to_array = new Object[size()];        
        for ( int value_i = 0; value_i < size(); value_i++ )
        {
            ObjectList next_value = get( value_i );                    
            to_array[value_i] = next_value.serialize();
        }
        return to_array;
    }

    public void serialize(ONCRPCBufferWriter writer) {
        if (this.size() > org.xtreemfs.interfaces.utils.XDRUtils.MAX_ARRAY_ELEMS)
        throw new IllegalArgumentException("array is too large ("+this.size()+")");
        writer.putInt( size() );
        for ( Iterator<ObjectList> i = iterator(); i.hasNext(); )
        {
            ObjectList next_value = i.next();        
            next_value.serialize( writer );;
        }
    }        


    public void deserialize( Object from_array )
    {
        this.deserialize( ( Object[] )from_array );
    }
        
    public void deserialize( Object[] from_array )
    {
        for ( int from_array_i = 0; from_array_i < from_array.length; from_array_i++ )
            this.add( new ObjectList( from_array[from_array_i] ) );
    }        

    public void deserialize( ReusableBuffer buf ) {
        int new_size = buf.getInt();
    if (new_size > org.xtreemfs.interfaces.utils.XDRUtils.MAX_ARRAY_ELEMS)
        throw new IllegalArgumentException("array is too large ("+this.size()+")");
        for ( int i = 0; i < new_size; i++ )
        {
            ObjectList new_value; new_value = new ObjectList(); new_value.deserialize( buf );;
            this.add( new_value );
        }
    } 


    public int calculateSize() {
        int my_size = Integer.SIZE/8;
        for ( Iterator<ObjectList> i = iterator(); i.hasNext(); ) {
            ObjectList value = i.next();
            my_size += value.calculateSize();
        }
        return my_size;
    }


}
