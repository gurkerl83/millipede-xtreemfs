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


         

public class internal_truncateResponse implements Response
{
    public internal_truncateResponse() {  }



    // Object
    public String toString()
    {
        return "internal_truncateResponse()";
    }    

    // Serializable
    public String getTypeName() { return "xtreemfs::interfaces::OSDInterface::internal_truncateResponse"; }    
    
    public void serialize(ONCRPCBufferWriter writer) {
        
    }
    
    public void deserialize( ReusableBuffer buf )
    {
    
    }
    
    public int calculateSize()
    {
        int my_size = 0;

        return my_size;
    }


    

    // Response
    public int getInterfaceVersion() { return 3; }
    public int getOperationNumber() { return 101; }    

}

