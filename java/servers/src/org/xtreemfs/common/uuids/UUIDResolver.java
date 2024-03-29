/*
 * Copyright (c) 2008-2011 by Bjoern Kolbeck,
 *               Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

package org.xtreemfs.common.uuids;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import org.xtreemfs.common.GlobalConstants;

import org.xtreemfs.common.util.NetUtils;
import org.xtreemfs.dir.DIRClient;
import org.xtreemfs.foundation.TimeSync;
import org.xtreemfs.foundation.logging.Logging;
import org.xtreemfs.foundation.logging.Logging.Category;
import org.xtreemfs.foundation.pbrpc.Schemes;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.UserCredentials;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.AddressMapping;
import org.xtreemfs.pbrpc.generatedinterfaces.DIR.AddressMappingSet;

/**
 * Resolves UUID to InetSocketAddress+Protocol mappings.
 * 
 * @author bjko
 */
public final class UUIDResolver extends Thread {
    
    Map<String, UUIDCacheEntry>   cache;
    
    protected transient boolean   quit;
    
    protected final DIRClient     dir;
    
    protected final List<String>  myNetworks;
    
    /**
     * interval between two cache cleanups/renewals in milliseconds
     */
    public final int              cacheCleanInterval;
    
    public final int              maxUnusedEntry;
    
    protected static UUIDResolver theInstance;

    protected final UserCredentials uc;
    
    protected UUIDResolver(DIRClient client, int cacheCleanInterval, int maxUnusedEntry, boolean singleton)
        throws IOException {
        
        super("UUID Resolver");
        setDaemon(true);
        
        cache = new ConcurrentHashMap<String, UUIDCacheEntry>();
        quit = false;
        this.dir = client;
        this.maxUnusedEntry = maxUnusedEntry;
        this.cacheCleanInterval = cacheCleanInterval;
        this.uc = UserCredentials.newBuilder().setUsername("uuidresolver").addGroups("xtreemfs-services").build();
        
        if (singleton) {
            assert (theInstance == null);
            theInstance = this;
        }
        
        List<AddressMapping.Builder> ntwrks = NetUtils.getReachableEndpoints(0, "http");
        myNetworks = new ArrayList(ntwrks.size());
        for (AddressMapping.Builder network : ntwrks) {
            myNetworks.add(network.getMatchNetwork());
        }
    }
    
    /**
     * Starts the UUIDResolver thread.
     * 
     * @param client
     *            a DIRClient used to resolve non-cached and non-local mappings
     * @param cacheCleanInterval
     *            the interval between two cleanup/renewals of cache entries (in
     *            ms)
     * @param maxUnusedEntry
     *            the duration for which to keep an unused entry (in ms, should
     *            be set to several tens of minutes)
     * @throws org.xtreemfs.foundation.json.JSONException
     * @throws java.io.IOException
     */
    public static synchronized void start(DIRClient client, int cacheCleanInterval, int maxUnusedEntry)
        throws IOException {
        if (theInstance == null) {
            new UUIDResolver(client, cacheCleanInterval, maxUnusedEntry, true);
            theInstance.start();
            if (Logging.isInfo())
                Logging.logMessage(Logging.LEVEL_INFO, Category.lifecycle, null, "started UUIDResolver",
                    new Object[0]);
        } else {
            if (Logging.isInfo())
                Logging.logMessage(Logging.LEVEL_INFO, Category.lifecycle, null,
                    "UUIDResolver already running!", new Object[0]);
        }
    }
    
    public static synchronized UUIDResolver startNonSingelton(DIRClient client, int cacheCleanInterval,
        int maxUnusedEntry) throws IOException {
        UUIDResolver tmp = new UUIDResolver(client, cacheCleanInterval, maxUnusedEntry, false);
        tmp.start();
        return tmp;
    }
    
    public static boolean isRunning() {
        return theInstance != null;
    }
    
    static UUIDCacheEntry resolve(String uuid) throws UnknownUUIDException {
        assert (theInstance != null);
        
        UUIDCacheEntry entry = theInstance.cache.get(uuid);
        // check if it is still valid
        if ((entry != null) && (entry.getValidUntil() > TimeSync.getLocalSystemTime())) {
            entry.setLastAccess(TimeSync.getLocalSystemTime());
            return entry;
        }
        return theInstance.fetchUUID(uuid);
    }
    
    static UUIDCacheEntry resolve(String uuid, UUIDResolver nonSingleton) throws UnknownUUIDException {
        
        UUIDCacheEntry entry = nonSingleton.cache.get(uuid);
        // check if it is still valid
        if ((entry != null) && (entry.getValidUntil() > TimeSync.getLocalSystemTime())) {
            entry.setLastAccess(TimeSync.getLocalSystemTime());
            return entry;
        }
        return nonSingleton.fetchUUID(uuid);
    }
    
    UUIDCacheEntry fetchUUID(String uuid) throws UnknownUUIDException {
        if (dir == null)
            throw new UnknownUUIDException("there is no mapping for " + uuid
                + ". Attention: local mode enabled, no remote lookup possible.");
        if (Logging.isDebug())
            Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "loading uuid mapping for %s", uuid);
        try {
            AddressMappingSet ams = dir.xtreemfs_address_mappings_get(null, GlobalConstants.AUTH_NONE, uc, uuid);
            if (Logging.isDebug())
                Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "sent request to DIR");
            if (Logging.isDebug())
                Logging
                        .logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "received response for %s",
                            uuid);
            if (ams.getMappingsCount() == 0) {
                if (Logging.isDebug())
                    Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "NO UUID MAPPING FOR: %s",
                        uuid);
                throw new UnknownUUIDException("uuid " + uuid + " is not registered at directory server");
            }
            for (AddressMapping addrMapping : ams.getMappingsList()) {
                final String network = addrMapping.getMatchNetwork();
                if (myNetworks.contains(network) || (network.equals("*"))) {
                    final String address = addrMapping.getAddress();
                    final String protocol = addrMapping.getProtocol();
                    final int port = addrMapping.getPort();
                    final long validUntil = TimeSync.getLocalSystemTime() + addrMapping.getTtlS() * 1000;
                    final InetSocketAddress endpoint = new InetSocketAddress(address, port);
                    if (Logging.isDebug())
                        Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this,
                            "matching uuid record found for uuid " + uuid + " with network " + network);
                    UUIDCacheEntry e = new UUIDCacheEntry(uuid, validUntil, new Mapping(protocol, endpoint));
                    cache.put(uuid, e);
                    return e;
                }
            }
            if (Logging.isDebug())
                Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this, "NO UUID MAPPING FOR: %s", uuid);
            throw new UnknownUUIDException(
                "there is no matching entry for my network in the uuid address mapping. The service at "
                    + uuid
                    + " is either not reachable from this machine or the mapping entry is misconfigured.");
        } catch (InterruptedException ex) {
            throw new UnknownUUIDException("cannot retrieve mapping from server due to IO error: " + ex);
        } catch (IOException ex) {
            throw new UnknownUUIDException("cannot retrieve mapping from server due to IO error: " + ex);
        } catch (Exception ex) {
            ex.printStackTrace();
            throw new UnknownUUIDException(
                "cannot retrieve mapping from server due to invalid data sent by the server: " + ex);
        }
    }
    
    @Override
    public void run() {
        List<UUIDCacheEntry> updates = new LinkedList();
        do {
            Iterator<UUIDCacheEntry> iter = cache.values().iterator();
            while (iter.hasNext()) {
                final UUIDCacheEntry entry = iter.next();
                if (entry.isSticky())
                    continue;
                if (entry.getLastAccess() + maxUnusedEntry < TimeSync.getLocalSystemTime()) {
                    // dump entry!
                    iter.remove();
                    if (Logging.isDebug())
                        Logging.logMessage(Logging.LEVEL_DEBUG, Category.misc, this,
                            "removed entry from UUID cache: %s", entry.getUuid());
                } else {
                    // check if update is necessary
                    if (entry.getValidUntil() < TimeSync.getLocalSystemTime() + cacheCleanInterval) {
                        // renew entry...
                        try {
                            updates.add(fetchUUID(entry.getUuid()));
                        } catch (Exception ex) {
                            Logging.logMessage(Logging.LEVEL_WARN, Category.misc, this,
                                "cannot refresh UIID mapping: %s", ex.toString());
                            iter.remove();
                        }
                    }
                }
                
            }
            try {
                sleep(cacheCleanInterval);
            } catch (InterruptedException ex) {
            }
        } while (!quit);
    }
    
    /**
     * Add a UUID which is mapped on localhost
     * 
     * @param localUUID
     *            the UUID to map
     * @param port
     *            the port to map the UUID to
     * @param useSSL
     *            defines the protocol
     */
    public static void addLocalMapping(String localUUID, int port, String protocol) {
        assert (theInstance != null);
        
        UUIDCacheEntry e = theInstance.cache.get(localUUID);
        if (e == null)
            e = new UUIDCacheEntry(localUUID, Long.MAX_VALUE, new Mapping(protocol, new InetSocketAddress(
                "localhost", port)));
        else
            e.addMapping(new Mapping(protocol, new InetSocketAddress("localhost", port)));
        
        e.setSticky(true);
        theInstance.cache.put(localUUID, e);
    }
    
    public static void addLocalMapping(ServiceUUID uuid, int port, String protocol) {
        addLocalMapping(uuid.toString(), port, protocol);
    }
    
    public static void addTestMapping(String uuid, String hostname, int port, boolean useSSL) {
        assert (theInstance != null);
        
        final String protocol = useSSL ? Schemes.SCHEME_PBRPCS : Schemes.SCHEME_PBRPC;
        
        UUIDCacheEntry e = theInstance.cache.get(uuid);
        if (e == null)
            e = new UUIDCacheEntry(uuid, Long.MAX_VALUE, new Mapping(protocol, new InetSocketAddress(
                hostname, port)));
        else
            e.addMapping(new Mapping(protocol, new InetSocketAddress(hostname, port)));
        
        e.setSticky(true);
        theInstance.cache.put(uuid, e);
    }
    
    public static void shutdown(UUIDResolver nonSingleton) {
        nonSingleton.quit = true;
        nonSingleton.interrupt();
    }
    
    public static void shutdown() {
        if (theInstance != null) {
            theInstance.quit = true;
            theInstance.interrupt();
            theInstance = null;
            if (Logging.isInfo())
                Logging.logMessage(Logging.LEVEL_INFO, Category.lifecycle, null, "UUIDREsolver shut down",
                    new Object[0]);
        } else {
            if (Logging.isInfo())
                Logging.logMessage(Logging.LEVEL_INFO, Category.lifecycle, null,
                    "UUIDREsolver was already shut down or is not running", new Object[0]);
        }
    }
    
    public static String getCache() {
        StringBuilder sb = new StringBuilder();
        for (UUIDCacheEntry e : theInstance.cache.values()) {
            sb.append(e.getUuid());
            sb.append(" -> ");
            for (Mapping mapping : e.getMappings()) {
                sb.append(mapping.protocol);
                sb.append("://");
                sb.append(mapping.resolvedAddr);
                sb.append(" ");
            }
            if (e.isSticky()) {
                sb.append(" - STICKY");
            } else {
                sb.append(" - valid for ");
                sb.append((e.getValidUntil() - TimeSync.getLocalSystemTime()) / 1000l);
                sb.append("s");
            }
            sb.append("\n");
        }
        return sb.toString();
    }
    
}
