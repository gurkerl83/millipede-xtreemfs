package org.xtreemfs.interfaces;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.foundation.buffer.ReusableBuffer;
import org.xtreemfs.foundation.oncrpc.utils.*;
import yidl.runtime.Marshaller;
import yidl.runtime.PrettyPrinter;
import yidl.runtime.Struct;
import yidl.runtime.Unmarshaller;




public class XCap implements Struct
{
    public static final int TAG = 2010030939;

    public XCap() { snap_config = SnapConfig.SNAP_CONFIG_SNAPS_DISABLED;  }
    public XCap( int access_mode, String client_identity, long expire_time_s, int expire_timeout_s, String file_id, boolean replicate_on_close, String server_signature, int truncate_epoch, SnapConfig snap_config, long snap_timestamp ) { this.access_mode = access_mode; this.client_identity = client_identity; this.expire_time_s = expire_time_s; this.expire_timeout_s = expire_timeout_s; this.file_id = file_id; this.replicate_on_close = replicate_on_close; this.server_signature = server_signature; this.truncate_epoch = truncate_epoch; this.snap_config = snap_config; this.snap_timestamp = snap_timestamp; }

    public int getAccess_mode() { return access_mode; }
    public String getClient_identity() { return client_identity; }
    public long getExpire_time_s() { return expire_time_s; }
    public int getExpire_timeout_s() { return expire_timeout_s; }
    public String getFile_id() { return file_id; }
    public boolean getReplicate_on_close() { return replicate_on_close; }
    public String getServer_signature() { return server_signature; }
    public int getTruncate_epoch() { return truncate_epoch; }
    public SnapConfig getSnap_config() { return snap_config; }
    public long getSnap_timestamp() { return snap_timestamp; }
    public void setAccess_mode( int access_mode ) { this.access_mode = access_mode; }
    public void setClient_identity( String client_identity ) { this.client_identity = client_identity; }
    public void setExpire_time_s( long expire_time_s ) { this.expire_time_s = expire_time_s; }
    public void setExpire_timeout_s( int expire_timeout_s ) { this.expire_timeout_s = expire_timeout_s; }
    public void setFile_id( String file_id ) { this.file_id = file_id; }
    public void setReplicate_on_close( boolean replicate_on_close ) { this.replicate_on_close = replicate_on_close; }
    public void setServer_signature( String server_signature ) { this.server_signature = server_signature; }
    public void setTruncate_epoch( int truncate_epoch ) { this.truncate_epoch = truncate_epoch; }
    public void setSnap_config( SnapConfig snap_config ) { this.snap_config = snap_config; }
    public void setSnap_timestamp( long snap_timestamp ) { this.snap_timestamp = snap_timestamp; }

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
    public static final long serialVersionUID = 2010030939;

    // yidl.runtime.Object
    public int getTag() { return 2010030939; }
    public String getTypeName() { return "org::xtreemfs::interfaces::XCap"; }

    public int getXDRSize()
    {
        int my_size = 0;
        my_size += Integer.SIZE / 8; // access_mode
        my_size += Integer.SIZE / 8 + ( client_identity != null ? ( ( client_identity.getBytes().length % 4 == 0 ) ? client_identity.getBytes().length : ( client_identity.getBytes().length + 4 - client_identity.getBytes().length % 4 ) ) : 0 ); // client_identity
        my_size += Long.SIZE / 8; // expire_time_s
        my_size += Integer.SIZE / 8; // expire_timeout_s
        my_size += Integer.SIZE / 8 + ( file_id != null ? ( ( file_id.getBytes().length % 4 == 0 ) ? file_id.getBytes().length : ( file_id.getBytes().length + 4 - file_id.getBytes().length % 4 ) ) : 0 ); // file_id
        my_size += Integer.SIZE / 8; // replicate_on_close
        my_size += Integer.SIZE / 8 + ( server_signature != null ? ( ( server_signature.getBytes().length % 4 == 0 ) ? server_signature.getBytes().length : ( server_signature.getBytes().length + 4 - server_signature.getBytes().length % 4 ) ) : 0 ); // server_signature
        my_size += Integer.SIZE / 8; // truncate_epoch
        my_size += Integer.SIZE / 8; // snap_config
        my_size += Long.SIZE / 8; // snap_timestamp
        return my_size;
    }

    public void marshal( Marshaller marshaller )
    {
        marshaller.writeUint32( "access_mode", access_mode );
        marshaller.writeString( "client_identity", client_identity );
        marshaller.writeUint64( "expire_time_s", expire_time_s );
        marshaller.writeUint32( "expire_timeout_s", expire_timeout_s );
        marshaller.writeString( "file_id", file_id );
        marshaller.writeBoolean( "replicate_on_close", replicate_on_close );
        marshaller.writeString( "server_signature", server_signature );
        marshaller.writeUint32( "truncate_epoch", truncate_epoch );
        marshaller.writeInt32( snap_config, snap_config.intValue() );
        marshaller.writeUint64( "snap_timestamp", snap_timestamp );
    }

    public void unmarshal( Unmarshaller unmarshaller )
    {
        access_mode = unmarshaller.readUint32( "access_mode" );
        client_identity = unmarshaller.readString( "client_identity" );
        expire_time_s = unmarshaller.readUint64( "expire_time_s" );
        expire_timeout_s = unmarshaller.readUint32( "expire_timeout_s" );
        file_id = unmarshaller.readString( "file_id" );
        replicate_on_close = unmarshaller.readBoolean( "replicate_on_close" );
        server_signature = unmarshaller.readString( "server_signature" );
        truncate_epoch = unmarshaller.readUint32( "truncate_epoch" );
        snap_config = SnapConfig.parseInt( unmarshaller.readInt32( "snap_config" ) );
        snap_timestamp = unmarshaller.readUint64( "snap_timestamp" );
    }

    private int access_mode;
    private String client_identity;
    private long expire_time_s;
    private int expire_timeout_s;
    private String file_id;
    private boolean replicate_on_close;
    private String server_signature;
    private int truncate_epoch;
    private SnapConfig snap_config;
    private long snap_timestamp;
}
