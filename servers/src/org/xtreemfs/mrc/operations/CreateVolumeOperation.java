/*  Copyright (c) 2008 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

 This file is part of XtreemFS. XtreemFS is part of XtreemOS, a Linux-based
 Grid Operating System, see <http://www.xtreemos.eu> for more details.
 The XtreemOS project has been developed with the financial support of the
 European Commission's IST program under contract #FP6-033576.

 XtreemFS is free software: you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation, either version 2 of the License, or (at your option)
 any later version.

 XtreemFS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with XtreemFS. If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * AUTHORS: Jan Stender (ZIB)
 */

package org.xtreemfs.mrc.operations;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.foundation.json.JSONException;
import org.xtreemfs.foundation.json.JSONParser;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.foundation.oncrpc.client.RPCResponseAvailableListener;
import org.xtreemfs.interfaces.Constants;
import org.xtreemfs.interfaces.KeyValuePair;
import org.xtreemfs.interfaces.KeyValuePairSet;
import org.xtreemfs.interfaces.ServiceRegistry;
import org.xtreemfs.interfaces.ServiceRegistrySet;
import org.xtreemfs.mrc.ErrNo;
import org.xtreemfs.mrc.ErrorRecord;
import org.xtreemfs.mrc.MRCRequest;
import org.xtreemfs.mrc.MRCRequestDispatcher;
import org.xtreemfs.mrc.UserException;
import org.xtreemfs.mrc.ErrorRecord.ErrorClass;
import org.xtreemfs.mrc.ac.YesToAnyoneFileAccessPolicy;
import org.xtreemfs.mrc.osdselection.RandomSelectionPolicy;

/**
 * 
 * @author stender
 */
public class CreateVolumeOperation extends MRCOperation {
    
    static class Args {
        
        public String               volumeName;
        
        private short               osdSelectionPolicyId;
        
        private Map<String, Object> defaultStripingPolicy;
        
        private short               acPolicyId;
    }
    
    public static final String RPC_NAME = "createVolume";
    
    public CreateVolumeOperation(MRCRequestDispatcher master) {
        super(master);
    }
    
    @Override
    public boolean hasArguments() {
        return true;
    }
    
    @Override
    public boolean isAuthRequired() {
        return true;
    }
    
    @Override
    public void startRequest(final MRCRequest rq) {
        
        try {
            
            final Args rqArgs = (Args) rq.getRequestArgs();
            
            // first, check whether the given policies are supported
            
            if (master.getOSDStatusManager().getOSDSelectionPolicy(rqArgs.osdSelectionPolicyId) == null)
                throw new UserException(ErrNo.EINVAL, "invalid OSD selection policy ID: "
                    + rqArgs.osdSelectionPolicyId);
            
            if (master.getFileAccessManager().getFileAccessPolicy(rqArgs.acPolicyId) == null)
                throw new UserException(ErrNo.EINVAL, "invalid file access policy ID: "
                    + rqArgs.acPolicyId);
            
            // in order to allow volume creation in a single-threaded
            // non-blocking
            // manner, it needs to be performed in two steps:
            // * first, the volume is registered with the directory service
            // * when registration has been confirmed at the directory service,
            // request processing is continued with step 2
            
            final String volumeId = master.getVolumeManager().newVolumeId();
            
            // check whether a volume with the same name has already been
            // registered at the Directory Service
            
            Map<String, Object> queryMap = new HashMap<String, Object>();
            queryMap.put("name", rqArgs.volumeName);
            List<String> attrs = new LinkedList<String>();
            attrs.add("version");
            
            RPCResponse<ServiceRegistrySet> response = master.getDirClient()
                    .service_get_by_type(null,Constants.SERVICE_TYPE_VOLUME);
            response.registerListener(new RPCResponseAvailableListener<ServiceRegistrySet>() {

                @Override
                public void responseAvailable(RPCResponse<ServiceRegistrySet> r) {
                    processStep2(rqArgs, volumeId, rq, r);
                }
            });
            
        } catch (UserException exc) {
            Logging.logMessage(Logging.LEVEL_TRACE, this, exc);
            finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, exc.getErrno(), exc
                    .getMessage(), exc));
        } catch (Exception exc) {
            finishRequest(rq, new ErrorRecord(ErrorClass.INTERNAL_SERVER_ERROR,
                "an error has occurred", exc));
        }
    }
    
    private void processStep2(final Args rqArgs, final String volumeId, final MRCRequest rq,
        RPCResponse<ServiceRegistrySet> rpcResponse) {
        
        try {
            
            // check the response; if a volume with the same name has already
            // been registered, return an error
            
            ServiceRegistrySet response = rpcResponse.get();
            
            // check if the volume already exists
            if (!response.isEmpty()) {
                
                String uuid = response.get(0).getUuid();
                throw new UserException(ErrNo.EEXIST, "volume '" + rqArgs.volumeName
                    + "' already exists in Directory Service, id='" + uuid + "'");
            }
            
            // otherwise, register the volume at the Directory Service

            KeyValuePairSet kvset = new KeyValuePairSet();
            kvset.add(new KeyValuePair("mrc", master.getConfig().getUUID().toString()));
            kvset.add(new KeyValuePair("free", "0"));
            ServiceRegistry vol = new ServiceRegistry(volumeId, 0, Constants.SERVICE_TYPE_VOLUME, rqArgs.volumeName, kvset);
            
            RPCResponse<Long> rpcResponse2 = master.getDirClient().service_register(null, vol);
            rpcResponse2.registerListener(new RPCResponseAvailableListener<Long>() {

                @Override
                public void responseAvailable(RPCResponse<Long> r) {
                    processStep3(rqArgs, volumeId, rq, r);
                }
            });
            
        } catch (UserException exc) {
            Logging.logMessage(Logging.LEVEL_TRACE, this, exc);
            finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, exc.getErrno(), exc
                    .getMessage(), exc));
        } catch (Exception exc) {
            finishRequest(rq, new ErrorRecord(ErrorClass.INTERNAL_SERVER_ERROR,
                "an error has occurred", exc));
        }
    }
    
    public void processStep3(final Args rqArgs, final String volumeId, final MRCRequest rq,
        RPCResponse<Long> rpcResponse) {
        
        try {
            
            // check whether an exception has occured; if so, an exception is
            // thrown when trying to parse the response
            
            rpcResponse.get();
            
            // FIXME: this line is needed due to a BUG in the client which
            // expects some useless return value
            rq.setData(ReusableBuffer.wrap(JSONParser.writeJSON(null).getBytes()));
            
            // create the volume and its database
            master.getVolumeManager().createVolume(master.getFileAccessManager(), volumeId,
                rqArgs.volumeName, rqArgs.acPolicyId, rqArgs.osdSelectionPolicyId, null,
                rq.getDetails().userId, rq.getDetails().groupIds.get(0),
                rqArgs.defaultStripingPolicy);
            
            finishRequest(rq);
            
        } catch (UserException exc) {
            Logging.logMessage(Logging.LEVEL_TRACE, this, exc);
            finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, exc.getErrno(), exc
                    .getMessage(), exc));
        } catch (Exception exc) {
            finishRequest(rq, new ErrorRecord(ErrorClass.INTERNAL_SERVER_ERROR,
                "an error has occurred", exc));
        }
    }
    
    @Override
    public ErrorRecord parseRPCBody(MRCRequest rq, List<Object> arguments) {
        
        Args args = new Args();
        
        try {
            
            args.volumeName = (String) arguments.get(0);
            if (arguments.size() == 1) {
                
                args.defaultStripingPolicy = new HashMap<String, Object>();
                args.defaultStripingPolicy.put("policy", "RAID0");
                args.defaultStripingPolicy.put("stripe-size", Long.valueOf(64));
                args.defaultStripingPolicy.put("width", Long.valueOf(1));
                
                args.osdSelectionPolicyId = RandomSelectionPolicy.POLICY_ID;
                args.acPolicyId = YesToAnyoneFileAccessPolicy.POLICY_ID;
                
                return null;
            }
            
            args.osdSelectionPolicyId = ((Long) arguments.get(1)).shortValue();
            args.defaultStripingPolicy = (Map<String, Object>) arguments.get(2);
            args.acPolicyId = ((Long) arguments.get(3)).shortValue();
            
            if (arguments.size() == 6)
                return null;
            
            throw new Exception();
            
        } catch (Exception exc) {
            try {
                return new ErrorRecord(ErrorClass.BAD_REQUEST, "invalid arguments for operation '"
                    + getClass().getSimpleName() + "': " + JSONParser.writeJSON(arguments));
            } catch (JSONException je) {
                Logging.logMessage(Logging.LEVEL_ERROR, this, exc);
                return new ErrorRecord(ErrorClass.BAD_REQUEST, "invalid arguments for operation '"
                    + getClass().getSimpleName() + "'");
            }
        } finally {
            rq.setRequestArgs(args);
        }
    }
    
}
