package org.xtreemfs.interfaces.MRCInterface;

import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.utils.*;
import yidl.Marshaller;
import yidl.Struct;
import yidl.Unmarshaller;




public class accessResponse extends org.xtreemfs.interfaces.utils.Response
{
    public static final int TAG = 2009090410;
    
    public accessResponse() {  }
    public accessResponse( boolean returnValue ) { this.returnValue = returnValue; }

    public boolean getReturnValue() { return returnValue; }
    public void setReturnValue( boolean returnValue ) { this.returnValue = returnValue; }

    // java.io.Serializable
    public static final long serialVersionUID = 2009090410;    

    // yidl.Object
    public int getTag() { return 2009090410; }
    public String getTypeName() { return "org::xtreemfs::interfaces::MRCInterface::accessResponse"; }
    
    public int getXDRSize()
    {
        int my_size = 0;
        my_size += 4;
        return my_size;
    }    
    
    public void marshal( Marshaller marshaller )
    {
        marshaller.writeBoolean( "returnValue", returnValue );
    }
    
    public void unmarshal( Unmarshaller unmarshaller ) 
    {
        returnValue = unmarshaller.readBoolean( "returnValue" );    
    }
        
    

    private boolean returnValue;    

}

