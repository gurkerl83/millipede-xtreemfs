package org.xtreemfs.interfaces;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.utils.*;
import yidl.Marshaller;
import yidl.PrettyPrinter;
import yidl.Struct;
import yidl.Unmarshaller;




public class StatVFS implements Struct
{
    public static final int TAG = 2009090252;
    
    public StatVFS() {  }
    public StatVFS( int bsize, long bavail, long blocks, String fsid, int namelen ) { this.bsize = bsize; this.bavail = bavail; this.blocks = blocks; this.fsid = fsid; this.namelen = namelen; }

    public int getBsize() { return bsize; }
    public void setBsize( int bsize ) { this.bsize = bsize; }
    public long getBavail() { return bavail; }
    public void setBavail( long bavail ) { this.bavail = bavail; }
    public long getBlocks() { return blocks; }
    public void setBlocks( long blocks ) { this.blocks = blocks; }
    public String getFsid() { return fsid; }
    public void setFsid( String fsid ) { this.fsid = fsid; }
    public int getNamelen() { return namelen; }
    public void setNamelen( int namelen ) { this.namelen = namelen; }

    // java.lang.Object
    public String toString() 
    { 
        StringWriter string_writer = new StringWriter();
        PrettyPrinter pretty_printer = new PrettyPrinter( string_writer );
        pretty_printer.writeStruct( "", this );
        return string_writer.toString();
    }


    // java.io.Serializable
    public static final long serialVersionUID = 2009090252;    

    // yidl.Object
    public int getTag() { return 2009090252; }
    public String getTypeName() { return "org::xtreemfs::interfaces::StatVFS"; }
    
    public int getXDRSize()
    {
        int my_size = 0;
        my_size += Integer.SIZE / 8; // bsize
        my_size += Long.SIZE / 8; // bavail
        my_size += Long.SIZE / 8; // blocks
        my_size += Integer.SIZE / 8 + ( fsid != null ? ( ( fsid.getBytes().length % 4 == 0 ) ? fsid.getBytes().length : ( fsid.getBytes().length + 4 - fsid.getBytes().length % 4 ) ) : 0 ); // fsid
        my_size += Integer.SIZE / 8; // namelen
        return my_size;
    }    
    
    public void marshal( Marshaller marshaller )
    {
        marshaller.writeUint32( "bsize", bsize );
        marshaller.writeUint64( "bavail", bavail );
        marshaller.writeUint64( "blocks", blocks );
        marshaller.writeString( "fsid", fsid );
        marshaller.writeUint32( "namelen", namelen );
    }
    
    public void unmarshal( Unmarshaller unmarshaller ) 
    {
        bsize = unmarshaller.readUint32( "bsize" );
        bavail = unmarshaller.readUint64( "bavail" );
        blocks = unmarshaller.readUint64( "blocks" );
        fsid = unmarshaller.readString( "fsid" );
        namelen = unmarshaller.readUint32( "namelen" );    
    }
        
    

    private int bsize;
    private long bavail;
    private long blocks;
    private String fsid;
    private int namelen;    

}

