package org.xtreemfs.interfaces;

import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.utils.*;
import yidl.Marshaller;
import yidl.Struct;
import yidl.Unmarshaller;




public class InternalReadLocalResponse extends Struct
{
    public static final int TAG = 2009082670;
    
    public InternalReadLocalResponse() { data = new ObjectData(); object_set = new ObjectListSet();  }
    public InternalReadLocalResponse( ObjectData data, ObjectListSet object_set ) { this.data = data; this.object_set = object_set; }

    public ObjectData getData() { return data; }
    public void setData( ObjectData data ) { this.data = data; }
    public ObjectListSet getObject_set() { return object_set; }
    public void setObject_set( ObjectListSet object_set ) { this.object_set = object_set; }

    // java.io.Serializable
    public static final long serialVersionUID = 2009082670;    

    // yidl.Object
    public int getTag() { return 2009082670; }
    public String getTypeName() { return "org::xtreemfs::interfaces::InternalReadLocalResponse"; }
    
    public int getXDRSize()
    {
        int my_size = 0;
        my_size += data.getXDRSize();
        my_size += object_set.getXDRSize();
        return my_size;
    }    
    
    public void marshal( Marshaller marshaller )
    {
        marshaller.writeStruct( "data", data );
        marshaller.writeSequence( "object_set", object_set );
    }
    
    public void unmarshal( Unmarshaller unmarshaller ) 
    {
        data = new ObjectData(); unmarshaller.readStruct( "data", data );
        object_set = new ObjectListSet(); unmarshaller.readSequence( "object_set", object_set );    
    }
        
    

    private ObjectData data;
    private ObjectListSet object_set;    

}

