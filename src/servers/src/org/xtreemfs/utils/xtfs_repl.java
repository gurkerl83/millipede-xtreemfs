/*  Copyright (c) 2008 Barcelona Supercomputing Center - Centro Nacional
    de Supercomputacion and Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

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
 * AUTHORS: Christian Lorenz (ZIB)
 */
package org.xtreemfs.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.Map.Entry;
import java.util.regex.Pattern;

import org.xtreemfs.common.TimeSync;
import org.xtreemfs.common.clients.io.RandomAccessFile;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.logging.Logging.Category;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.common.uuids.UUIDResolver;
import org.xtreemfs.common.uuids.UnknownUUIDException;
import org.xtreemfs.common.xloc.Replica;
import org.xtreemfs.common.xloc.ReplicationFlags;
import org.xtreemfs.common.xloc.StripingPolicyImpl;
import org.xtreemfs.common.xloc.XLocations;
import org.xtreemfs.dir.client.DIRClient;
import org.xtreemfs.foundation.SSLOptions;
import org.xtreemfs.foundation.oncrpc.client.RPCNIOSocketClient;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.interfaces.FileCredentials;
import org.xtreemfs.interfaces.ObjectData;
import org.xtreemfs.interfaces.ServiceSet;
import org.xtreemfs.interfaces.StringSet;
import org.xtreemfs.interfaces.UserCredentials;
import org.xtreemfs.interfaces.utils.ONCRPCException;
import org.xtreemfs.mrc.client.MRCClient;
import org.xtreemfs.osd.client.OSDClient;
import org.xtreemfs.utils.CLIParser.CliOption;

/**
 * A tool to manage your Replicas. File can be marked as read-only, replicas can be added, ... <br>
 * 06.04.2009
 */
public class xtfs_repl {
    public final static String      ADD_REPLICA                        = "a";

    public final static String      ADD_REPLICA_INTERACTIVE            = "-interactive_add";

    public final static String      ADD_AUTOMATIC_REPLICA              = "-auto_add";

    public final static String      REMOVE_REPLICA                     = "r";

    public final static String      REMOVE_REPLICA_INTERACTIVE         = "-interactive_remove";

    public final static String      REMOVE_AUTOMATIC_REPLICA           = "-auto_remove";

    public final static String      SET_READ_ONLY                      = "-set_readonly";

    public final static String      SET_WRITABLE                       = "-set_writeable";

    public final static String      IS_READ_ONLY                       = "-is_readonly";

    public final static String      LIST_REPLICAS                      = "l";

    public final static String      LIST_SUITABLE_OSDS_FOR_REPLICA     = "o";

    public final static String      HELP                               = "h";

    public final static String      HELP_LONG                          = "-help";

    public final static String      METHOD_RANDOM                      = "random";

    public final static String      METHOD_DNS                         = "dns";

    public final static String      REPLICATION_FLAG_FULL_REPLICA     = "-full";

    public final static String      REPLICATION_FLAG_TRANSFER_STRATEGY = "-strategy";

    public final static String      TRANSFER_STRATEGY_RANDOM           = "random";

    public final static String      TRANSFER_STRATEGY_SEQUENTIAL       = "sequential";

    public final static String      SSL_CREDS_FILE                     = "c";

    public final static String      SSL_CREDS_PASSWORD                 = "cpass";

    public final static String      SSL_TRUSTED_CA_FILE                = "t";

    public final static String      SSL_TRUSTED_CA_PASSWORD            = "tpass";

    public final static int         DEFAULT_REPLICATION_FLAGS          = ReplicationFlags
                                                                               .setPartialReplica(ReplicationFlags
                                                                                       .setRandomStrategy(0));

    private final String            relPath;

    private final String            volPath;

    private RandomAccessFile        file;

    private MRCClient               mrcClient;

    public final UserCredentials    credentials;

    public final String             volume;

    private final InetSocketAddress dirAddress;

    private final DIRClient         dirClient;

    private InetSocketAddress       mrcAddress;

    private XLocations              xLoc;

    private RPCNIOSocketClient      client;

    private TimeSync                timeSync;

    public static final Pattern     IPV4_PATTERN                       = Pattern
                                                                               .compile("b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).)"
                                                                                       + "{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)b");

    public static final Pattern     IPV6_PATTERN                       = Pattern
                                                                               .compile(
                                                                                       "((([0-9a-f]{1,4}+:){7}+[0-9a-f]{1,4}+)|(:(:[0-9a-f]"
                                                                                               + "{1,4}+){1,6}+)|(([0-9a-f]{1,4}+:){1,6}+:)|(::)|(([0-9a-f]"
                                                                                               + "{1,4}+:)(:[0-9a-f]{1,4}+){1,5}+)|(([0-9a-f]{1,4}+:){1,2}"
                                                                                               + "+(:[0-9a-f]{1,4}+){1,4}+)|(([0-9a-f]{1,4}+:){1,3}+(:[0-9a-f]{1,4}+)"
                                                                                               + "{1,3}+)|(([0-9a-f]{1,4}+:){1,4}+(:[0-9a-f]{1,4}+){1,2}+)|(([0-9a-f]"
                                                                                               + "{1,4}+:){1,5}+(:[0-9a-f]{1,4}+))|(((([0-9a-f]{1,4}+:)?([0-9a-f]"
                                                                                               + "{1,4}+:)?([0-9a-f]{1,4}+:)?([0-9a-f]{1,4}+:)?)|:)(:(([0-9]{1,3}+\\.)"
                                                                                               + "{3}+[0-9]{1,3}+)))|(:(:[0-9a-f]{1,4}+)*:([0-9]{1,3}+\\.){3}+[0-9]"
                                                                                               + "{1,3}+))(/[0-9]+)?",
                                                                                       Pattern.CASE_INSENSITIVE);

    /**
     * required for METHOD_DNS <br>
     * 13.05.2009
     */
    private static final class UsableOSD implements Comparable {
        public ServiceUUID osd;

        public int         match;

        public UsableOSD(ServiceUUID uuid, int match) {
            this.match = match;
            this.osd = uuid;
        }

        @Override
        public int compareTo(Object o) {
            UsableOSD other = (UsableOSD) o;
            return other.match - this.match;
        }
    }

    /**
     * @param sslOptions TODO
     * @throws IOException
     * @throws InterruptedException
     * @throws ONCRPCException
     * 
     */
    public xtfs_repl(String relPath, InetSocketAddress dirAddress, String volume, String volPath, SSLOptions sslOptions)
            throws Exception {
        try {
            Logging.start(Logging.LEVEL_ERROR, Category.tool);

            this.relPath = relPath;
            this.volPath = volPath;
            this.volume = volume;
            this.dirAddress = dirAddress;

            // TODO: use REAL user credentials (this is a SECURITY HOLE)
            StringSet groupIDs = new StringSet();
            groupIDs.add("root");
            this.credentials = new UserCredentials("root", groupIDs, "");

            // client
            client = new RPCNIOSocketClient(sslOptions, 10000, 5 * 60 * 1000);
            client.start();
            client.waitForStartup();
            dirClient = new DIRClient(client, dirAddress);

            // start services
            timeSync = TimeSync.initialize(dirClient, 60 * 1000, 50);
            timeSync.waitForStartup();

            UUIDResolver.start(dirClient, 1000, 10 * 10 * 1000);
        } catch (Exception e) {
            shutdown();
            throw e;
        }
    }

    /**
     * @throws ONCRPCException
     * @throws IOException
     * @throws InterruptedException
     */
    public void initialize() throws ONCRPCException, IOException, InterruptedException {
        ServiceSet sSet;
        // get MRC address
        RPCResponse<ServiceSet> r = dirClient.xtreemfs_service_get_by_name(dirAddress, volume);
        sSet = r.get();
        r.freeBuffers();

        if (sSet.size() != 0)
            mrcAddress = new ServiceUUID(sSet.get(0).getData().get("mrc")).getAddress();
        else {
            System.err.println("unknown volume");
            System.exit(1);
        }

        this.mrcClient = new MRCClient(client, mrcAddress);

        File f = new File(relPath);
        if (!f.isFile()) {
            System.err.println("'" + relPath + "' is not a file");
            System.exit(1);
        }

        this.file = new RandomAccessFile("r", mrcAddress, volume + volPath, client, credentials);

        xLoc = new XLocations(file.getCredentials().getXlocs());
    }

    // interactive
    public void addReplica() throws Exception {
        if (file.isReadOnly()) {
            List<ServiceUUID> suitableOSDs = listSuitableOSDs();

            // get OSDs
            String[] osdNumbers = null;
            BufferedReader in = null;
            while (true) {
                try {
                    // at the moment all replicas must have the same StripingPolicy
                    in = new BufferedReader(new InputStreamReader(System.in));
                    System.out.println("Please select " + file.getStripingPolicy().getWidth()
                            + " OSD(s) which should be used for the replica.");
                    System.out
                            .println("# Select the OSD(s) through the prefix-numbers and use ',' as seperator. #");
                    osdNumbers = in.readLine().split(",");

                    // correct count of OSDs
                    if (osdNumbers.length != file.getStripingPolicy().getWidth()) {
                        System.out.println("Please try it again");
                        continue;
                    }
                    break;
                } catch (IOException e) {
                    System.out.println("Please try it again");
                    continue;
                } finally {
                    if (in != null)
                        in.close();
                }
            }

            // create list with selected OSDs for replica
            List<ServiceUUID> osds = new ArrayList<ServiceUUID>(file.getStripingPolicy().getWidth());
            for (String osdNumber : osdNumbers) {
                osds.add(suitableOSDs.get(Integer.parseInt(osdNumber) - 1));
            }

            // get replication flags
            String[] args = null;
            int replicationFlags = 0;
            while (true) {
                try {
                    in = new BufferedReader(new InputStreamReader(System.in));
                    System.out
                            .println("Please choose the type of the replica (full | partial)"
                                    + " and a Transfer Strategy (random | sequential).");
                    System.out.println("# Please use ',' as seperator. #");
                    args = in.readLine().split(",");

                    List<String> argsList = Arrays.asList(args);
                    if (argsList.contains("partial"))
                        replicationFlags = ReplicationFlags.setPartialReplica(replicationFlags);
                    if (argsList.contains("full")) {
                        replicationFlags = ReplicationFlags.setFullReplica(replicationFlags);
                    }
                    if (argsList.contains("random"))
                        replicationFlags = ReplicationFlags.setRandomStrategy(replicationFlags);
                    if (argsList.contains("sequential"))
                        replicationFlags = ReplicationFlags.setSequentialStrategy(replicationFlags);

                    break;
                } catch (IOException e) {
                    System.out.println("Please try it again");
                    continue;
                } finally {
                    if (in != null)
                        in.close();
                }
            }

            addReplica(osds, replicationFlags);
        } else
            System.err.println("File is not marked as read-only.");
    }

    public void addReplica(List<ServiceUUID> osds, int replicationFlags) throws Exception {
        if (file.isReadOnly()) {
            // at the moment all replicas must have the same StripingPolicy
            file.addReplica(osds, file.getStripingPolicy(), replicationFlags);

            if (ReplicationFlags.isFullReplica(replicationFlags))
                startReplicationOnOSDs(osds.get(0));
        } else
            System.err.println("File is not marked as read-only.");
    }

    // automatic
    public void addReplicaAutomatically(String method, int replicationFlags) throws Exception {
        if (file.isReadOnly()) {
            List<ServiceUUID> suitableOSDs = file.getSuitableOSDsForAReplica();
            if (suitableOSDs.size() == 0) {
                System.err.println("could not create replica: no suitable OSDs available");
                System.exit(1);
            }

            List<ServiceUUID> osds = new ArrayList<ServiceUUID>(file.getStripingPolicy().getWidth());

            // create list with OSDs for replica
            if (method.equals(METHOD_RANDOM)) {
                Random random = new Random();
                int count = 0;
                while (count < file.getStripingPolicy().getWidth()) {
                    ServiceUUID nextOSD = suitableOSDs.get(random.nextInt(suitableOSDs.size()));
                    if (!osds.contains(nextOSD)) {
                        osds.add(nextOSD);
                        count++;
                    }
                }
            } else if (method.equals(METHOD_DNS)) {
                String clientFQDN = InetAddress.getLocalHost().getCanonicalHostName();
                // if the FQDN cannot be resolved, we get only an IP-address
                if (IPV4_PATTERN.matcher(clientFQDN).matches() || IPV6_PATTERN.matcher(clientFQDN).matches()) {
                    System.err
                            .println("The FQDN of this computer cannot be resolved. Please check your settings.");
                    System.exit(1);
                }

                int minPrefix = 0;

                PriorityQueue<UsableOSD> list = new PriorityQueue<UsableOSD>();
                for (ServiceUUID uuid : suitableOSDs) {
                    try {
                        final String osdHostName = uuid.getAddress().getAddress().getCanonicalHostName();
                        final int minLen = (osdHostName.length() > clientFQDN.length()) ? clientFQDN.length()
                                : osdHostName.length();
                        int osdI = osdHostName.length() - 1;
                        int clientI = clientFQDN.length() - 1;
                        int match = 0;
                        for (int i = minLen - 1; i > 0; i--) {
                            if (osdHostName.charAt(osdI--) != clientFQDN.charAt(clientI--)) {
                                break;
                            }
                            match++;
                        }
                        if (match < minPrefix)
                            continue;

                        list.add(new UsableOSD(uuid, match));
                    } catch (UnknownUUIDException ex) {
                    }
                }

                // from the remaining set, take a subset of OSDs
                while (osds.size() < file.getStripingPolicy().getWidth()) {
                    final ServiceUUID osd = list.poll().osd;
                    if (osd == null)
                        break;
                    osds.add(osd);
                }
            }
            addReplica(osds, replicationFlags);
        } else
            System.err.println("File is not marked as read-only.");
    }

    /**
     * contacts the OSDs so they begin to replicate the file
     * 
     * @param addedOSD
     * @throws IOException
     */
    private void startReplicationOnOSDs(ServiceUUID addedOSD) throws IOException {
        // get just added replica
        Replica addedReplica = file.getXLoc().getReplica(addedOSD);
        if (addedReplica.isPartialReplica()) // break, because replica should not be filled
            return;
        StripingPolicyImpl sp = addedReplica.getStripingPolicy();
        String fileID = file.getFileId();
        FileCredentials cred = file.getCredentials();

        OSDClient osdClient = new OSDClient(client);
        // send requests to all OSDs of this replica
        try {
            List<ServiceUUID> osdList = addedReplica.getOSDs();
            // take lowest objects of file
            for (int objectNo = 0; osdList.size() == 0; objectNo++) {
                // get index of OSD for this object
                int indexOfOSD = sp.getOSDforObject(objectNo);
                // remove OSD
                ServiceUUID osd = osdList.remove(indexOfOSD);
                // send request (read only 1 byte)
                RPCResponse<ObjectData> r = osdClient.read(osd.getAddress(), fileID, cred, objectNo, 0, 0, 1);
                r.get();
                r.freeBuffers();
            }
        } catch (UnknownUUIDException e) {
            // ignore; should not happen
        } catch (ONCRPCException e) {
            throw new IOException("At least one OSD could not be contacted to replicate the file.", e);
        } catch (IOException e) {
            throw new IOException("At least one OSD could not be contacted to replicate the file.", e);
        } catch (InterruptedException e) {
            // ignore
        }
    }

    // interactive
    public void removeReplica() throws Exception {
        if (file.isReadOnly()) {
            printListOfReplicas(xLoc.getReplicas());

            int replicaNumber;
            BufferedReader in = null;
            while (true) {
                try {
                    // at the moment all replicas must have the same
                    // StripingPolicy
                    in = new BufferedReader(new InputStreamReader(System.in));
                    System.out.println("Please select a replica which should be removed.");
                    System.out.println("# Select the replica through the prefix-number. #");
                    replicaNumber = Integer.parseInt(in.readLine());

                    break;
                } catch (NumberFormatException e) {
                    System.out.println("Please try it again");
                    continue;
                } catch (IOException e) {
                    System.out.println("Please try it again");
                    continue;
                } finally {
                    if (in != null)
                        in.close();
                }
            }

            file.removeReplica(xLoc.getReplicas().get(replicaNumber - 1));
        } else
            System.err.println("File is not marked as read-only.");
    }

    public void removeReplica(ServiceUUID osd) throws Exception {
        if (file.isReadOnly()) {
            boolean noOtherCompleteReplica = true;
            if (file.getXLoc().getReplica(osd).isComplete()) { // complete replica
                // check if another replica is also complete
                for (Replica r : file.getXLoc().getReplicas())
                    if (r.isComplete()) {
                        noOtherCompleteReplica = false;
                        break;
                    }
                if (!noOtherCompleteReplica)
                    file.removeReplica(osd);
                else
                    System.err.println("This is the last remaining COMPLETE replica. It cannot be removed,"
                            + " otherwise it can happen that the file will be destroyed.");
            } else
                file.removeReplica(osd);
        } else
            System.err.println("File is not marked as read-only.");
    }

    // automatic
    public void removeReplicaAutomatically(String method) throws Exception {
        if (file.isReadOnly()) {
            ServiceUUID osd = null;
            if (method.equals(METHOD_RANDOM)) {
                Random random = new Random();
                Replica replica = xLoc.getReplicas().get(random.nextInt(xLoc.getNumReplicas()));
                osd = replica.getHeadOsd();
            }
            file.removeReplica(osd);
        } else
            System.err.println("File is not marked as read-only.");
    }

    public void setReadOnly(boolean mode) throws Exception {
        file.setReadOnly(mode);
    }

    public void isReadOnly() throws Exception {
        if (file.isReadOnly())
            System.out.println("File is marked as read-only.");
        else
            System.out.println("File is NOT marked as read-only.");
    }

    public void listReplicas() throws UnknownUUIDException {
        printListOfReplicas(xLoc.getReplicas());
    }

    private List<ServiceUUID> listSuitableOSDs() throws Exception {
        List<ServiceUUID> osds = file.getSuitableOSDsForAReplica();
        printListOfOSDs(osds);
        return osds;
    }

    private void printListOfReplicas(List<Replica> replicas) throws UnknownUUIDException {
        StringBuffer out = new StringBuffer();
        int replicaNumber = 1;
        for (Replica r : replicas) {
            // head line
            out.append("[" + replicaNumber + "] ");
            out.append("REPLICA " + (replicaNumber++) + ": " + r.getStripingPolicy().toString() + "\n");

            int osdNumber = 1;
            // OSDs of this replica
            for (ServiceUUID osd : r.getOSDs()) {
                out.append("\t OSD " + (osdNumber++) + ": " + osd.toString() + " ("
                        + osd.getAddress().toString() + ")\n");
            }
        }
        System.out.print(out.toString());
    }

    private void printListOfOSDs(List<ServiceUUID> osds) throws UnknownUUIDException {
        StringBuffer out = new StringBuffer();
        if (osds.size() != 0) {
            int number = 1;
            for (ServiceUUID osd : osds) {
                out.append("[" + number + "] ");
                out.append(osd.toString() + " (" + osd.getAddress().toString() + ")\n");
            }
        } else
            out.append("no suitable OSDs available\n");
        System.out.print(out.toString());
    }

    /**
     * 
     */
    private void shutdown() {
        try {
            if (client != null) {
                client.shutdown();
                client.waitForShutdown();
            }

            UUIDResolver.shutdown();

            if (timeSync != null) {
                timeSync.shutdown();
                timeSync.waitForShutdown();
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * @param args
     */
    public static void main(String[] args) {

        Map<String, CliOption> options = new HashMap<String, CliOption>();
        List<String> arguments = new ArrayList<String>(3);
        options.put(IS_READ_ONLY, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(SET_READ_ONLY, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(SET_WRITABLE, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(LIST_REPLICAS, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(LIST_SUITABLE_OSDS_FOR_REPLICA, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(ADD_REPLICA, new CliOption(CliOption.OPTIONTYPE.STRING));
        options.put(ADD_REPLICA_INTERACTIVE, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(ADD_AUTOMATIC_REPLICA, new CliOption(CliOption.OPTIONTYPE.STRING));
        options.put(REMOVE_REPLICA, new CliOption(CliOption.OPTIONTYPE.STRING));
        options.put(REMOVE_REPLICA_INTERACTIVE, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(REMOVE_AUTOMATIC_REPLICA, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(HELP, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(HELP_LONG, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(SSL_CREDS_FILE, new CliOption(CliOption.OPTIONTYPE.STRING));
        options.put(SSL_CREDS_PASSWORD, new CliOption(CliOption.OPTIONTYPE.STRING));
        options.put(SSL_TRUSTED_CA_FILE, new CliOption(CliOption.OPTIONTYPE.STRING));
        options.put(SSL_TRUSTED_CA_PASSWORD, new CliOption(CliOption.OPTIONTYPE.STRING));
        options.put("p", new CliOption(CliOption.OPTIONTYPE.STRING));
        options.put(REPLICATION_FLAG_FULL_REPLICA, new CliOption(CliOption.OPTIONTYPE.SWITCH));
        options.put(REPLICATION_FLAG_TRANSFER_STRATEGY, new CliOption(CliOption.OPTIONTYPE.STRING));

        try {
            CLIParser.parseCLI(args, options, arguments);
        } catch (Exception exc) {
            System.out.println(exc);
            usage();
            return;
        }

        CliOption h = options.get(HELP);
        if (h.switchValue) {
            usage();
            return;
        }

        
        h = options.get(HELP_LONG);
        if (h.switchValue) {
            usage();
            return;
        }
        
        if (arguments.size() < 1) {
            usage();
            return;
        }

        xtfs_repl system = null;

        try {
            // resolve the path
            final String filePath = utils.expandPath(arguments.get(0));
            final String url = utils.getxattr(filePath, "xtreemfs.url");

            if (url == null) {
                System.err.println("could not retrieve XtreemFS URL for file '" + filePath + "'");
                System.exit(1);
            }

            final int i0 = url.indexOf("://") + 2;
            final int i1 = url.indexOf(':', i0);
            final int i2 = url.indexOf('/', i1);
            final int i3 = url.indexOf('/', i2 + 1);

            final String dirURL = url.substring(i0 + 1, i1);
            final int dirPort = Integer.parseInt(url.substring(i1 + 1, i2));
            final String volume = url.substring(i2 + 1, i3 == -1 ? url.length() : i3);
            final String volPath = i3 == -1 ? "" : url.substring(i3);
            final InetSocketAddress dirAddress = new InetSocketAddress(dirURL, dirPort);

            // create SSL options (if set)
            SSLOptions sslOptions = null;
            if ((options.get(SSL_CREDS_FILE).stringValue != null)
                    && (options.get(SSL_CREDS_PASSWORD).stringValue != null)
                    && (options.get(SSL_TRUSTED_CA_FILE).stringValue != null)
                    && (options.get(SSL_TRUSTED_CA_PASSWORD).stringValue != null)) { // SSL set
                sslOptions = new SSLOptions(new FileInputStream(options.get(SSL_CREDS_FILE).stringValue),
                        options.get(SSL_CREDS_PASSWORD).stringValue, new FileInputStream(options
                                .get(SSL_TRUSTED_CA_FILE).stringValue),
                        options.get(SSL_TRUSTED_CA_PASSWORD).stringValue);
            }

            system = new xtfs_repl(filePath, dirAddress, volume, volPath, sslOptions);
            system.initialize();

            for (Entry<String, CliOption> e : options.entrySet()) {
                if (e.getKey().equals(ADD_REPLICA) && e.getValue().stringValue != null) {

                    // parse replication flags
                    int replicationFlags = DEFAULT_REPLICATION_FLAGS;
                    CliOption option = options.get(REPLICATION_FLAG_FULL_REPLICA);
                    if (option != null && option.switchValue)
                        replicationFlags = ReplicationFlags.setFullReplica(replicationFlags);

                    option = options.get(REPLICATION_FLAG_TRANSFER_STRATEGY);
                    if (option != null && option.stringValue != null) {
                        String method = option.stringValue.replace('\"', ' ').trim();

                        if (method.equals(TRANSFER_STRATEGY_RANDOM))
                            replicationFlags = ReplicationFlags.setRandomStrategy(replicationFlags);
                        else if (method.equals(TRANSFER_STRATEGY_SEQUENTIAL))
                            replicationFlags = ReplicationFlags.setSequentialStrategy(replicationFlags);
                    }

                    StringTokenizer st = new StringTokenizer(e.getValue().stringValue, "\", \t");
                    List<ServiceUUID> osds = new ArrayList<ServiceUUID>(st.countTokens());

                    if (st.countTokens() > 0) {
                        while (st.hasMoreTokens())
                            osds.add(new ServiceUUID(st.nextToken()));
                        system.addReplica(osds, replicationFlags);
                    } else
                        usage();

                } else if (e.getKey().equals(ADD_REPLICA_INTERACTIVE) && e.getValue().switchValue) {

                    // interactive mode
                    system.addReplica();

                } else if (e.getKey().equals(REMOVE_REPLICA) && e.getValue().stringValue != null) {

                    String headOSD = e.getValue().stringValue.replace('\"', ' ').trim();

                    if (headOSD.length() > 0) {
                        ServiceUUID osd = new ServiceUUID(headOSD);
                        system.removeReplica(osd);
                    } else
                        usage();

                } else if (e.getKey().equals(REMOVE_REPLICA_INTERACTIVE) && e.getValue().stringValue != null) {

                    // interactive mode
                    system.removeReplica();

                } else if (e.getKey().equals(ADD_AUTOMATIC_REPLICA) && e.getValue().stringValue != null) {

                    // parse replication flags
                    int replicationFlags = DEFAULT_REPLICATION_FLAGS;
                    CliOption option = options.get(REPLICATION_FLAG_FULL_REPLICA);
                    if (option != null && option.switchValue)
                        replicationFlags = ReplicationFlags.setFullReplica(replicationFlags);

                    option = options.get(REPLICATION_FLAG_TRANSFER_STRATEGY);
                    if (option != null && option.stringValue != null) {
                        String method = option.stringValue.replace('\"', ' ').trim();

                        if (method.equals(TRANSFER_STRATEGY_RANDOM))
                            replicationFlags = ReplicationFlags.setRandomStrategy(replicationFlags);
                        else if (method.equals(TRANSFER_STRATEGY_SEQUENTIAL))
                            replicationFlags = ReplicationFlags.setSequentialStrategy(replicationFlags);
                    }

                    String method = e.getValue().stringValue.replace('\"', ' ').trim();

                    if (method.equals(METHOD_RANDOM) || method.equals(METHOD_DNS)) {
                        system.initialize();
                        system.addReplicaAutomatically(method, replicationFlags);
                    } else
                        System.err.println("unknown method - must be '" + METHOD_RANDOM + "' or '"
                                + METHOD_DNS + "'");

                } else if (e.getKey().equals(REMOVE_AUTOMATIC_REPLICA) && e.getValue().switchValue) {
                    system.removeReplicaAutomatically(METHOD_RANDOM);
                } else if (e.getKey().equals(SET_READ_ONLY) && e.getValue().switchValue) {
                    system.setReadOnly(true);
                } else if (e.getKey().equals(SET_WRITABLE) && e.getValue().switchValue) {
                    system.setReadOnly(false);
                } else if (e.getKey().equals(IS_READ_ONLY) && e.getValue().switchValue) {
                    system.isReadOnly();
                } else if (e.getKey().equals(LIST_REPLICAS) && e.getValue().switchValue) {
                    system.listReplicas();
                } else if (e.getKey().equals(LIST_SUITABLE_OSDS_FOR_REPLICA) && e.getValue().switchValue) {
                    system.listSuitableOSDs();
                }
            }
        } catch (Exception e) {
            System.err.println("an error has occurred");
            e.printStackTrace();
        } finally {
            if (system != null)
                system.shutdown();
        }
    }

    public static void usage() {
        StringBuffer out = new StringBuffer();
        out.append("Usage: " + xtfs_repl.class.getSimpleName());
        out.append(" [options] <path>\n");
        out.append("options:\n");
        out.append("\t-" + ADD_REPLICA
            + " <UUID_of_OSD1 UUID_of_OSD2 ...>: Adds a replica with the given OSDs. "
            + "The number of OSDs must be the same as in the file's striping policy. "
            + "Use space as seperator.\n");
        out.append("\t-" + HELP + "/-" + HELP_LONG + ": show usage info\n");
        out.append("\t-" + LIST_REPLICAS + ": lists all replicas of this file\n");
        out.append("\t-" + LIST_SUITABLE_OSDS_FOR_REPLICA
            + ": lists all suitable OSDs for this file, which can be used for a new replica\n");
        out.append("\t-" + REMOVE_REPLICA + " <UUID_of_head-OSD>"
            + ": removes the replica with the given head OSD\n");
        out.append("\t-" + ADD_AUTOMATIC_REPLICA + " {" + METHOD_RANDOM + "|" + METHOD_DNS
            + "}: adds a replica and automatically selects OSDs according to the chosen method\n");
        out.append("\t-" + REMOVE_AUTOMATIC_REPLICA + ": removes a randomly selected replica\n");
        out.append("\t-" + ADD_REPLICA_INTERACTIVE + ": an interactive mode for adding a replica\n");
        out.append("\t-" + REMOVE_REPLICA_INTERACTIVE + ": an interactive mode for removing a replica\n");
        out.append("\t-" + IS_READ_ONLY + ": checks if the file is already marked as read-only\n");
        out.append("\t-" + REPLICATION_FLAG_FULL_REPLICA
                        + ": if set, the replica will be a complete copy of the file; otherwise only requested data will be replicated\n");
        out.append("\t-" + SET_READ_ONLY + ": marks the file as read-only\n");
        out.append("\t-" + SET_WRITABLE + ": marks the file as writable (normal file)\n");
        out.append("\t-" + REPLICATION_FLAG_TRANSFER_STRATEGY + " {" + TRANSFER_STRATEGY_RANDOM + "|"
                + TRANSFER_STRATEGY_SEQUENTIAL + "}: the replica to add will use the chosen strategy\n");
        out.append("\n");
        out.append("\tTo use SSL it is necessary to also specify credentials:\n");
        out.append("\t-" + SSL_CREDS_FILE + " <creds_file>: a PKCS#12 file containing user credentials\n");
        out.append("\t-" + SSL_CREDS_PASSWORD
                + " <creds_passphrase>: a pass phrase to decrypt the the user credentials file\n");
        out.append("\t-" + SSL_TRUSTED_CA_FILE
                + " <trusted_CAs>: a PKCS#12 file containing a set of certificates from trusted CAs\n");
        out.append("\t-" + SSL_TRUSTED_CA_PASSWORD
                + " <trusted_passphrase>: a pass phrase to decrypt the trusted CAs file");

        System.out.println(out.toString());
    }
}
