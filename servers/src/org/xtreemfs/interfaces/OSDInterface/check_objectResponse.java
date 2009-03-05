package org.xtreemfs.interfaces.OSDInterface;

import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.OSDInterface.*;
import org.xtreemfs.interfaces.utils.*;

import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.buffer.BufferPool;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;


         

public class check_objectResponse implements Response
{
    public check_objectResponse() { returnValue = new org.xtreemfs.interfaces.ObjectData(); }
    public check_objectResponse( ObjectData returnValue ) { this.returnValue = returnValue; }

    public ObjectData getReturnValue() { return returnValue; }
    public void setReturnValue( ObjectData returnValue ) { this.returnValue = returnValue; }

    // Object
    public String toString()
    {
        return "check_objectResponse( " + returnValue.toString() + " )";
    }    

    // Serializable
    public String getTypeName() { return "xtreemfs::interfaces::OSDInterface::check_objectResponse"; }    
    
    public void serialize(ONCRPCBufferWriter writer) {
        returnValue.serialize( writer );        
    }
    
    public void deserialize( ReusableBuffer buf )
    {
        returnValue = new org.xtreemfs.interfaces.ObjectData(); returnValue.deserialize( buf );    
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += returnValue.calculateSize();
        return my_size;
    }

    private ObjectData returnValue;
    

    // Response
    public int getInterfaceVersion() { return 3; }
    public int getOperationNumber() { return 103; }    

}
