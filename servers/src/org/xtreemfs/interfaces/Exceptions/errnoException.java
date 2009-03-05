package org.xtreemfs.interfaces.Exceptions;

import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.Exceptions.*;
import org.xtreemfs.interfaces.utils.*;

import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.buffer.BufferPool;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;


         
public class errnoException extends org.xtreemfs.interfaces.utils.ONCRPCException 
{
    public errnoException() { error_code = 0; errro_message = ""; stack_trace = ""; }
    public errnoException( int error_code, String errro_message, String stack_trace ) { this.error_code = error_code; this.errro_message = errro_message; this.stack_trace = stack_trace; }

    public int getError_code() { return error_code; }
    public void setError_code( int error_code ) { this.error_code = error_code; }
    public String getErrro_message() { return errro_message; }
    public void setErrro_message( String errro_message ) { this.errro_message = errro_message; }
    public String getStack_trace() { return stack_trace; }
    public void setStack_trace( String stack_trace ) { this.stack_trace = stack_trace; }

    // Object
    public String toString()
    {
        return "errnoException( " + Integer.toString( error_code ) + ", " + "\"" + errro_message + "\"" + ", " + "\"" + stack_trace + "\"" + " )";
    }    

    // Serializable
    public String getTypeName() { return "xtreemfs::interfaces::Exceptions::errnoException"; }    
    
    public void serialize(ONCRPCBufferWriter writer) {
        writer.putInt( error_code );
        { org.xtreemfs.interfaces.utils.XDRUtils.serializeString(errro_message,writer); }
        { org.xtreemfs.interfaces.utils.XDRUtils.serializeString(stack_trace,writer); }        
    }
    
    public void deserialize( ReusableBuffer buf )
    {
        error_code = buf.getInt();
        { errro_message = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString(buf); }
        { stack_trace = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString(buf); }    
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += ( Integer.SIZE / 8 );
        my_size += 4 + ( errro_message.length() + 4 - ( errro_message.length() % 4 ) );
        my_size += 4 + ( stack_trace.length() + 4 - ( stack_trace.length() % 4 ) );
        return my_size;
    }

    private int error_code;
    private String errro_message;
    private String stack_trace;
    
}
