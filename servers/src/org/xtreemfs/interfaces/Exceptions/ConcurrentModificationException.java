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


         
public class ConcurrentModificationException extends Exception implements org.xtreemfs.interfaces.utils.Serializable
{
    public ConcurrentModificationException() { stack_trace = ""; }
    public ConcurrentModificationException( String stack_trace ) { this.stack_trace = stack_trace; }

    public String getStack_trace() { return stack_trace; }
    public void setStack_trace( String stack_trace ) { this.stack_trace = stack_trace; }

    // Object
    public String toString()
    {
        return "ConcurrentModificationException( " + "\"" + stack_trace + "\"" + " )";
    }    

    // Serializable
    public String getTypeName() { return "xtreemfs::interfaces::Exceptions::ConcurrentModificationException"; }    
    
    public void serialize(ONCRPCBufferWriter writer) {
        { final byte[] bytes = stack_trace.getBytes(); writer.putInt( bytes.length ); writer.put( bytes );  if (bytes.length % 4 > 0) {for (int k = 0; k < (4 - (bytes.length % 4)); k++) { writer.put((byte)0); } }}        
    }
    
    public void deserialize( ReusableBuffer buf )
    {
        { int stack_trace_new_length = buf.getInt(); byte[] stack_trace_new_bytes = new byte[stack_trace_new_length]; buf.get( stack_trace_new_bytes ); stack_trace = new String( stack_trace_new_bytes ); if (stack_trace_new_length % 4 > 0) {for (int k = 0; k < (4 - (stack_trace_new_length % 4)); k++) { buf.get(); } } }    
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += 4 + ( stack_trace.length() + 4 - ( stack_trace.length() % 4 ) );
        return my_size;
    }

    private String stack_trace;
    
}

