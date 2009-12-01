package org.xtreemfs.interfaces.OSDInterface;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.utils.*;
import yidl.runtime.PrettyPrinter;




public class OSDInterface
{
    public static final int HTTP_PORT_DEFAULT = 30640;
    public static final int ONCRPC_PORT_DEFAULT = 32640;
    public static final int ONCRPCG_PORT_DEFAULT = 32640;
    public static final int ONCRPCS_PORT_DEFAULT = 32640;
    public static final int ONCRPCU_PORT_DEFAULT = 32640;


    public static int getVersion() { return 2009120411; }

    public static ONCRPCException createException( int accept_stat ) throws Exception
    {
        switch( accept_stat )
        {
            case 2009120412: return new ConcurrentModificationException();
            case 2009120413: return new errnoException();
            case 2009120414: return new InvalidArgumentException();
            case 2009120415: return new OSDException();
            case 2009120416: return new ProtocolException();
            case 2009120417: return new RedirectException();

            default: throw new Exception( "unknown accept_stat " + Integer.toString( accept_stat ) );
        }
    }

    public static Request createRequest( ONCRPCRequestHeader header ) throws Exception
    {
        switch( header.getProcedure() )
        {
            case 2009120421: return new readRequest();
            case 2009120422: return new truncateRequest();
            case 2009120423: return new unlinkRequest();
            case 2009120424: return new writeRequest();
            case 2009120431: return new xtreemfs_broadcast_gmaxRequest();
            case 2009120432: return new xtreemfs_check_objectRequest();
            case 2009120441: return new xtreemfs_cleanup_get_resultsRequest();
            case 2009120442: return new xtreemfs_cleanup_is_runningRequest();
            case 2009120443: return new xtreemfs_cleanup_startRequest();
            case 2009120444: return new xtreemfs_cleanup_statusRequest();
            case 2009120445: return new xtreemfs_cleanup_stopRequest();
            case 2009120451: return new xtreemfs_internal_get_gmaxRequest();
            case 2009120452: return new xtreemfs_internal_truncateRequest();
            case 2009120453: return new xtreemfs_internal_get_file_sizeRequest();
            case 2009120454: return new xtreemfs_internal_read_localRequest();
            case 2009120455: return new xtreemfs_internal_get_object_setRequest();
            case 2009120461: return new xtreemfs_lock_acquireRequest();
            case 2009120462: return new xtreemfs_lock_checkRequest();
            case 2009120463: return new xtreemfs_lock_releaseRequest();
            case 2009120471: return new xtreemfs_pingRequest();
            case 2009120481: return new xtreemfs_shutdownRequest();

            default: throw new Exception( "unknown request tag " + Integer.toString( header.getProcedure() ) );
        }
    }
            
    public static Response createResponse( ONCRPCResponseHeader header ) throws Exception
    {
        switch( header.getXID() )
        {
            case 2009120421: return new readResponse();            case 2009120422: return new truncateResponse();            case 2009120423: return new unlinkResponse();            case 2009120424: return new writeResponse();            case 2009120431: return new xtreemfs_broadcast_gmaxResponse();            case 2009120432: return new xtreemfs_check_objectResponse();            case 2009120441: return new xtreemfs_cleanup_get_resultsResponse();            case 2009120442: return new xtreemfs_cleanup_is_runningResponse();            case 2009120443: return new xtreemfs_cleanup_startResponse();            case 2009120444: return new xtreemfs_cleanup_statusResponse();            case 2009120445: return new xtreemfs_cleanup_stopResponse();            case 2009120451: return new xtreemfs_internal_get_gmaxResponse();            case 2009120452: return new xtreemfs_internal_truncateResponse();            case 2009120453: return new xtreemfs_internal_get_file_sizeResponse();            case 2009120454: return new xtreemfs_internal_read_localResponse();            case 2009120455: return new xtreemfs_internal_get_object_setResponse();            case 2009120461: return new xtreemfs_lock_acquireResponse();            case 2009120462: return new xtreemfs_lock_checkResponse();            case 2009120463: return new xtreemfs_lock_releaseResponse();            case 2009120471: return new xtreemfs_pingResponse();            case 2009120481: return new xtreemfs_shutdownResponse();
            default: throw new Exception( "unknown response XID " + Integer.toString( header.getXID() ) );
        }
    }    

}
