package org.xtreemfs.interfaces.MRCInterface;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.utils.*;
import yidl.runtime.Marshaller;
import yidl.runtime.PrettyPrinter;
import yidl.runtime.Struct;
import yidl.runtime.Unmarshaller;




public class xtreemfs_internal_debugRequest extends org.xtreemfs.interfaces.utils.Request
{
    public static final int TAG = 2009120448;
    
    public xtreemfs_internal_debugRequest() {  }
    public xtreemfs_internal_debugRequest( String operation ) { this.operation = operation; }

    public String getOperation() { return operation; }
    public void setOperation( String operation ) { this.operation = operation; }

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

    // Request
    public Response createDefaultResponse() { return new xtreemfs_internal_debugResponse(); }


    // java.io.Serializable
    public static final long serialVersionUID = 2009120448;    

    // yidl.runtime.Object
    public int getTag() { return 2009120448; }
    public String getTypeName() { return "org::xtreemfs::interfaces::MRCInterface::xtreemfs_internal_debugRequest"; }
    
    public int getXDRSize()
    {
        int my_size = 0;
        my_size += Integer.SIZE / 8 + ( operation != null ? ( ( operation.getBytes().length % 4 == 0 ) ? operation.getBytes().length : ( operation.getBytes().length + 4 - operation.getBytes().length % 4 ) ) : 0 ); // operation
        return my_size;
    }    
    
    public void marshal( Marshaller marshaller )
    {
        marshaller.writeString( "operation", operation );
    }
    
    public void unmarshal( Unmarshaller unmarshaller ) 
    {
        operation = unmarshaller.readString( "operation" );    
    }
        
    

    private String operation;    

}

