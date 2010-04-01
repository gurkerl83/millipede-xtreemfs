package org.xtreemfs.interfaces.MRCInterface;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.foundation.buffer.ReusableBuffer;
import org.xtreemfs.foundation.oncrpc.utils.*;
import org.xtreemfs.interfaces.*;
import yidl.runtime.Marshaller;
import yidl.runtime.PrettyPrinter;
import yidl.runtime.Struct;
import yidl.runtime.Unmarshaller;




public class ConcurrentModificationException extends org.xtreemfs.foundation.oncrpc.utils.ONCRPCException
{
    public static final int TAG = 2010031166;

    public ConcurrentModificationException() {  }
    public ConcurrentModificationException( String stack_trace ) { this.stack_trace = stack_trace; }

    public String getStack_trace() { return stack_trace; }
    public void setStack_trace( String stack_trace ) { this.stack_trace = stack_trace; }

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
    public static final long serialVersionUID = 2010031166;

    // yidl.runtime.Object
    public int getTag() { return 2010031166; }
    public String getTypeName() { return "org::xtreemfs::interfaces::MRCInterface::ConcurrentModificationException"; }

    public int getXDRSize()
    {
        int my_size = 0;
        my_size += Integer.SIZE / 8 + ( stack_trace != null ? ( ( stack_trace.getBytes().length % 4 == 0 ) ? stack_trace.getBytes().length : ( stack_trace.getBytes().length + 4 - stack_trace.getBytes().length % 4 ) ) : 0 ); // stack_trace
        return my_size;
    }

    public void marshal( Marshaller marshaller )
    {
        marshaller.writeString( "stack_trace", stack_trace );
    }

    public void unmarshal( Unmarshaller unmarshaller )
    {
        stack_trace = unmarshaller.readString( "stack_trace" );
    }

    private String stack_trace;
}
