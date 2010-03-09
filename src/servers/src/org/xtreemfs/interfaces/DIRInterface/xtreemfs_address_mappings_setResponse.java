package org.xtreemfs.interfaces.DIRInterface;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.utils.*;
import yidl.runtime.Marshaller;
import yidl.runtime.PrettyPrinter;
import yidl.runtime.Struct;
import yidl.runtime.Unmarshaller;




public class xtreemfs_address_mappings_setResponse extends org.xtreemfs.interfaces.utils.Response
{
    public static final int TAG = 2010031019;

    public xtreemfs_address_mappings_setResponse() {  }
    public xtreemfs_address_mappings_setResponse( long returnValue ) { this.returnValue = returnValue; }

    public long getReturnValue() { return returnValue; }
    public void setReturnValue( long returnValue ) { this.returnValue = returnValue; }

    // java.lang.Object
    public String toString() 
    { 
        StringWriter string_writer = new StringWriter();
        string_writer.append(this.getClass().getCanonicalName());
        string_writer.append(" ");
        PrettyPrinter pretty_printer = new PrettyPrinter( string_writer );
        pretty_printer.writeStruct( "", this );
        return string_writer.toString();
    }


    // java.io.Serializable
    public static final long serialVersionUID = 2010031019;

    // yidl.runtime.Object
    public int getTag() { return 2010031019; }
    public String getTypeName() { return "org::xtreemfs::interfaces::DIRInterface::xtreemfs_address_mappings_setResponse"; }

    public int getXDRSize()
    {
        int my_size = 0;
        my_size += Long.SIZE / 8; // returnValue
        return my_size;
    }

    public void marshal( Marshaller marshaller )
    {
        marshaller.writeUint64( "returnValue", returnValue );
    }

    public void unmarshal( Unmarshaller unmarshaller )
    {
        returnValue = unmarshaller.readUint64( "returnValue" );
    }

    

    private long returnValue;

}

