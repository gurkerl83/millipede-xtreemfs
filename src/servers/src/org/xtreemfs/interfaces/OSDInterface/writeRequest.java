package org.xtreemfs.interfaces.OSDInterface;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.utils.*;
import yidl.runtime.Marshaller;
import yidl.runtime.PrettyPrinter;
import yidl.runtime.Struct;
import yidl.runtime.Unmarshaller;




public class writeRequest extends org.xtreemfs.interfaces.utils.Request
{
    public static final int TAG = 2009112327;
    
    public writeRequest() { file_credentials = new FileCredentials(); object_data = new ObjectData();  }
    public writeRequest( FileCredentials file_credentials, String file_id, long object_number, long object_version, int offset, long lease_timeout, ObjectData object_data ) { this.file_credentials = file_credentials; this.file_id = file_id; this.object_number = object_number; this.object_version = object_version; this.offset = offset; this.lease_timeout = lease_timeout; this.object_data = object_data; }

    public FileCredentials getFile_credentials() { return file_credentials; }
    public void setFile_credentials( FileCredentials file_credentials ) { this.file_credentials = file_credentials; }
    public String getFile_id() { return file_id; }
    public void setFile_id( String file_id ) { this.file_id = file_id; }
    public long getObject_number() { return object_number; }
    public void setObject_number( long object_number ) { this.object_number = object_number; }
    public long getObject_version() { return object_version; }
    public void setObject_version( long object_version ) { this.object_version = object_version; }
    public int getOffset() { return offset; }
    public void setOffset( int offset ) { this.offset = offset; }
    public long getLease_timeout() { return lease_timeout; }
    public void setLease_timeout( long lease_timeout ) { this.lease_timeout = lease_timeout; }
    public ObjectData getObject_data() { return object_data; }
    public void setObject_data( ObjectData object_data ) { this.object_data = object_data; }

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
    public Response createDefaultResponse() { return new writeResponse(); }


    // java.io.Serializable
    public static final long serialVersionUID = 2009112327;    

    // yidl.runtime.Object
    public int getTag() { return 2009112327; }
    public String getTypeName() { return "org::xtreemfs::interfaces::OSDInterface::writeRequest"; }
    
    public int getXDRSize()
    {
        int my_size = 0;
        my_size += file_credentials.getXDRSize(); // file_credentials
        my_size += Integer.SIZE / 8 + ( file_id != null ? ( ( file_id.getBytes().length % 4 == 0 ) ? file_id.getBytes().length : ( file_id.getBytes().length + 4 - file_id.getBytes().length % 4 ) ) : 0 ); // file_id
        my_size += Long.SIZE / 8; // object_number
        my_size += Long.SIZE / 8; // object_version
        my_size += Integer.SIZE / 8; // offset
        my_size += Long.SIZE / 8; // lease_timeout
        my_size += object_data.getXDRSize(); // object_data
        return my_size;
    }    
    
    public void marshal( Marshaller marshaller )
    {
        marshaller.writeStruct( "file_credentials", file_credentials );
        marshaller.writeString( "file_id", file_id );
        marshaller.writeUint64( "object_number", object_number );
        marshaller.writeUint64( "object_version", object_version );
        marshaller.writeUint32( "offset", offset );
        marshaller.writeUint64( "lease_timeout", lease_timeout );
        marshaller.writeStruct( "object_data", object_data );
    }
    
    public void unmarshal( Unmarshaller unmarshaller ) 
    {
        file_credentials = new FileCredentials(); unmarshaller.readStruct( "file_credentials", file_credentials );
        file_id = unmarshaller.readString( "file_id" );
        object_number = unmarshaller.readUint64( "object_number" );
        object_version = unmarshaller.readUint64( "object_version" );
        offset = unmarshaller.readUint32( "offset" );
        lease_timeout = unmarshaller.readUint64( "lease_timeout" );
        object_data = new ObjectData(); unmarshaller.readStruct( "object_data", object_data );    
    }
        
    

    private FileCredentials file_credentials;
    private String file_id;
    private long object_number;
    private long object_version;
    private int offset;
    private long lease_timeout;
    private ObjectData object_data;    

}

