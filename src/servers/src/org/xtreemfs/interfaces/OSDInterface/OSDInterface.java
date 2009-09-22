package org.xtreemfs.interfaces.OSDInterface;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.utils.*;
import yidl.runtime.PrettyPrinter;




public class OSDInterface
{
    public static final int DEFAULT_ONCRPC_PORT = 32640;
    public static final int DEFAULT_ONCRPCS_PORT = 32640;
    public static final int DEFAULT_HTTP_PORT = 30640;


    public static int getVersion() { return 2009082918; }

    public static ONCRPCException createException( int accept_stat ) throws Exception
    {
        switch( accept_stat )
        {
            case 2009082919: return new ConcurrentModificationException();
            case 2009082920: return new errnoException();
            case 2009082921: return new InvalidArgumentException();
            case 2009082922: return new OSDException();
            case 2009082923: return new ProtocolException();
            case 2009082924: return new RedirectException();

            default: throw new Exception( "unknown accept_stat " + Integer.toString( accept_stat ) );
        }
    }

    public static Request createRequest( ONCRPCRequestHeader header ) throws Exception
    {
        switch( header.getProcedure() )
        {
            case 2009082928: return new readRequest();
            case 2009082929: return new truncateRequest();
            case 2009082930: return new unlinkRequest();
            case 2009082931: return new writeRequest();
            case 2009082938: return new xtreemfs_broadcast_gmaxRequest();
            case 2009082939: return new xtreemfs_check_objectRequest();
            case 2009082948: return new xtreemfs_cleanup_get_resultsRequest();
            case 2009082949: return new xtreemfs_cleanup_is_runningRequest();
            case 2009082950: return new xtreemfs_cleanup_startRequest();
            case 2009082951: return new xtreemfs_cleanup_statusRequest();
            case 2009082952: return new xtreemfs_cleanup_stopRequest();
            case 2009082958: return new xtreemfs_internal_get_gmaxRequest();
            case 2009082959: return new xtreemfs_internal_truncateRequest();
            case 2009082960: return new xtreemfs_internal_get_file_sizeRequest();
            case 2009082961: return new xtreemfs_internal_read_localRequest();
            case 2009082962: return new xtreemfs_internal_get_object_setRequest();
            case 2009082968: return new xtreemfs_lock_acquireRequest();
            case 2009082969: return new xtreemfs_lock_checkRequest();
            case 2009082970: return new xtreemfs_lock_releaseRequest();
            case 2009082978: return new xtreemfs_pingRequest();
            case 2009082988: return new xtreemfs_shutdownRequest();

            default: throw new Exception( "unknown request tag " + Integer.toString( header.getProcedure() ) );
        }
    }
            
    public static Response createResponse( ONCRPCResponseHeader header ) throws Exception
    {
        switch( header.getXID() )
        {
            case 2009082928: return new readResponse();            case 2009082929: return new truncateResponse();            case 2009082930: return new unlinkResponse();            case 2009082931: return new writeResponse();            case 2009082938: return new xtreemfs_broadcast_gmaxResponse();            case 2009082939: return new xtreemfs_check_objectResponse();            case 2009082948: return new xtreemfs_cleanup_get_resultsResponse();            case 2009082949: return new xtreemfs_cleanup_is_runningResponse();            case 2009082950: return new xtreemfs_cleanup_startResponse();            case 2009082951: return new xtreemfs_cleanup_statusResponse();            case 2009082952: return new xtreemfs_cleanup_stopResponse();            case 2009082958: return new xtreemfs_internal_get_gmaxResponse();            case 2009082959: return new xtreemfs_internal_truncateResponse();            case 2009082960: return new xtreemfs_internal_get_file_sizeResponse();            case 2009082961: return new xtreemfs_internal_read_localResponse();            case 2009082962: return new xtreemfs_internal_get_object_setResponse();            case 2009082968: return new xtreemfs_lock_acquireResponse();            case 2009082969: return new xtreemfs_lock_checkResponse();            case 2009082970: return new xtreemfs_lock_releaseResponse();            case 2009082978: return new xtreemfs_pingResponse();            case 2009082988: return new xtreemfs_shutdownResponse();
            default: throw new Exception( "unknown response XID " + Integer.toString( header.getXID() ) );
        }
    }    

}
