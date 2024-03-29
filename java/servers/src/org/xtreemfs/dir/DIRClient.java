/*
 * Copyright (c) 2011 by Bjoern Kolbeck,
 *               Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */


package org.xtreemfs.dir;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.List;
import org.xtreemfs.foundation.TimeServerClient;
import org.xtreemfs.foundation.logging.Logging;
import org.xtreemfs.foundation.logging.Logging.Category;
import org.xtreemfs.foundation.pbrpc.client.PBRPCException;
import org.xtreemfs.foundation.pbrpc.client.RPCResponse;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.Auth;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.AuthType;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.ErrorType;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.UserCredentials;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.AddressMapping;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.AddressMappingSet;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.Configuration;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.Service;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.ServiceSet;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.ServiceType;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.addressMappingSetResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.configurationSetResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.globalTimeSGetResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.serviceRegisterResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.DIRServiceClient;

/**
 * DIR Client with automatic fail-over and redirect support.
 * All operations are synchronous.
 * @author bjko
 */
public class DIRClient implements TimeServerClient {

    /**
     * Generated DIR service rpc client.
     */
    protected DIRServiceClient rpcClient;

    /**
     * List of DIR servers.
     */
    protected final InetSocketAddress[] servers;

    /**
     * number of server currently used.
     */
    protected int currentServer;

    /**
     * Maximum number of retries.
     */
    protected final int maxRetries;

    /**
     * Time to wait between retries (in milliseconds).
     */
    protected final int retryWaitMs;

    /**
     * True, if the client was redirected before.
     */
    protected boolean isRedirect;

    /**
     * Auth used for TimeServerClient calls.
     */
    protected final Auth auth;

    /**
     * UserCredentials used for TimeServerClient calls.
     */
    protected final UserCredentials user;

    /**
     * Initializes the DIRClient.
     * @param rpcClient RPC client (must be running).
     * @param servers list of servers to use (must contain at least one item).
     * @param maxRetries maximum retries before throwing exception.
     * @param retryWaitMs time (milliseconds) to wait between retries.
     */
    public DIRClient(DIRServiceClient rpcClient,
                     InetSocketAddress[] servers,
                     int maxRetries,
                     int retryWaitMs) {
        if (servers.length == 0) {
            throw new IllegalArgumentException("Must provide at least one directory service address.");
        }
        this.maxRetries = maxRetries;
        this.servers = servers;
        this.rpcClient = rpcClient;
        this.currentServer = 0;
        this.retryWaitMs = retryWaitMs;
        auth = Auth.newBuilder().setAuthType(AuthType.AUTH_NONE).build();
        user = UserCredentials.newBuilder().setUsername("service").addGroups("xtreemfs").build();
    }

    public AddressMappingSet xtreemfs_address_mappings_get(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final String uuid) throws IOException, InterruptedException {
        AddressMappingSet response = (AddressMappingSet) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_address_mappings_get(server, authHeader, userCreds, uuid);
            }
        });
        return response;
    }

    public void xtreemfs_address_mappings_remove(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final String uuid) throws IOException, InterruptedException {
        Object response = syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_address_mappings_remove(server, authHeader, userCreds, uuid);
            }
        });
    }

    public addressMappingSetResponse xtreemfs_address_mappings_set(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final List<AddressMapping> mappings) throws IOException, InterruptedException {
        addressMappingSetResponse response = (addressMappingSetResponse) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_address_mappings_set(server, authHeader, userCreds, mappings);
            }
        });
        return response;
    }

    public addressMappingSetResponse xtreemfs_address_mappings_set(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final AddressMappingSet input) throws IOException, InterruptedException {
        addressMappingSetResponse response = (addressMappingSetResponse) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_address_mappings_set(server, authHeader, userCreds, input);
            }
        });
        return response;
    }

    public void xtreemfs_service_deregister(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final String uuid) throws IOException, InterruptedException {
        Object response = syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_service_deregister(server, authHeader, userCreds, uuid);
            }
        });
    }

    public void xtreemfs_service_offline(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final String name) throws IOException, InterruptedException {
        Object response = syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_service_offline(server, authHeader, userCreds, name);
            }
        });
    }

    public ServiceSet xtreemfs_service_get_by_name(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final String name) throws IOException, InterruptedException {
        ServiceSet response = (ServiceSet) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_service_get_by_name(server, authHeader, userCreds, name);
            }
        });
        return response;
    }

    public ServiceSet xtreemfs_service_get_by_uuid(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final String uuid) throws IOException, InterruptedException {
        ServiceSet response = (ServiceSet) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_service_get_by_uuid(server, authHeader, userCreds, uuid);
            }
        });
        return response;
    }

    public ServiceSet xtreemfs_service_get_by_type(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final ServiceType type) throws IOException, InterruptedException {
        ServiceSet response = (ServiceSet) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_service_get_by_type(server, authHeader, userCreds, type);
            }
        });
        return response;
    }

    public serviceRegisterResponse xtreemfs_service_register(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final Service service) throws IOException, InterruptedException {
        serviceRegisterResponse response = (serviceRegisterResponse) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_service_register(server, authHeader, userCreds, service);
            }
        });
        return response;
    }

    public Configuration xtreemfs_configuration_get(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final String uuid) throws IOException, InterruptedException {
        Configuration response = (Configuration) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_configuration_get(server, authHeader, userCreds, uuid);
            }
        });
        return response;
    }

    public configurationSetResponse xtreemfs_configuration_set(InetSocketAddress server, final Auth authHeader, final UserCredentials userCreds, final Configuration config) throws IOException, InterruptedException {
        configurationSetResponse response = (configurationSetResponse) syncCall(new CallGenerator() {
            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_configuration_set(server, authHeader, userCreds, config);
            }
        });
        return response;
    }

    @Override
    public long xtreemfs_global_time_get(InetSocketAddress server) {
        try {
            return xtreemfs_global_time_get();
        } catch (Exception ex) {
            return 0;
        }
    }

    public long xtreemfs_global_time_get() throws InterruptedException, PBRPCException, IOException {
        globalTimeSGetResponse response = (globalTimeSGetResponse) syncCall(new CallGenerator() {

            @Override
            public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException {
                return client.xtreemfs_global_time_s_get(server, auth, user);
            }
        });
        return response.getTimeInSeconds();
    }

    public boolean clientIsAlive() {
        return rpcClient.clientIsAlive();
    }

    /**
     * Interface for syncCall which generates the calls. Will be called for each retry.
     */
    protected interface CallGenerator {
        public RPCResponse executeCall(DIRServiceClient client, InetSocketAddress server) throws IOException;
    }
    
    protected Object syncCall(CallGenerator call) throws InterruptedException, PBRPCException, IOException {
        InetSocketAddress server = null;
        isRedirect = false;
        int numTries = 0;
        Exception lastException = null;
        do {
            synchronized (this) {
                server = servers[currentServer];
            }
            RPCResponse response = null;
            try {
                response = call.executeCall(rpcClient, server);
                Object result = response.get();
                return result;
            } catch (PBRPCException ex) {
                switch (ex.getErrorType()) {
                    case REDIRECT: {
                        lastException = ex;
                        if (numTries < maxRetries) {
                            redirect(ex);
                        }
                        break;
                    }
                    case ERRNO: throw ex;
                    default: {
                        lastException = ex;
                        if (numTries < maxRetries) {
                            failover(ex);
                        }
                        break;
                    }
                }
            } catch (IOException ex) {
                Logging.logMessage(Logging.LEVEL_INFO, Category.net, this, "Request failed due to exception: %s", ex);
                lastException = ex;
                failover(ex);
            } finally {
                if (response != null) {
                    response.freeBuffers();
                }
            }

            numTries++;
        } while (numTries <= maxRetries);
        throw new IOException("Request finally failed after "+ (numTries-1) + " tries.", lastException);
    }


    protected void redirect(PBRPCException exception) throws InterruptedException {
        assert(exception.getErrorType() == ErrorType.REDIRECT);
        // We "abuse" the UUID field for the address (hostname:port) as the
        // DIR service has no UUIDs.
        final String address = exception.getRedirectToServerUUID();
        try {
            final int colon = address.indexOf(':');
            final String hostname = address.substring(0, colon);
            final String portStr = address.substring(colon + 1);
            final int port = Integer.valueOf(portStr);
            final InetSocketAddress redirectTo = new InetSocketAddress(hostname, port);

            // Find the entry in the list.
            for (int i = 0; i < servers.length; i++) {
                final InetSocketAddress server = servers[i];
                if (server.equals(redirectTo)) {
                    Logging.logMessage(Logging.LEVEL_INFO, Logging.Category.net, this, "redirected to DIR: %s", server);
                    synchronized (this) {
                        currentServer = i;
                    }
                    if (isRedirect) {
                        // Wait between redirects, but not for the first redirect.
                        Thread.sleep(retryWaitMs);
                    }
                    isRedirect = true;
                    return;
                }
            }
            throw new IOException("Cannot redirect to unknown server: " + redirectTo);
        } catch (InterruptedException ex) {
            // Pass on interruped exception, ignore everything else.
            throw ex;
        } catch (Exception ex) {
            Logging.logError(Logging.LEVEL_ERROR, this, ex);
        }
    }

    protected void failover(IOException exception) throws InterruptedException {
        Logging.logMessage(Logging.LEVEL_ERROR, Category.net, this, "Request to server %s failed due to exception: %s", servers[currentServer], exception);
        Thread.sleep(retryWaitMs);
        synchronized (this) {
            currentServer++;
            if (currentServer >= servers.length) {
                currentServer = 0;
            }
        }
        Logging.logMessage(Logging.LEVEL_INFO, Category.net, this, "Switching to server %s", servers[currentServer]);
    }


}
