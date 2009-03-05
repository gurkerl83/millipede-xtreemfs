package org.xtreemfs.interfaces.OSDInterface;

import org.xtreemfs.interfaces.utils.ONCRPCRequestHeader;
import org.xtreemfs.interfaces.utils.ONCRPCResponseHeader;
import org.xtreemfs.interfaces.utils.Request;
import org.xtreemfs.interfaces.utils.Response;
import org.xtreemfs.interfaces.utils.ONCRPCException;
import org.xtreemfs.interfaces.Exceptions.*;


public class OSDInterface
{
    public static int getVersion() { return 3; }

    public static Request createRequest( ONCRPCRequestHeader header ) throws Exception
    {
        switch( header.getOperationNumber() )
        {
            case 1: return new readRequest();
            case 2: return new truncateRequest();
            case 3: return new unlinkRequest();
            case 4: return new writeRequest();
            case 100: return new internal_get_gmaxRequest();
            case 101: return new internal_truncateRequest();
            case 102: return new internal_read_localRequest();
            case 103: return new check_objectRequest();
            case 50: return new admin_shutdownRequest();
            default: throw new Exception( "unknown request number " + Integer.toString( header.getOperationNumber() ) );
        }
    }
            
    public static Response createResponse( ONCRPCResponseHeader header ) throws Exception
    {
        switch( header.getXID() )
        {
            case 1: return new readResponse();
            case 2: return new truncateResponse();
            case 3: return new unlinkResponse();
            case 4: return new writeResponse();
            case 100: return new internal_get_gmaxResponse();
            case 101: return new internal_truncateResponse();
            case 102: return new internal_read_localResponse();
            case 103: return new check_objectResponse();
            case 50: return new admin_shutdownResponse();
            default: throw new Exception( "unknown response number " + Integer.toString( header.getXID() ) );
        }
    }    
}