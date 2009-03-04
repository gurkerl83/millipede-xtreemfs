package org.xtreemfs.interfaces.MRCInterface;

import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.MRCInterface.*;
import org.xtreemfs.interfaces.utils.*;

import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.buffer.BufferPool;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;


         

public class chownRequest implements Request
{
    public chownRequest() { context = new org.xtreemfs.interfaces.Context(); path = ""; userId = ""; groupId = ""; }
    public chownRequest( Context context, String path, String userId, String groupId ) { this.context = context; this.path = path; this.userId = userId; this.groupId = groupId; }

    public Context getContext() { return context; }
    public void setContext( Context context ) { this.context = context; }
    public String getPath() { return path; }
    public void setPath( String path ) { this.path = path; }
    public String getUserId() { return userId; }
    public void setUserId( String userId ) { this.userId = userId; }
    public String getGroupId() { return groupId; }
    public void setGroupId( String groupId ) { this.groupId = groupId; }

    // Object
    public String toString()
    {
        return "chownRequest( " + context.toString() + ", " + "\"" + path + "\"" + ", " + "\"" + userId + "\"" + ", " + "\"" + groupId + "\"" + " )";
    }    

    // Serializable
    public String getTypeName() { return "xtreemfs::interfaces::MRCInterface::chownRequest"; }    
    
    public void serialize(ONCRPCBufferWriter writer) {
        context.serialize( writer );
        { org.xtreemfs.interfaces.utils.XDRUtils.serializeString(path,writer); }
        { org.xtreemfs.interfaces.utils.XDRUtils.serializeString(userId,writer); }
        { org.xtreemfs.interfaces.utils.XDRUtils.serializeString(groupId,writer); }        
    }
    
    public void deserialize( ReusableBuffer buf )
    {
        context = new org.xtreemfs.interfaces.Context(); context.deserialize( buf );
        { path = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString(buf); }
        { userId = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString(buf); }
        { groupId = org.xtreemfs.interfaces.utils.XDRUtils.deserializeString(buf); }    
    }
    
    public int calculateSize()
    {
        int my_size = 0;
        my_size += context.calculateSize();
        my_size += 4 + ( path.length() + 4 - ( path.length() % 4 ) );
        my_size += 4 + ( userId.length() + 4 - ( userId.length() % 4 ) );
        my_size += 4 + ( groupId.length() + 4 - ( groupId.length() % 4 ) );
        return my_size;
    }

    private Context context;
    private String path;
    private String userId;
    private String groupId;
    

    // Request
    public int getInterfaceVersion() { return 2; }    
    public int getOperationNumber() { return 3; }
    public Response createDefaultResponse() { return new chownResponse(); }

}

