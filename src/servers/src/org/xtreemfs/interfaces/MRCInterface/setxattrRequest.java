package org.xtreemfs.interfaces.MRCInterface;

import org.xtreemfs.interfaces.*;
import java.util.HashMap;
import org.xtreemfs.interfaces.utils.*;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;




public class setxattrRequest implements org.xtreemfs.interfaces.utils.Request
{
    public setxattrRequest() { path = ""; name = ""; value = ""; flags = 0; }
    public setxattrRequest( String path, String name, String value, int flags ) { this.path = path; this.name = name; this.value = value; this.flags = flags; }
    public setxattrRequest( Object from_hash_map ) { path = ""; name = ""; value = ""; flags = 0; this.deserialize( from_hash_map ); }
    public setxattrRequest( Object[] from_array ) { path = ""; name = ""; value = ""; flags = 0;this.deserialize( from_array ); }

    public String getPath() { return path; }
    public void setPath( String path ) { this.path = path; }
    public String getName() { return name; }
    public void setName( String name ) { this.name = name; }
    public String getValue() { return value; }
    public void setValue( String value ) { this.value = value; }
    public int getFlags() { return flags; }
    public void setFlags( int flags ) { this.flags = flags; }

    public String getTypeName() { return "org::xtreemfs::interfaces::MRCInterface::setxattrRequest"; }    
    public long getTypeId() { return 18; }

    public String toString()
    {
        return "setxattrRequest( " + "\"" + path + "\"" + ", " + "\"" + name + "\"" + ", " + "\"" + value + "\"" + ", " + Integer.toString( flags ) + " )";
    }


    public void deserialize( Object from_hash_map )
    {
        this.deserialize( ( HashMap<String, Object> )from_hash_map );
    }
        
    public void deserialize( HashMap<String, Object> from_hash_map )
    {
        this.path = ( String )from_hash_map.get( "path" );
        this.name = ( String )from_hash_map.get( "name" );
        this.value = ( String )from_hash_map.get( "value" );
        this.flags = ( ( Integer )from_hash_map.get( "flags" ) ).intValue();
    }
    
    public void deserialize( Object[] from_array )
    {
        this.path = ( String )from_array[0];
        this.name = ( String )from_array[1];
        this.value = ( String )from_array[2];
        this.flags = ( ( Integer )from_array[3] ).intValue();        
    }

    public void deserialize( ReusableBuffer buf )
    {
        path = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );
        name = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );
        value = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString( buf );
        flags = buf.getInt();
    }

    public Object serialize()
    {
        HashMap<String, Object> to_hash_map = new HashMap<String, Object>();
        to_hash_map.put( "path", path );
        to_hash_map.put( "name", name );
        to_hash_map.put( "value", value );
        to_hash_map.put( "flags", new Integer( flags ) );
        return to_hash_map;        
    }

    public void serialize( ONCRPCBufferWriter writer ) 
    {
        org.xtreemfs.interfaces.utils.XDRUtils.serializeString( path, writer );
        org.xtreemfs.interfaces.utils.XDRUtils.serializeString( name, writer );
        org.xtreemfs.interfaces.utils.XDRUtils.serializeString( value, writer );
        writer.putInt( flags );
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(path);
        my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(name);
        my_size += org.xtreemfs.interfaces.utils.XDRUtils.stringLengthPadded(value);
        my_size += ( Integer.SIZE / 8 );
        return my_size;
    }

    // Request
    public int getOperationNumber() { return 18; }
    public Response createDefaultResponse() { return new setxattrResponse(); }


    private String path;
    private String name;
    private String value;
    private int flags;

}
