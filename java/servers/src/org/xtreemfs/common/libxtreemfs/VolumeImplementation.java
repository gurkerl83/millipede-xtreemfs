/*
 * Copyright (c) 2011 by Paul Seiferth, Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */
package org.xtreemfs.common.libxtreemfs;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

import org.xtreemfs.common.libxtreemfs.RPCCaller.CallGenerator;
import org.xtreemfs.common.libxtreemfs.exceptions.PosixErrorException;

import org.xtreemfs.foundation.SSLOptions;
import org.xtreemfs.foundation.logging.Logging;
import org.xtreemfs.foundation.logging.Logging.Category;
import org.xtreemfs.foundation.pbrpc.client.RPCAuthentication;
import org.xtreemfs.foundation.pbrpc.client.RPCNIOSocketClient;
import org.xtreemfs.foundation.pbrpc.client.RPCResponse;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.Auth;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.POSIXErrno;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.UserCredentials;
import org.xtreemfs.pbrpc.generatedinterfaces.Common.emptyResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.FileCredentials;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.Replica;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.Replicas;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.SERVICES;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.SYSTEM_V_FCNTL;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.StripingPolicyType;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.XLocSet;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.DirectoryEntries;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.Setattrs;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.Stat;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.StatVFS;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.XAttr;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.accessRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.getattrRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.getattrResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.getxattrRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.getxattrResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.linkRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.listxattrRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.listxattrResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.mkdirRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.openRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.openResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.readdirRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.readlinkRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.readlinkResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.removexattrRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.renameRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.renameResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.rmdirRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.setattrRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.setxattrRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.statvfsRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.symlinkRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.timestampResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.unlinkRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.unlinkResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.xtreemfs_get_suitable_osdsRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.xtreemfs_get_suitable_osdsResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.xtreemfs_replica_addRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.xtreemfs_replica_listRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRC.xtreemfs_replica_removeRequest;
import org.xtreemfs.pbrpc.generatedinterfaces.MRCServiceClient;
import org.xtreemfs.pbrpc.generatedinterfaces.OSD.unlink_osd_Request;
import org.xtreemfs.pbrpc.generatedinterfaces.OSDServiceClient;

/**
 * 
 * <br>
 * Sep 5, 2011
 */
public class VolumeImplementation extends Volume {

    /**
     * UUID String of the client.
     */
    private String                            clientUuid;

    /**
     * Client who opened this volume.
     */
    private ClientImplementation              client;

    /**
     * UUIDResolver used by this volume.
     */
    private UUIDResolver                      uuidResolver;

    /**
     * Name of this volume.
     */
    private String                            volumeName;

    /**
     * The options of the client that should be used for this volume.
     */
    private Options                           volumeOptions;

    /**
     * Bogus object of UserCredentials. Used when no real UserCredentials are needed.
     */
    private UserCredentials                   userCredentialsBogus;

    /**
     * The RPC Client.
     * */
    private RPCNIOSocketClient                networkClient;

    /**
     * An MRCServiceClient is a wrapper for an RPC Client.
     * */
    private MRCServiceClient                  mrcServiceClient;

    /**
     * A OSDServiceClient is a wrapper for an RPC Client.
     */
    private OSDServiceClient                  osdServiceClient;

    /**
     * SSLOptions required for connections to the services.
     */
    private SSLOptions                        sslOptions;

    /**
     * Bogus auth used for calls where no valid Auth is required.
     */
    private Auth                              authBogus;

    /**
     * UUIDIterator for all MRCs which know this volume.
     */
    private UUIDIterator                      mrcUUIDIterator;

    /**
     * Concurrent map that stores all open files
     */
    private ConcurrentHashMap<Long, FileInfo> openFileTable;

    /**
     * MetadataCache to cache already fetches Metadata.
     */
    private MetadataCache                     metadataCache;

    /**
     * XCap renewal thread to renew Xcap periodically.
     */
    private PeriodicXcapRenewalThread         xcapRenewalThread;

    /**
     * FileSize update thread to update file size periodically.
     */
    private PeriodicFileSizeUpdateThread      fileSizeUpdateThread;
    
    /**
     * Maps a StripingPolicyType to a StripeTranslator. Should be filled with all possible 
     * StripingPolicys.
     */
    private Map<StripingPolicyType, StripeTranslator> stripeTranslators;

    /**
     * 
     */
    public VolumeImplementation(Client client, String clientUUID, UUIDIterator mrcUuidIterator,
            String volumeName, SSLOptions sslOptions, Options options) {

        this.clientUuid = clientUUID;
        this.uuidResolver = client;
        this.volumeName = volumeName;
        this.volumeOptions = options;
        this.sslOptions = sslOptions;

        this.mrcUUIDIterator = mrcUuidIterator;

        this.userCredentialsBogus = UserCredentials.newBuilder().setUsername("xtreemfs").build();
        this.authBogus = RPCAuthentication.authNone;

        this.metadataCache = new MetadataCache(options.getMetadataCacheSize(), options.getMetadataCacheTTLs());
        // register all stripe translators
        this.stripeTranslators = new HashMap<StripingPolicyType, StripeTranslator>();
        stripeTranslators.put(StripingPolicyType.STRIPING_POLICY_RAID0, new StripeTranslatorRaid0());
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.xtreemfs.common.libxtreemfs.Volume#start()
     */
    @Override
    public void start() throws Exception {
        networkClient = new RPCNIOSocketClient(sslOptions, volumeOptions.getRequestTimeout_s(),
                volumeOptions.getConnectTimeout_s());
        networkClient.start();
        networkClient.waitForStartup();

        mrcServiceClient = new MRCServiceClient(networkClient, null);
        osdServiceClient = new OSDServiceClient(networkClient, null);

        openFileTable = new ConcurrentHashMap<Long, FileInfo>();

        // // Start periodic threads.
        fileSizeUpdateThread = new PeriodicFileSizeUpdateThread(this);
        fileSizeUpdateThread.start();

        xcapRenewalThread = new PeriodicXcapRenewalThread(this);
        xcapRenewalThread.start();

    }

    /*
     * (non-Javadoc)
     * 
     * @see org.xtreemfs.common.libxtreemfs.Volume#internalShutdown()
     */
    @Override
    public void internalShutdown() {
        // Stop periodic threads
        try {
            fileSizeUpdateThread.join();
            xcapRenewalThread.join();
        } catch (InterruptedException e) {
        	e.printStackTrace();
        }

        // There must no FileInfo left in "openFileTable".
        assert (openFileTable.size() == 0);

        // Shutdown network client.
        networkClient.shutdown();
        try {
            networkClient.waitForShutdown();
        } catch (Exception e) {
            if (Logging.isDebug()) {
                Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "Volume: Couldn't shut"
                        + " down network client corretly: %s ", e.getMessage());
            }
        }

    }

    /*
     * (non-Javadoc)
     * 
     * @see org.xtreemfs.common.libxtreemfs.Volume#close()
     */
    @Override
    public void close() {
        internalShutdown();

        client.closeVolume(this);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#statFS(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials)
     */
    @Override
    public StatVFS statFS(UserCredentials userCredentials) throws IOException {
        statvfsRequest request = statvfsRequest.newBuilder().setKnownEtag(0).setVolumeName(volumeName)
                .build();

        StatVFS stat = RPCCaller.<statvfsRequest, StatVFS> syncCall(SERVICES.MRC, userCredentials, authBogus,
                volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<statvfsRequest, StatVFS>() {
                    @Override
                    public RPCResponse<StatVFS> executeCall(InetSocketAddress server, Auth authHeader,
                            UserCredentials userCreds, statvfsRequest input) throws IOException {
                        return mrcServiceClient.statvfs(server, authHeader, userCreds, input);
                    }
                });
        return stat;
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#readLink(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, java.lang.String)
     */
    @Override
    public String readLink(UserCredentials userCredentials, String path) throws IOException {
        readlinkRequest request = readlinkRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .build();

        readlinkResponse response = RPCCaller.<readlinkRequest, readlinkResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<readlinkRequest, readlinkResponse>() {
                    @Override
                    public RPCResponse<readlinkResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, readlinkRequest input)
                            throws IOException {
                        return mrcServiceClient.readlink(server, authHeader, userCreds, input);
                    }
                });
        // The XtreemFS MRC always returns one resolved target or throws an EINVAL.
        assert (response != null && response.getLinkTargetPathCount() == 1);

        return response.getLinkTargetPath(0);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#symlink(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, java.lang.String)
     */
    @Override
    public void symlink(UserCredentials userCredentials, String targetPath, String linkPath)
            throws IOException {
        symlinkRequest request = symlinkRequest.newBuilder().setLinkPath(linkPath).setTargetPath(targetPath)
                .setVolumeName(volumeName).build();

        timestampResponse response = RPCCaller.<symlinkRequest, timestampResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<symlinkRequest, timestampResponse>() {
                    @Override
                    public RPCResponse<timestampResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, symlinkRequest input)
                            throws IOException {
                        return mrcServiceClient.symlink(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        String parentDir = Helper.resolveParentDirectory(linkPath);
        metadataCache.updateStatTime(parentDir, response.getTimestampS(), Setattrs.SETATTR_CTIME.getNumber()
                | Setattrs.SETATTR_MTIME.getNumber());
        // TODO: Retrieve stat as optional member of the response instead
        // and update cached DirectoryEntries accordingly.
        metadataCache.invalidateDirEntries(parentDir);
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.xtreemfs.common.libxtreemfs.Volume#link(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.
     * UserCredentials, java.lang.String, java.lang.String)
     */
    @Override
    public void link(UserCredentials userCredentials, String targetPath, String linkPath) throws IOException {
        linkRequest request = linkRequest.newBuilder().setLinkPath(linkPath).setTargetPath(targetPath)
                .setVolumeName(volumeName).build();

        timestampResponse response = RPCCaller.<linkRequest, timestampResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<linkRequest, timestampResponse>() {
                    @Override
                    public RPCResponse<timestampResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, linkRequest input) throws IOException {
                        return mrcServiceClient.link(server, authHeader, userCreds, input);
                    }
                });
        ;

        assert (response != null);

        String parentDir = Helper.resolveParentDirectory(linkPath);
        metadataCache.updateStatTime(parentDir, response.getTimestampS(), Setattrs.SETATTR_CTIME.getNumber()
                | Setattrs.SETATTR_MTIME.getNumber());

        // TODO: Retrieve stat as optional member of the response instead
        // and update cached DirectoryEntries accordingly.
        metadataCache.invalidateDirEntries(parentDir);

        // Invalidate caches as we don't cache links and their targets.
        metadataCache.invalidate(linkPath);
        metadataCache.invalidate(targetPath);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#access(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, org.xtreemfs.pbrpc.generatedinterfaces.MRC.ACCESS_FLAGS)
     */
    @Override
    public void access(UserCredentials userCredentials, String path, int flags) throws IOException,
            PosixErrorException {
        accessRequest request = accessRequest.newBuilder().setFlags(flags).setPath(path)
                .setVolumeName(volumeName).build();

        RPCCaller.<accessRequest, emptyResponse> syncCall(SERVICES.MRC, userCredentials, authBogus,
                volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<accessRequest, emptyResponse>() {
                    @SuppressWarnings("unchecked")
                    @Override
                    public RPCResponse<emptyResponse> executeCall(InetSocketAddress server, Auth authHeader,
                            UserCredentials userCreds, accessRequest input) throws IOException {
                        return mrcServiceClient.access(server, authHeader, userCreds, input);
                    }
                });
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#openFile(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.SYSTEM_V_FCNTL)
     */
    @Override
    public FileHandle openFile(UserCredentials userCredentials, String path, int flags) throws IOException,
            PosixErrorException {
        return openFile(userCredentials, path, flags, 0, 0);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#openFile(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.SYSTEM_V_FCNTL,
     * int)
     */
    @Override
    public FileHandle openFile(UserCredentials userCredentials, String path, int flags, int mode)
            throws IOException, PosixErrorException {
        return openFile(userCredentials, path, flags, mode, 0);
    }

    /**
     * Used by Volume.truncate() method. Otherwise truncateNewFileSize = 0;
     * 
     * @param userCredentials
     * @param path
     * @param flags
     * @param mode
     * @param truncateNewFileSize
     * @return
     */
    public FileHandle openFile(UserCredentials userCredentials, String path, int flags, int mode,
            int truncateNewFileSize) throws IOException, PosixErrorException {

        boolean asyncWritesEnabled = (volumeOptions.getMaxWriteahead() > 0);

        if ((SYSTEM_V_FCNTL.SYSTEM_V_FCNTL_H_O_SYNC.getNumber() & flags) > 0) {
            if (Logging.isDebug()) {
                Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "open called with"
                        + " O_SYNC, async writes were disabled");
            }
            asyncWritesEnabled = false;
        }

        openRequest request = openRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .setFlags(flags).setMode(mode).setAttributes(0).build();

        openResponse response = RPCCaller.<openRequest, openResponse> syncCall(SERVICES.MRC, userCredentials,
                authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<openRequest, openResponse>() {
                    @Override
                    public RPCResponse<openResponse> executeCall(InetSocketAddress server, Auth authHeader,
                            UserCredentials userCreds, openRequest input) throws IOException {
                        return mrcServiceClient.open(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        // We must have obtained file credentials.
        assert (response.hasCreds());

        if (response.getCreds().getXlocs().getReplicasCount() == 0) {
            String errorMessage = "MRC assigned no OSDs to file on open" + path + ", xloc: "
                    + response.getCreds().getXlocs().toString();
            Logging.logMessage(Logging.LEVEL_ERROR, Category.misc, this, errorMessage);
            throw new PosixErrorException(POSIXErrno.POSIX_ERROR_EIO, errorMessage);
        }

        FileHandle fileHandle = null;

        // Create a FileInfo object if it does not exist yet.
        FileInfo fileInfo = getOrCreateFileInfo(Helper.extractFileIdFromXcap(response.getCreds().getXcap()),
                path, response.getCreds().getXcap().getReplicateOnClose(), response.getCreds().getXlocs());

        fileHandle = fileInfo.createFileHandle(response.getCreds().getXcap(), asyncWritesEnabled);

        // If O_CREAT is set and the file did not previously exist, upon successful
        // completion, open() shall mark for update the st_atime, st_ctime, and
        // st_mtime fields of the file and the st_ctime and st_mtime fields of
        // the parent directory.
        if ((flags & SYSTEM_V_FCNTL.SYSTEM_V_FCNTL_H_O_CREAT.getNumber()) > 0) {
            String parentDir = Helper.resolveParentDirectory(path);
            metadataCache.updateStatTime(path, response.getTimestampS(), Setattrs.SETATTR_CTIME.getNumber()
                    | Setattrs.SETATTR_MTIME.getNumber());
            // TODO: Retrieve stat as optional member of the response instead
            // and update cached DirectoryEntries accordingly.
            metadataCache.invalidate(parentDir);
        }

        // If O_TRUNC was set, go on processing the truncate request.
        if ((flags & SYSTEM_V_FCNTL.SYSTEM_V_FCNTL_H_O_TRUNC.getNumber()) > 0) {

            // Update mtime and ctime of the file if O_TRUNC was set.
            metadataCache.updateStatTime(path, response.getTimestampS(), Setattrs.SETATTR_CTIME.getNumber()
                    | Setattrs.SETATTR_MTIME.getNumber());

            if (Logging.isDebug()) {
                Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "open called with O_TRUNK.");
            }

            // TODO: Do this stuff
            //
            // try {
            // file_handle->TruncatePhaseTwoAndThree(user_credentials,
            // truncate_new_file_size);
            // } catch(const XtreemFSException& e) {
            // // Truncate did fail, close file again.
            // file_handle->Close();
            // throw; // Rethrow error.
            // }
        }

        return fileHandle;
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#truncate(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, int)
     */
    @Override
    public void truncate(UserCredentials userCredentials, String path, int newFileSize) throws IOException,
            PosixErrorException {
        // Open file with O_TRUNC
        int flags = SYSTEM_V_FCNTL.SYSTEM_V_FCNTL_H_O_WRONLY.getNumber()
                | SYSTEM_V_FCNTL.SYSTEM_V_FCNTL_H_O_TRUNC.getNumber();
        FileHandle fileHandle = openFile(userCredentials, path, flags, newFileSize);

        // close file
        fileHandle.close();
        // TODO: write Unittest
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#getAttr(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, org.xtreemfs.pbrpc.generatedinterfaces.MRC.Stat)
     */
    @Override
    public Stat getAttr(UserCredentials userCredentials, String path) throws IOException {

        // Retrive stat object from cache or MRC
        Stat stat = getAttrHelper(userCredentials, path);

        // TODO: Do the rest of michaels implementation. Waiting for async writes to finished and
        // updating the metadatacache appropriately

        return stat;
    }

    private Stat getAttrHelper(UserCredentials userCredentials, String path) throws IOException {
        // Check if Stat object is cached.
        Stat stat = metadataCache.getStat(path);

        if (stat != null) {
            // Found in StatCache
            if (Logging.isDebug()) {
                Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this,
                        "getattr: serving from stat-cache %s  %s", path, stat.getSize());
            }
        } else {

            // if not, retrive stat from MRC
            getattrRequest request = getattrRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                    .setKnownEtag(0).build();

            getattrResponse response = RPCCaller.<getattrRequest, getattrResponse> syncCall(SERVICES.MRC,
                    userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                    new CallGenerator<getattrRequest, getattrResponse>() {
                        @Override
                        public RPCResponse<getattrResponse> executeCall(InetSocketAddress server,
                                Auth authHeader, UserCredentials userCreds, getattrRequest input)
                                throws IOException {
                            return mrcServiceClient.getattr(server, authHeader, userCreds, input);
                        }
                    });

            assert (response != null);

            stat = response.getStbuf();
            if (stat.getNlink() > 1) { // Do not cache hardlinks
                metadataCache.invalidate(path);
            } else {
                metadataCache.updateStat(path, stat);
            }
        }
        return stat;
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#setAttr(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, org.xtreemfs.pbrpc.generatedinterfaces.MRC.Stat,
     * org.xtreemfs.pbrpc.generatedinterfaces.MRC.Setattrs)
     */
    @Override
    public void setAttr(UserCredentials userCredentials, String path, Stat stat, int toSet)
            throws IOException {
        setattrRequest request = setattrRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .setStbuf(stat).setToSet(toSet).build();

        timestampResponse response = RPCCaller.<setattrRequest, timestampResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<setattrRequest, timestampResponse>() {
                    @Override
                    public RPCResponse<timestampResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, setattrRequest input)
                            throws IOException {
                        return mrcServiceClient.setattr(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        // "chmod" or "chown" operations result into updating the ctime attribute.
        if (((toSet & Setattrs.SETATTR_MODE.getNumber()) > 0)
                || ((toSet & Setattrs.SETATTR_UID.getNumber()) > 0)
                || ((toSet & Setattrs.SETATTR_GID.getNumber()) > 0)) {

            toSet = (toSet | Setattrs.SETATTR_CTIME.getNumber());
            stat = stat.toBuilder().setAtimeNs(1000000000L * response.getTimestampS()).build();
        }

        // Do not cache hardlinks or chmod operations which try to set the SGID bit
        // as it might get cleared by the MRC.
        if (stat.getNlink() > 1
                || (((toSet & Setattrs.SETATTR_MODE.getNumber()) > 0) && ((stat.getMode() & (1 << 10))) > 0)) {
            metadataCache.invalidate(path);
        } else {
            metadataCache.updateStatAttributes(path, stat, toSet);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#nnlink(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String)
     */
    @Override
    public void unlink(UserCredentials userCredentials, String path) throws IOException {
        // 1. Delete file at MRC.
        unlinkRequest request = unlinkRequest.newBuilder().setPath(path).setVolumeName(volumeName).build();

        unlinkResponse response = RPCCaller.<unlinkRequest, unlinkResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<unlinkRequest, unlinkResponse>() {
                    @Override
                    public RPCResponse<unlinkResponse> executeCall(InetSocketAddress server, Auth authHeader,
                            UserCredentials userCreds, unlinkRequest input) throws IOException {
                        return mrcServiceClient.unlink(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        // 2. Invalidate metadata cache
        metadataCache.invalidate(path);
        String parentDir = Helper.resolveParentDirectory(path);
        metadataCache.updateStatTime(path, response.getTimestampS(), Setattrs.SETATTR_CTIME.getNumber()
                | Setattrs.SETATTR_MTIME.getNumber());
        metadataCache.invalidateDirEntry(parentDir, Helper.getBasename(path));

        // 3. Delete objects of all replicas on the OSDs.
        if (response.hasCreds()) {
            unlinkAtOsd(response.getCreds(), path);
        }
    }

    private void unlinkAtOsd(FileCredentials fc, String path) throws IOException {
        unlink_osd_Request request = unlink_osd_Request.newBuilder().setFileCredentials(fc)
                .setFileId(fc.getXcap().getFileId()).build();

        UUIDIterator osdUuidIterator = new UUIDIterator();

        // Remove _all_ replicas.
        for (int i = 0; i < fc.getXlocs().getReplicasCount(); i++) {
            osdUuidIterator.clearAndAddUUID(Helper.getOSDUUIDFromXlocSet(fc.getXlocs(), i, 0));

            RPCCaller.<unlink_osd_Request, emptyResponse> syncCall(SERVICES.OSD, userCredentialsBogus,
                    authBogus, volumeOptions, uuidResolver, osdUuidIterator, false, request,
                    new CallGenerator<unlink_osd_Request, emptyResponse>() {
                        @SuppressWarnings("unchecked")
                        @Override
                        public RPCResponse<emptyResponse> executeCall(InetSocketAddress server,
                                Auth authHeader, UserCredentials userCreds, unlink_osd_Request input)
                                throws IOException {
                            return osdServiceClient.unlink(server, authHeader, userCreds, input);
                        }
                    });
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#rename(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, java.lang.String)
     */
    @Override
    public void rename(UserCredentials userCredentials, String path, String newPath) throws IOException {
        if (path.equals(newPath)) {
            return; // Do nothing.
        }

        // 1. Issue rename at MRC.
        renameRequest request = renameRequest.newBuilder().setVolumeName(volumeName).setSourcePath(path)
                .setTargetPath(newPath).build();

        renameResponse response = RPCCaller.<renameRequest, renameResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<renameRequest, renameResponse>() {
                    @Override
                    public RPCResponse<renameResponse> executeCall(InetSocketAddress server, Auth authHeader,
                            UserCredentials userCreds, renameRequest input) throws IOException {
                        return mrcServiceClient.rename(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        // 2. Remove file content of any previous files at "newPath".
        if (response.hasCreds()) {
            unlinkAtOsd(response.getCreds(), newPath);
        }

        // 3. Update caches // pseiferth: Implement metadatacache first.
        // Update the timestamps of parents of both directories.
        String parentPath = Helper.resolveParentDirectory(path);
        String parentNewPath = Helper.resolveParentDirectory(parentPath);
        if (response.getTimestampS() != 0) {
            metadataCache.updateStatTime(parentPath, response.getTimestampS(),
                    Setattrs.SETATTR_CTIME.getNumber() | Setattrs.SETATTR_MTIME.getNumber());
            metadataCache.updateStatTime(parentNewPath, response.getTimestampS(),
                    Setattrs.SETATTR_CTIME.getNumber() | Setattrs.SETATTR_MTIME.getNumber());
        }

        metadataCache.invalidateDirEntry(parentPath, Helper.getBasename(path));
        // TODO(mberlin): Add DirEntry instead to parent_new_path if stat available.
        metadataCache.invalidateDirEntries(parentNewPath);
        // Overwrite an existing entry; no Prefix() operation needed because:
        // "If new names an existing directory, it shall be required to be an empty
        // directory."
        // see http://pubs.opengroup.org/onlinepubs/009695399/functions/rename.html
        metadataCache.invalidate(newPath);
        // Rename all affected entries.
        metadataCache.renamePrefix(path, newPath);
        // http://pubs.opengroup.org/onlinepubs/009695399/functions/rename.html:
        // "Some implementations mark for update the st_ctime field of renamed files
        // and some do not."
        // => XtreemFS does so, i.e. update the client's cache, too.
        metadataCache.updateStatTime(newPath, response.getTimestampS(), Setattrs.SETATTR_CTIME.getNumber());

        // 4. Rename path in all open FileInfo objects.
        for (Entry<Long, FileInfo> entry : openFileTable.entrySet()) {
            entry.getValue().renamePath(path, newPath);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#createDirectory(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String, int)
     */
    @Override
    public void createDirectory(UserCredentials userCredentials, String path, int mode) throws IOException {
        mkdirRequest request = mkdirRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .setMode(mode).build();

        timestampResponse response = RPCCaller.<mkdirRequest, timestampResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<mkdirRequest, timestampResponse>() {
                    @Override
                    public RPCResponse<timestampResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, mkdirRequest input)
                            throws IOException {
                        return mrcServiceClient.mkdir(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        String parentDir = Helper.resolveParentDirectory(path);
        metadataCache.updateStatTime(path, response.getTimestampS(), Setattrs.SETATTR_CTIME.getNumber()
                | Setattrs.SETATTR_MTIME.getNumber());
        // TODO: Retrieve stat as optional member of openResponse instead
        // and update cached DirectoryEntries accordingly.
        metadataCache.invalidateDirEntries(parentDir);

    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#removeDirectory(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String)
     */
    @Override
    public void removeDirectory(UserCredentials userCredentials, String path) throws IOException {
        rmdirRequest request = rmdirRequest.newBuilder().setVolumeName(volumeName).setPath(path).build();

        timestampResponse response = RPCCaller.<rmdirRequest, timestampResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<rmdirRequest, timestampResponse>() {
                    @Override
                    public RPCResponse<timestampResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, rmdirRequest input)
                            throws IOException {
                        return mrcServiceClient.rmdir(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        // Invalidate Metadatacache.
        String parentDir = Helper.resolveParentDirectory(path);
        metadataCache.updateStatTime(parentDir, response.getTimestampS(), Setattrs.SETATTR_CTIME.getNumber()
                | Setattrs.SETATTR_MTIME.getNumber());
        metadataCache.invalidatePrefix(path);
        metadataCache.invalidateDirEntry(path, Helper.getBasename(path));

    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#readDir(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, long, long, boolean)
     */
    @Override
    public DirectoryEntries readDir(UserCredentials userCredentials, String path, int offset, int count,
            boolean namesOnly) throws IOException {
        DirectoryEntries result = null;

        // Try to get DirectoryEntries from cache
        result = metadataCache.getDirEntries(path, offset, count);
        if (result != null) {
            return result;
        }

        DirectoryEntries.Builder dirEntriesBuilder = DirectoryEntries.newBuilder();

        // Process large requests in multiples of readdirChunkSize.
        for (int currentOffset = offset; currentOffset < offset + count; offset += volumeOptions
                .getReaddirChunkSize()) {

            int limitDirEntriesCount = (currentOffset > offset + count) ? (currentOffset - offset - count)
                    : volumeOptions.getReaddirChunkSize();

            readdirRequest request = readdirRequest.newBuilder().setPath(path).setVolumeName(volumeName)
                    .setNamesOnly(namesOnly).setKnownEtag(0).setSeenDirectoryEntriesCount(currentOffset)
                    .setLimitDirectoryEntriesCount(limitDirEntriesCount).build();

            result = RPCCaller.<readdirRequest, DirectoryEntries> syncCall(SERVICES.MRC, userCredentials,
                    authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                    new CallGenerator<readdirRequest, DirectoryEntries>() {
                        @Override
                        public RPCResponse<DirectoryEntries> executeCall(InetSocketAddress server,
                                Auth authHeader, UserCredentials userCreds, readdirRequest input)
                                throws IOException {
                            return mrcServiceClient.readdir(server, authHeader, userCreds, input);
                        }
                    });

            assert (result != null);

            dirEntriesBuilder.addAllEntries(result.getEntriesList());

            // Break if this is the last chunk.
            if (result.getEntriesCount() < (currentOffset + volumeOptions.getReaddirChunkSize())) {
                break;
            }
        }

        // TODO: Merge possible pending file size updates of files into
        // the stat entries of listed files.

        // Cache the first stat buffers that fit into the cache.
        int minimum = (int) ((volumeOptions.getMetadataCacheSize() > result.getEntriesCount()) ? volumeOptions
                .getMetadataCacheSize() : result.getEntriesCount());
        for (int i = 0; i < minimum; i++) {
            if (result.getEntries(i).getStbuf().getNlink() > 1) { // Do not cache hard links.
                metadataCache.invalidate(path);
            } else {
                metadataCache.updateStat(Helper.concatenatePath(path, result.getEntries(i).getName()), result
                        .getEntries(i).getStbuf());
            }
        }

        // Cache the result if it's the complete directory.
        // We can't tell for sure whether result contains all directory entries if
        // it's size is not less than the requested "count".
        // TODO: Cache only names and no stat entries and remove names_only
        // condition.
        // TODO: Set an upper bound of dentries, otherwise don't cache it.

        if (!namesOnly && offset == 0 && result.getEntriesCount() < count) {
            metadataCache.updateDirEntries(path, result);
        }

        return dirEntriesBuilder.build();
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#listXAttrs(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String)
     */
    @Override
    public listxattrResponse listXAttrs(UserCredentials userCredentials, String path) throws IOException {
        return listXAttrs(userCredentials, path, true);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#listXAttrs(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String, boolean)
     */
    @Override
    public listxattrResponse listXAttrs(UserCredentials userCredentials, String path, boolean useCache)
            throws IOException {
        // Check if information was cached.
        if (useCache) {
            // TODO: retrieve information from metadatacache if available.
        }

        listxattrRequest request = listxattrRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .setNamesOnly(false).build();

        listxattrResponse response = RPCCaller.<listxattrRequest, listxattrResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<listxattrRequest, listxattrResponse>() {
                    @Override
                    public RPCResponse<listxattrResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, listxattrRequest input)
                            throws IOException {
                        return mrcServiceClient.listxattr(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        metadataCache.updateXAttrs(path, response);

        return response;
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#setXAttr(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, java.lang.String, java.lang.String,
     * org.xtreemfs.pbrpc.generatedinterfaces.MRC.XATTR_FLAGS)
     */
    @Override
    public void setXAttr(UserCredentials userCredentials, String path, String name, String value, int flags)
            throws IOException {
        setxattrRequest request = setxattrRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .setName(name).setValue(value).setFlags(flags).build();

        timestampResponse response = RPCCaller.<setxattrRequest, timestampResponse> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<setxattrRequest, timestampResponse>() {
                    @Override
                    public RPCResponse<timestampResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, setxattrRequest input)
                            throws IOException {
                        return mrcServiceClient.setxattr(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        metadataCache.updateXAttr(path, name, value);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#getXAttr(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, java.lang.String, java.lang.String)
     */
    @Override
    public String getXAttr(UserCredentials userCredentials, String path, String name) throws IOException {
        boolean xtreemfsAttrRequest = name.substring(0, 9).equals("xtreemfs.");

        if (xtreemfsAttrRequest) {
            // Retrive only the value of the requested attribute, not the whole list.
            getxattrRequest request = getxattrRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                    .setName(name).build();

            getxattrResponse response = RPCCaller.<getxattrRequest, getxattrResponse> syncCall(SERVICES.MRC,
                    userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                    new CallGenerator<getxattrRequest, getxattrResponse>() {
                        @Override
                        public RPCResponse<getxattrResponse> executeCall(InetSocketAddress server,
                                Auth authHeader, UserCredentials userCreds, getxattrRequest input)
                                throws IOException {
                            return mrcServiceClient.getxattr(server, authHeader, userCreds, input);
                        }
                    });

            assert (response != null);

            if (response.hasValue()) {
                return response.getValue();
            } else {
                return null;
            }

        } else {
            // No "xtreemfs." attribute, lookup metadata cache.
            Tupel<String, Boolean> cachedXattr = metadataCache.getXAttr(path, name);
            if (cachedXattr.getFirst() == null) {
                // Xattr not found in cache
                if (cachedXattr.getSecond()) {
                    // All attributes were cached but the requested attribute was not found,
                    // i.e. it won't exist on the server.
                    return null;
                }
            }

            // If not found in metadatacache retrieve the whole list when attrs weren't cached.
            listxattrResponse xattrList = listXAttrs(userCredentials, path);
            if (xattrList.getXattrsCount() > 0) {
                for (XAttr xattr : xattrList.getXattrsList()) {
                    if (xattr.getName().equals(name)) {
                        return xattr.getValue();
                    }
                }
            }
        }
        return null;
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#getXAttrSize(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String, java.lang.String, int)
     */
    @Override
    public int getXAttrSize(UserCredentials userCredentials, String path, String name) throws IOException {

        // Try to get it from cache first.
        // We also return the size of cached "xtreemfs." attributes. However, the
        // actual size may differ as getXAttr() does never return "xtreemfs."
        // attributes from the cache.
        boolean xtreemfsAttributeRequested = name.subSequence(0, 9).equals("xtreemfs.");

        // Differ between "xtreeemfs." attributes and user attributes.
        if (xtreemfsAttributeRequested) {
            // Always retrive from the server
            String attr = getXAttr(userCredentials, path, name);
            // Return size of value if it was found. -1 otherwise.
            if (attr != null) {
                return attr.length();
            } else {
                return -1;
            }
        } else {
            // user attribute
            Tupel<Integer, Boolean> cachedXattrSize = metadataCache.getXAttrSize(path, name);

            // valid user attribute was found in cache
            if (cachedXattrSize.getFirst() != 0) {
                return cachedXattrSize.getFirst();
            }

            // no valid user attribute was found in cache, but Xattrs for this path where cached.
            if (cachedXattrSize.getSecond()) {
                return 0;
            }

            // Retrive complete list of xattrs.
            listxattrResponse xattrList = listXAttrs(userCredentials, path);
            if (xattrList.getXattrsCount() > 0) {
                for (XAttr xattr : xattrList.getXattrsList()) {
                    if (xattr.getName().equals(name)) {
                        return xattr.getValue().length();
                    }
                }
            }
            return -1;
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#removeXAttr(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String, java.lang.String)
     */
    @Override
    public void removeXAttr(UserCredentials userCredentials, String path, String name) throws IOException {
        removexattrRequest request = removexattrRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .setName(name).build();

        RPCCaller.<removexattrRequest, timestampResponse> syncCall(SERVICES.MRC, userCredentials, authBogus,
                volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<removexattrRequest, timestampResponse>() {
                    @Override
                    public RPCResponse<timestampResponse> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, removexattrRequest input)
                            throws IOException {
                        return mrcServiceClient.removexattr(server, authHeader, userCreds, input);
                    }
                });
        metadataCache.invalidateXAttr(path, name);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#addReplica(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String, org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.Replica)
     */
    @Override
    public void addReplica(UserCredentials userCredentials, String path, Replica newReplica)
            throws IOException, PosixErrorException {
        xtreemfs_replica_addRequest request = xtreemfs_replica_addRequest.newBuilder()
                .setVolumeName(volumeName).setPath(path).setNewReplica(newReplica).build();

        RPCCaller.<xtreemfs_replica_addRequest, emptyResponse> syncCall(SERVICES.MRC, userCredentials,
                authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<xtreemfs_replica_addRequest, emptyResponse>() {
                    @SuppressWarnings("unchecked")
                    @Override
                    public RPCResponse<emptyResponse> executeCall(InetSocketAddress server, Auth authHeader,
                            UserCredentials userCreds, xtreemfs_replica_addRequest input) throws IOException {
                        return mrcServiceClient.xtreemfs_replica_add(server, authHeader, userCreds, input);
                    }
                });

        // Trigger the replication at this point by reading at least one byte.
        FileHandle fileHandle = openFile(userCredentials, path,
                SYSTEM_V_FCNTL.SYSTEM_V_FCNTL_H_O_RDONLY.getNumber());
        fileHandle.pingReplica(userCredentials, newReplica.getOsdUuids(0));
        fileHandle.close();
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#listReplicas(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String)
     */
    @Override
    public Replicas listReplicas(UserCredentials userCredentials, String path) throws IOException {
        xtreemfs_replica_listRequest request = xtreemfs_replica_listRequest.newBuilder()
                .setVolumeName(volumeName).setPath(path).build();

        Replicas response = RPCCaller.<xtreemfs_replica_listRequest, Replicas> syncCall(SERVICES.MRC,
                userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator, false, request,
                new CallGenerator<xtreemfs_replica_listRequest, Replicas>() {
                    @Override
                    public RPCResponse<Replicas> executeCall(InetSocketAddress server, Auth authHeader,
                            UserCredentials userCreds, xtreemfs_replica_listRequest input) throws IOException {
                        return mrcServiceClient.xtreemfs_replica_list(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        return response;
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#removeReplica(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String, java.lang.String)
     */
    @Override
    public void removeReplica(UserCredentials userCredentials, String path, String osdUuid)
            throws IOException {
        // remove the replica
        xtreemfs_replica_removeRequest request = xtreemfs_replica_removeRequest.newBuilder()
                .setVolumeName(volumeName).setPath(path).setOsdUuid(osdUuid).build();

        FileCredentials response = RPCCaller.<xtreemfs_replica_removeRequest, FileCredentials> syncCall(
                SERVICES.MRC, userCredentials, authBogus, volumeOptions, uuidResolver, mrcUUIDIterator,
                false, request, new CallGenerator<xtreemfs_replica_removeRequest, FileCredentials>() {
                    @Override
                    public RPCResponse<FileCredentials> executeCall(InetSocketAddress server,
                            Auth authHeader, UserCredentials userCreds, xtreemfs_replica_removeRequest input)
                            throws IOException {
                        return mrcServiceClient.xtreemfs_replica_remove(server, authHeader, userCreds, input);
                    }
                });

        assert (response != null);

        // Now unlink the replica at the OSD.
        UUIDIterator osdUuidIterator = new UUIDIterator();
        osdUuidIterator.addUUID(osdUuid);

        unlink_osd_Request request2 = unlink_osd_Request.newBuilder()
                .setFileId(response.getXcap().getFileId()).setFileCredentials(response).build();

        RPCCaller.<unlink_osd_Request, emptyResponse> syncCall(SERVICES.OSD, userCredentials, authBogus,
                volumeOptions, uuidResolver, mrcUUIDIterator, false, request2,
                new CallGenerator<unlink_osd_Request, emptyResponse>() {
                    @SuppressWarnings("unchecked")
                    @Override
                    public RPCResponse<emptyResponse> executeCall(InetSocketAddress server, Auth authHeader,
                            UserCredentials userCreds, unlink_osd_Request input) throws IOException {
                        return osdServiceClient.unlink(server, authHeader, userCreds, input);
                    }
                });
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#getSuitableOSDs(org.xtreemfs.foundation.pbrpc.generatedinterfaces
     * .RPC.UserCredentials, java.lang.String, int, java.util.List)
     */
    @Override
    public List<String> getSuitableOSDs(UserCredentials userCredentials, String path, int numberOfOsds)
            throws IOException {
        xtreemfs_get_suitable_osdsRequest request = xtreemfs_get_suitable_osdsRequest.newBuilder()
                .setVolumeName(volumeName).setPath(path).setNumOsds(numberOfOsds).build();

        xtreemfs_get_suitable_osdsResponse response = RPCCaller
                .<xtreemfs_get_suitable_osdsRequest, xtreemfs_get_suitable_osdsResponse> syncCall(
                        SERVICES.MRC,
                        userCredentials,
                        authBogus,
                        volumeOptions,
                        uuidResolver,
                        mrcUUIDIterator,
                        false,
                        request,
                        new CallGenerator<xtreemfs_get_suitable_osdsRequest, xtreemfs_get_suitable_osdsResponse>() {
                            @Override
                            public RPCResponse<xtreemfs_get_suitable_osdsResponse> executeCall(
                                    InetSocketAddress server, Auth authHeader, UserCredentials userCreds,
                                    xtreemfs_get_suitable_osdsRequest input) throws IOException {
                                return mrcServiceClient.xtreemfs_get_suitable_osds(server, authHeader,
                                        userCreds, input);
                            }
                        });

        assert (response != null);

        return response.getOsdUuidsList();
    }

    /**
     * Called by FileHandle.Close() to remove file_handle from the list.
     */
    protected void closeFile(long fileId, FileInfo fileInfo, FileHandleImplementation fileHandle) {
        // TODO: Implement this method correctly.
    }

    /**
     * Returns FileInfo object. Looking for it at first in the openFileTable and if it is not found there it
     * creates a new one.
     * 
     * @return FileInfo
     */
    private FileInfo getOrCreateFileInfo(long fileId, String path, boolean replicateOnClose, XLocSet xlocset) {
        FileInfo fileInfo = openFileTable.get(fileId);

        if (fileInfo != null) {
            fileInfo.updateXLocSetAndRest(xlocset, replicateOnClose);
            return fileInfo;
        } else {
            // File has not been opened yet, add it.
            fileInfo = new FileInfo(this, fileId, path, replicateOnClose, xlocset, clientUuid);
            openFileTable.put(fileId, fileInfo);
            if (Logging.isDebug()) {
                Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "Created a new FileInfo"
                        + " object for the fileId: %s", fileId);
            }
            return fileInfo;
        }
    }

    protected UUIDIterator getMrcUuidIterator() {
        return this.mrcUUIDIterator;
    }

    protected UUIDResolver getUUIDResolver() {
        return this.uuidResolver;
    }

    protected MRCServiceClient getMrcServiceClient() {
        return this.mrcServiceClient;
    }

    protected OSDServiceClient getOsdServiceClient() {
        return this.osdServiceClient;
    }

    protected Options getOptions() {
        return this.volumeOptions;
    }

    protected ConcurrentHashMap<Long, FileInfo> getOpenFileTable() {
        return this.openFileTable;
    }

    protected Auth getAuthBogus() {
        return this.authBogus;
    }

    protected UserCredentials getUserCredentialsBogus() {
        return this.userCredentialsBogus;
    }
    
    protected Map<StripingPolicyType, StripeTranslator> getStripeTranslators() {
    	return this.stripeTranslators;
    }

}
