package org.xtreemfs.interfaces;

import java.util.Iterator;
import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.utils.*;
import yidl.Marshaller;
import yidl.Sequence;
import yidl.Unmarshaller;




public class StringSet extends Sequence<String>
{
    public StringSet() { }

    // yidl.Object
    public int getTag() { return 2009082619; }
    public String getTypeName() { return "org::xtreemfs::interfaces::StringSet"; }

    public int getXDRSize() {
        int my_size = Integer.SIZE/8;
        for ( Iterator<String> i = iterator(); i.hasNext(); ) {
            String value = i.next();
            my_size += value != null ? ( ( value.getBytes().length + Integer.SIZE/8 ) % 4 == 0 ) ? ( value.getBytes().length + Integer.SIZE/8 ) : ( value.getBytes().length + Integer.SIZE/8 + 4 - ( value.getBytes().length + Integer.SIZE/8 ) % 4 ) : 0;
        }
        return my_size;
    }
    
    public void marshal( Marshaller marshaller )
    {
        for ( Iterator<String> i = iterator(); i.hasNext(); )
            marshaller.writeString( "value", i.next() );;
    }
    
    public void unmarshal( Unmarshaller unmarshaller )
    {
        String value; 
        value = unmarshaller.readString( "value" );
        this.add( value );    
    }
        

}

