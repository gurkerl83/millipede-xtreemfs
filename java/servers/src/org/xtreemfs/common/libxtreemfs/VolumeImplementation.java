/*
 * Copyright (c) 2011 by Paul Seiferth, Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */
package org.xtreemfs.common.libxtreemfs;

import java.io.IOException;
import java.lang.reflect.Method;
import java.net.InetSocketAddress;
import java.util.List;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

import org.xtreemfs.common.libxtreemfs.exceptions.PosixErrorException;
import org.xtreemfs.foundation.SSLOptions;
import org.xtreemfs.foundation.logging.Logging;
import org.xtreemfs.foundation.logging.Logging.Category;
import org.xtreemfs.foundation.pbrpc.client.RPCAuthentication;
import org.xtreemfs.foundation.pbrpc.client.RPCNIOSocketClient;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.Auth;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.POSIXErrno;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.UserCredentials;
import org.xtreemfs.pbrpc.generatedinterfaces.Common.emptyResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.FileCredentials;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.Replica;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.Replicas;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.SYSTEM_V_FCNTL;
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
     * 
     */
    public VolumeImplementation(Client client, String clientUUID, UUIDIterator mrcUuidIterator,
            String volumeName, SSLOptions sslOptions, Options options) {

        this.clientUuid = clientUUID;
        this.uuidResolver = client;
        this.volumeName = volumeName;
        this.volumeOptions = options;
        this.sslOptions = sslOptions;
        // TODO: Ask if the mrcUUIDIterator needs to be copied since the calling method can later
        // alter the content of this iterator.
        this.mrcUUIDIterator = mrcUuidIterator;

        this.userCredentialsBogus = UserCredentials.newBuilder().setUsername("xtreemfs").build();
        this.authBogus = RPCAuthentication.authNone;
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

        // TODO: Do this stuff?!?!
        // // Start periodic threads.
        // xcap_renewal_thread_.reset(new boost::thread(boost::bind(
        // &xtreemfs::VolumeImplementation::PeriodicXCapRenewal, this)));
        // filesize_writeback_thread_.reset(new boost::thread(boost::bind(
        // &xtreemfs::VolumeImplementation::PeriodicFileSizeUpdate, this)));
    }

    /*
     * (non-Javadoc)
     * 
     * @see org.xtreemfs.common.libxtreemfs.Volume#internalShutdown()
     */
    @Override
    public void internalShutdown() {
        // TODO Auto-generated method stub

    }

    /*
     * (non-Javadoc)
     * 
     * @see org.xtreemfs.common.libxtreemfs.Volume#close()
     */
    @Override
    public void close() {
        // TODO Auto-generated method stub

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

        StatVFS stat = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("statvfs", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, statvfsRequest.class });

            stat = RPCCaller.<MRCServiceClient, statvfsRequest, StatVFS> makeCall(mrcServiceClient, m,
                    userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {

            throw new IOException(e);
        }
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
        readlinkResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("readlink", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, readlinkRequest.class });

            RPCCaller.<MRCServiceClient, readlinkRequest, readlinkResponse> makeCall(mrcServiceClient, m,
                    userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
        }

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
        timestampResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("", new Class<?>[] { InetSocketAddress.class,
                    Auth.class, UserCredentials.class, symlinkRequest.class });

            response = RPCCaller.<MRCServiceClient, symlinkRequest, timestampResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
        }

        assert (response != null);
        // TODO: Update Metadatacache when implemented!

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

        timestampResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("link", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, linkRequest.class });

            response = RPCCaller.<MRCServiceClient, linkRequest, timestampResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
        }

        assert (response != null);

        // TODO: Update MetadataCache when it is implemented.
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

        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("access", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, accessRequest.class });

            RPCCaller.<MRCServiceClient, accessRequest, emptyResponse> makeCall(mrcServiceClient, m,
                    userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

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

        openResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("open", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, openRequest.class });

            response = RPCCaller.<MRCServiceClient, openRequest, openResponse> makeCall(mrcServiceClient, m,
                    userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

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
            // TODO: Do this stuff if metadatacache is implemented.
            // Sttring parentDir = ResolveParentDirectory(path);
            // metadata_cache_.UpdateStatTime(
            // parent_dir,
            // timestamp_s,
            // static_cast<Setattrs>(SETATTR_CTIME | SETATTR_MTIME));
            // // TODO(mberlin): Retrieve stat as optional member of openResponse instead
            // // and update cached DirectoryEntries accordingly.
            // metadata_cache_.InvalidateDirEntries(parent_dir);
        }

        // If O_TRUNC was set, go on processing the truncate request.
        if ((flags & SYSTEM_V_FCNTL.SYSTEM_V_FCNTL_H_O_TRUNC.getNumber()) > 0) {
            // TODO: Do this stuff if metadatacache is implemented.
            // Update mtime and ctime of the file if O_TRUNC was set.
            // metadata_cache_.UpdateStatTime(
            // path,
            // timestamp_s,
            // static_cast<Setattrs>(SETATTR_CTIME | SETATTR_MTIME));
            //
            // if (Logging::log->loggingActive(LEVEL_DEBUG)) {
            // Logging::log->getLog(LEVEL_DEBUG)
            // << "open called with O_TRUNC." << endl;
            // }
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
        // 1. Check if Stat object is cached.

        // if not, retrive stat from MRC
        getattrRequest request = getattrRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .setKnownEtag(0).build();

        getattrResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("getattr", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, getattrRequest.class });

            response = RPCCaller.<MRCServiceClient, getattrRequest, getattrResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
        }

        assert (response != null);

        Stat stat = response.getStbuf();
        if (stat.getNlink() > 1) { // Do not cache hardlinks
            // metadata_cache_.Invalidate(path); //TODO: Metadatastuff if cache is implemented.
        } else {
            // metadata_cache_.UpdateStat(path, *stat_buffer);
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

        timestampResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("setattr", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, setattrRequest.class });

            response = RPCCaller.<MRCServiceClient, setattrRequest, timestampResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
        }

        assert (response != null);

        // "chmod" or "chown" operations result into updating the ctime attribute.
        if (((toSet & Setattrs.SETATTR_MODE.getNumber()) > 0)
                || ((toSet & Setattrs.SETATTR_UID.getNumber()) > 0)
                || ((toSet & Setattrs.SETATTR_GID.getNumber()) > 0)) {

            toSet = (toSet | Setattrs.SETATTR_CTIME.getNumber());
            stat = stat.toBuilder().setAtimeNs(response.getTimestampS() * 1000000000).build();
        }

        // Do not cache hardlinks or chmod operations which try to set the SGID bit
        // as it might get cleared by the MRC.
        if (stat.getNlink() > 1
                || (((toSet & Setattrs.SETATTR_MODE.getNumber()) > 0) && ((stat.getMode() & (1 << 10))) > 0)) {
            // metadata_cache_.Invalidate(path); //TODO: metadatacache
        } else {
            // metadata_cache_.UpdateStatAttributes(path, stat, toSet);
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
        unlinkRequest request = unlinkRequest.newBuilder().setPath(path).setVolumeName(volumeName).build();

        unlinkResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("unlink", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, unlinkRequest.class });

            response = RPCCaller.<MRCServiceClient, unlinkRequest, unlinkResponse> makeCall(mrcServiceClient,
                    m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        assert (response != null);

        // TODO: Invalidate Metadatacache as soon as it is implemented.

        // Delete objects of all replicas on the OSDs.
        if (response.hasCreds()) {
            unlinkAtOsd(response.getCreds(), path);
        }
    }

    private void unlinkAtOsd(FileCredentials fc, String path) throws IOException {

        XLocSet xlocs = fc.getXlocs();

        unlink_osd_Request request = unlink_osd_Request.newBuilder().setFileCredentials(fc)
                .setFileId(fc.getXcap().getFileId()).build();

        UUIDIterator osdUuidIterator = new UUIDIterator();

        // Remove _all_ replicas.
        for (int i = 0; i < xlocs.getReplicasCount(); i++) {
            osdUuidIterator.clearAndAddUUID(Helper.getOSDUUIDFromXlocSet(xlocs, i, 0));

            try {
                Method m = OSDServiceClient.class.getDeclaredMethod("unlink",
                        new Class<?>[] { InetSocketAddress.class, Auth.class, UserCredentials.class,
                                unlink_osd_Request.class });
                RPCCaller.<OSDServiceClient, unlink_osd_Request, emptyResponse> makeCall(osdServiceClient, m,
                        userCredentialsBogus, authBogus, request, osdUuidIterator, uuidResolver,
                        volumeOptions.getMaxTries(), volumeOptions, false);

            } catch (NoSuchMethodException nsm) {
                // should never happen unless there is a programming error
                nsm.printStackTrace();
            } catch (SecurityException se) {
                // should never happen unless there is a programming error
                se.printStackTrace();
            } catch (Exception e) {
                throw new IOException(e);
                // TODO: Find out when to throw a PosixErrorExcpetion.
            }
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

        renameResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("rename", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, renameRequest.class });

            response = RPCCaller.<MRCServiceClient, renameRequest, renameResponse> makeCall(mrcServiceClient,
                    m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        assert (response != null);

        // 2. Remove file content of any previous files at "newPath".
        if (response.hasCreds()) {
            unlinkAtOsd(response.getCreds(), newPath);
        }

        // 3. Update caches // pseiferth: Implement metadatacache first.
        // Update the timestamps of parents of both directories.
        // const std::string parent_path = ResolveParentDirectory(path);
        // const std::string parent_new_path = ResolveParentDirectory(new_path);
        // if (response->response()->timestamp_s() != 0) {
        // metadata_cache_.UpdateStatTime(
        // parent_path,
        // response->response()->timestamp_s(),
        // static_cast<Setattrs>(SETATTR_CTIME | SETATTR_MTIME));
        // metadata_cache_.UpdateStatTime(
        // parent_new_path,
        // response->response()->timestamp_s(),
        // static_cast<Setattrs>(SETATTR_CTIME | SETATTR_MTIME));
        // }
        // metadata_cache_.InvalidateDirEntry(parent_path, GetBasename(path));
        // // TODO(mberlin): Add DirEntry instead to parent_new_path if stat available.
        // metadata_cache_.InvalidateDirEntries(parent_new_path);
        // // Overwrite an existing entry; no Prefix() operation needed because:
        // // "If new names an existing directory, it shall be required to be an empty
        // // directory."
        // // see http://pubs.opengroup.org/onlinepubs/009695399/functions/rename.html
        // metadata_cache_.Invalidate(new_path);
        // // Rename all affected entries.
        // metadata_cache_.RenamePrefix(path, new_path);
        // // http://pubs.opengroup.org/onlinepubs/009695399/functions/rename.html:
        // // "Some implementations mark for update the st_ctime field of renamed files
        // // and some do not."
        // // => XtreemFS does so, i.e. update the client's cache, too.
        // metadata_cache_.UpdateStatTime(new_path,
        // response->response()->timestamp_s(),
        // static_cast<Setattrs>(SETATTR_CTIME));

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

        timestampResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("mkdir", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, mkdirRequest.class });

            response = RPCCaller.<MRCServiceClient, mkdirRequest, timestampResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        assert (response != null);

        // TODO: Update Metadata Cache
        // const string parent_dir = ResolveParentDirectory(path);
        // metadata_cache_.UpdateStatTime(
        // parent_dir,
        // response->response()->timestamp_s(),
        // static_cast<Setattrs>(SETATTR_CTIME | SETATTR_MTIME));
        // // TODO(mberlin): Retrieve stat as optional member of openResponse instead
        // // and update cached DirectoryEntries accordingly.
        // metadata_cache_.InvalidateDirEntries(parent_dir);
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

        timestampResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("rmdir", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, rmdirRequest.class });

            response = RPCCaller.<MRCServiceClient, rmdirRequest, timestampResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        assert (response != null);

        // TODO: Update metadata cache!
        // const string parent_dir = ResolveParentDirectory(path);
        // metadata_cache_.UpdateStatTime(
        // parent_dir,
        // response->response()->timestamp_s(),
        // static_cast<Setattrs>(SETATTR_CTIME | SETATTR_MTIME));
        // metadata_cache_.InvalidatePrefix(path);
        // metadata_cache_.InvalidateDirEntry(parent_dir, GetBasename(path));

    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.xtreemfs.common.libxtreemfs.Volume#readDir(org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC
     * .UserCredentials, java.lang.String, long, long, boolean)
     */
    @Override
    public DirectoryEntries readDir(UserCredentials userCredentials, String path, long offset, long count,
            boolean namesOnly) throws IOException {

        DirectoryEntries result = null;

        // TODO: get result from metadatacache as soon as it is implemented.

        DirectoryEntries.Builder dirEntriesBuilder = DirectoryEntries.newBuilder();

        // Process large requests in multiples of readdirChunkSize.
        for (long currentOffset = offset; currentOffset < offset + count; offset += volumeOptions
                .getReaddirChunkSize()) {

            long limitDirEntriesCountLong = (currentOffset > offset + count) ? (currentOffset - offset - count)
                    : volumeOptions.getReaddirChunkSize();

            // cast the long to int because readdir RPC Operation won't work with long values.
            int limitDirEntriesCount = (int) limitDirEntriesCountLong;
            if ((long) limitDirEntriesCount != limitDirEntriesCountLong) {
                throw new IllegalArgumentException(limitDirEntriesCountLong
                        + " cannot be cast to int without changing its value.");
            }

            readdirRequest request = readdirRequest.newBuilder().setPath(path).setVolumeName(volumeName)
                    .setNamesOnly(namesOnly).setKnownEtag(0).setSeenDirectoryEntriesCount(currentOffset)
                    .setLimitDirectoryEntriesCount(limitDirEntriesCount).build();

            DirectoryEntries response = null;
            try {
                Method m = MRCServiceClient.class.getDeclaredMethod("readdir", new Class<?>[] {
                        InetSocketAddress.class, Auth.class, UserCredentials.class, readdirRequest.class });

                response = RPCCaller.<MRCServiceClient, readdirRequest, DirectoryEntries> makeCall(
                        mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator,
                        uuidResolver, volumeOptions.getMaxTries(), volumeOptions, false);
            } catch (NoSuchMethodException nsm) {
                // should never happen unless there is a programming error
                nsm.printStackTrace();
            } catch (SecurityException se) {
                // should never happen unless there is a programming error
                se.printStackTrace();
            } catch (Exception e) {
                throw new IOException(e);
                // TODO: Find out when to throw a PosixErrorExcpetion.
            }

            assert (response != null);

            dirEntriesBuilder.addAllEntries(response.getEntriesList());

            // Break if this is the last chunk.
            if (response.getEntriesCount() < (currentOffset + volumeOptions.getReaddirChunkSize())) {
                break;
            }
        }

        // TODO: Use metadatachache when it is implemented.
        // TODO: Merge possible pending file size updates of files into
        // the stat entries of listed files.

        // Cache the first stat buffers that fit into the cache.
        // for (int i = 0;
        // i < min(volume_options_.metadata_cache_size,
        // static_cast<boost::uint64_t>(result->entries_size()));
        // i++) {
        // if (result->entries(i).stbuf().nlink() > 1) { // Do not cache hard links.
        // metadata_cache_.Invalidate(path);
        // } else {
        // metadata_cache_.UpdateStat(
        // ConcatenatePath(path, result->entries(i).name()),
        // result->entries(i).stbuf());
        // }
        // }

        // Cache the result if it's the complete directory.
        // We can't tell for sure whether result contains all directory entries if
        // it's size is not less than the requested "count".
        // TODO(mberlin): Cache only names and no stat entries and remove names_only
        // condition.
        // TODO(mberlin): Set an upper bound of dentries, otherwise don't cache it.
        // if (offset == 0 && result->entries_size() < count && !names_only) {
        // metadata_cache_.UpdateDirEntries(path, *result);
        // }

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

        listxattrResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("listxattr", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, listxattrRequest.class });

            response = RPCCaller.<MRCServiceClient, listxattrRequest, listxattrResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        assert (response != null);

        // TODO: Update Metadatacache

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
        // TODO Auto-generated method stub
        setxattrRequest request = setxattrRequest.newBuilder().setVolumeName(volumeName).setPath(path)
                .setName(name).setValue(value).setFlags(flags).build();

        timestampResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("setxattr", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, setxattrRequest.class });

            response = RPCCaller.<MRCServiceClient, setxattrRequest, timestampResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        assert (response != null);

        // TODO: Update metadatacache.
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

            getxattrResponse response = null;
            try {
                Method m = MRCServiceClient.class.getDeclaredMethod("getxattr", new Class<?>[] {
                        InetSocketAddress.class, Auth.class, UserCredentials.class, getxattrRequest.class });

                response = RPCCaller.<MRCServiceClient, getxattrRequest, getxattrResponse> makeCall(
                        mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator,
                        uuidResolver, volumeOptions.getMaxTries(), volumeOptions, false);
            } catch (NoSuchMethodException nsm) {
                // should never happen unless there is a programming error
                nsm.printStackTrace();
            } catch (SecurityException se) {
                // should never happen unless there is a programming error
                se.printStackTrace();
            } catch (Exception e) {
                throw new IOException(e);
                // TODO: Find out when to throw a PosixErrorExcpetion.
            }

            assert (response != null);

            if (response.hasValue()) {
                return response.getValue();
            } else {
                return null;
            }

        } else {
            // No "xtreemfs." attribute, lookup metadata cache.

            // TODO: lookup with metadatacache

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
            // TODO: check if attribute is in metadatacache
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

        timestampResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("removexattr", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, removexattrRequest.class });

            response = RPCCaller.<MRCServiceClient, removexattrRequest, timestampResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        // TODO: Update metadatacache when it is implemented.

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

        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("xtreemfs_replica_add", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class,
                    xtreemfs_replica_addRequest.class });

            RPCCaller.<MRCServiceClient, xtreemfs_replica_addRequest, emptyResponse> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

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

        Replicas response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("xtreemfs_replica_list", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class,
                    xtreemfs_replica_listRequest.class });

            response = RPCCaller.<MRCServiceClient, xtreemfs_replica_listRequest, Replicas> makeCall(
                    mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

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
        // TODO Auto-generated method stub

        // remove the replica
        xtreemfs_replica_removeRequest request = xtreemfs_replica_removeRequest.newBuilder()
                .setVolumeName(volumeName).setPath(path).setOsdUuid(osdUuid).build();

        FileCredentials response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("xtreemfs_replica_remove", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class,
                    xtreemfs_replica_removeRequest.class });

            response = RPCCaller
                    .<MRCServiceClient, xtreemfs_replica_removeRequest, FileCredentials> makeCall(
                            mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator,
                            uuidResolver, volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        assert (response != null);

        // Now unlink the replica at the OSD.
        UUIDIterator osdUuidIterator = new UUIDIterator();
        osdUuidIterator.addUUID(osdUuid);

        unlink_osd_Request request2 = unlink_osd_Request.newBuilder()
                .setFileId(response.getXcap().getFileId()).setFileCredentials(response).build();

        try {
            Method m = OSDServiceClient.class.getDeclaredMethod("unlink", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class, unlink_osd_Request.class });

            RPCCaller.<OSDServiceClient, unlink_osd_Request, emptyResponse> makeCall(osdServiceClient, m,
                    userCredentials, authBogus, request2, osdUuidIterator, uuidResolver,
                    volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }
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

        xtreemfs_get_suitable_osdsResponse response = null;
        try {
            Method m = MRCServiceClient.class.getDeclaredMethod("xtreemfs_replica_remove", new Class<?>[] {
                    InetSocketAddress.class, Auth.class, UserCredentials.class,
                    xtreemfs_get_suitable_osdsRequest.class });

            response = RPCCaller
                    .<MRCServiceClient, xtreemfs_get_suitable_osdsRequest, xtreemfs_get_suitable_osdsResponse> makeCall(
                            mrcServiceClient, m, userCredentials, authBogus, request, mrcUUIDIterator,
                            uuidResolver, volumeOptions.getMaxTries(), volumeOptions, false);
        } catch (NoSuchMethodException nsm) {
            // should never happen unless there is a programming error
            nsm.printStackTrace();
        } catch (SecurityException se) {
            // should never happen unless there is a programming error
            se.printStackTrace();
        } catch (Exception e) {
            throw new IOException(e);
            // TODO: Find out when to throw a PosixErrorExcpetion.
        }

        assert (response != null);

        return response.getOsdUuidsList();
    }
    
    /** 
     * Called by FileHandle.Close() to remove file_handle from the list. 
     */
    protected void closeFile(long fileId, FileInfo fileInfo, FileHandleImplementation fileHandle) {
        //TODO: Implement this method correctly.
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
            fileInfo.UpdateXLocSetAndRest(xlocset, replicateOnClose);
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

    protected Auth getAuthBogus() {
        return this.authBogus;
    }

    protected UserCredentials getUserCredentialsBogus() {
        return this.userCredentialsBogus;
    }

}
