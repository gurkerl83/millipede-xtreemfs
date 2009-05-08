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
 * AUTHORS: Björn Kolbeck (ZIB)
 */


package org.xtreemfs.sandbox.tests;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.xtreemfs.common.TimeSync;
import org.xtreemfs.common.clients.io.RandomAccessFile;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.util.ONCRPCServiceURL;
import org.xtreemfs.common.uuids.UUIDResolver;
import org.xtreemfs.dir.client.DIRClient;
import org.xtreemfs.foundation.SSLOptions;
import org.xtreemfs.foundation.oncrpc.client.RPCNIOSocketClient;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.interfaces.AccessControlPolicyType;
import org.xtreemfs.interfaces.OSDSelectionPolicyType;
import org.xtreemfs.interfaces.StripingPolicy;
import org.xtreemfs.interfaces.StripingPolicyType;
import org.xtreemfs.interfaces.UserCredentials;
import org.xtreemfs.interfaces.utils.ONCRPCException;
import org.xtreemfs.mrc.client.MRCClient;
import org.xtreemfs.utils.CLIParser;
import org.xtreemfs.utils.CLIParser.CliOption;

/**
 *
 * @author bjko
 */
public class TortureXtreemFS {

    public static void main(String[] args) {
        try {

            Map<String, CliOption> options = new HashMap<String, CliOption>();
            List<String> arguments = new ArrayList<String>(2);
            options.put("v", new CliOption(CliOption.OPTIONTYPE.STRING));
            options.put("p", new CliOption(CliOption.OPTIONTYPE.STRING));
            options.put("c", new CliOption(CliOption.OPTIONTYPE.STRING));
            options.put("cpass", new CliOption(CliOption.OPTIONTYPE.STRING));
            options.put("t", new CliOption(CliOption.OPTIONTYPE.STRING));
            options.put("tpass", new CliOption(CliOption.OPTIONTYPE.STRING));
            options.put("mkvol", new CliOption(CliOption.OPTIONTYPE.SWITCH));

            CLIParser.parseCLI(args, options, arguments);

            Logging.start(Logging.LEVEL_WARN);
            TimeSync.initialize(null, 10000, 50);

            if (arguments.size() != 2) {
                usage();
                return;
            }

            final ONCRPCServiceURL mrcURL = new ONCRPCServiceURL(arguments.get(1));

            final String path = (options.get("p").stringValue != null) ?
                                options.get("p").stringValue : "/torture.data";
            final String volname = (options.get("v").stringValue != null) ?
                                    options.get("v").stringValue : "test";

            final ONCRPCServiceURL    dirURL = new ONCRPCServiceURL(arguments.get(0));

            boolean useSSL = false;

            final SSLOptions sslOptions;

            if (dirURL.getProtocol().equalsIgnoreCase("oncrpcs")) {
                //require credentials!
                String serviceCredsFile = null;
                String serviceCredsPass = null;
                String trustedCAsFile = null;
                String trustedCAsPass = null;

                useSSL = true;
                serviceCredsFile = options.get("c").stringValue;
                serviceCredsPass = options.get("cpass").stringValue;
                trustedCAsFile = options.get("t").stringValue;
                trustedCAsPass = options.get("tpass").stringValue;

                sslOptions = new SSLOptions(new FileInputStream(serviceCredsFile), serviceCredsPass,
                        SSLOptions.PKCS12_CONTAINER, new FileInputStream(trustedCAsFile), trustedCAsPass,
                        SSLOptions.JKS_CONTAINER, false);
                System.out.println("USING SSL");
            } else {
                sslOptions = null;
            }


            RPCNIOSocketClient rpcClient = new RPCNIOSocketClient(sslOptions, 10000, 5*60*1000);
            rpcClient.start();
            rpcClient.waitForStartup();

            DIRClient dir = new DIRClient(rpcClient,new InetSocketAddress(dirURL.getHost(),dirURL.getPort()));
            UUIDResolver.start(dir, 10000, 9999999);
            System.out.println("file size from 64k to 512MB with record length from 4k to 1M");

            final int MIN_FS = 64*1024;
            final int MAX_FS = 512*1024*1024;

            final int MIN_REC = 4*1024;
            final int MAX_REC = 1024*1024;


            InetSocketAddress mrcAddr = new InetSocketAddress(mrcURL.getHost(), mrcURL.getPort());

            List<String> grs = new ArrayList(1);
            grs.add("torture");
            final UserCredentials uc = MRCClient.getCredentials("torture", grs);


            if (options.get("mkvol").switchValue) {
                MRCClient mrcClient = new MRCClient(rpcClient, mrcAddr);
                StripingPolicy sp = new StripingPolicy(StripingPolicyType.STRIPING_POLICY_RAID0, 128, 1);
                RPCResponse r = mrcClient.mkvol(mrcAddr, uc, volname,
                    OSDSelectionPolicyType.OSD_SELECTION_POLICY_SIMPLE.intValue(), sp,
                    AccessControlPolicyType.ACCESS_CONTROL_POLICY_POSIX.intValue(), 0775);
                r.get();
                r.freeBuffers();
            }

            RandomAccessFile tmp = new RandomAccessFile("rw",mrcAddr,volname+path+".tmp",rpcClient,"root",grs);
            System.out.println("Default striping policy is: "+tmp.getStripingPolicyAsString());

            for (int fsize = MIN_FS; fsize <= MAX_FS; fsize = fsize * 2) {
                for (int recsize = MIN_REC; recsize <= MAX_REC; recsize = recsize *2) {
                    if (testSequential(fsize, recsize, mrcAddr, volname, path, rpcClient, grs)) {
                        continue;
                    }
                }
            }
            
            System.out.println("\nrandom test\n");

            for (int fsize = MIN_FS; fsize <= MAX_FS; fsize = fsize * 2) {
                for (int recsize = MIN_REC; recsize <= MAX_REC; recsize = recsize *2) {
                    if (testRandom(fsize, recsize, mrcAddr, volname, path, rpcClient, grs)) {
                        continue;
                    }
                }
            }

            System.out.println("finished");
            rpcClient.shutdown();
            UUIDResolver.shutdown();
        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(1);
        }

    }

    private static boolean testSequential(int fsize, int recsize, InetSocketAddress mrcAddr, final String volname, final String path, RPCNIOSocketClient rpcClient, List<String> grs) throws ONCRPCException, InterruptedException, Exception, IOException {
        final int numRecs = fsize / recsize;
        if (numRecs == 0) {
            return true;
        }
        byte[] sendBuffer = new byte[recsize];
        for (int i = 0; i < recsize; i++) {
            sendBuffer[i] = (byte) ((i%26) + 65);
        }
        long tStart = System.currentTimeMillis();
        RandomAccessFile raf = new RandomAccessFile("rw", mrcAddr, volname + path, rpcClient, "root", grs);
        long tOpen = System.currentTimeMillis();
        long bytesWritten = 0;
        //do writes
        long tWrite = 0;
        for (int rec = 0; rec < numRecs; rec++) {
            long tmpStart = System.currentTimeMillis();
            bytesWritten += raf.write(sendBuffer, 0, recsize);
            tWrite += System.currentTimeMillis()-tmpStart;
        }
        assert (bytesWritten == numRecs * recsize);
        raf.flush();
        raf.seek(0);
        final long tFlush = System.currentTimeMillis();
        long tRead = 0;
        //do writes
        byte[] readBuffer = new byte[recsize];
        for (int rec = 0; rec < numRecs; rec++) {
            long tmpStart = System.currentTimeMillis();
            final int bytesRead = raf.read(readBuffer, 0, recsize);
            tRead += System.currentTimeMillis()-tmpStart;
            if (bytesRead != recsize) {
                System.out.println("PREMATURE END-OF-FILE AT " + rec * recsize);
                System.out.println("expected " + recsize + " bytes");
                System.out.println("got " + bytesRead + " bytes");
                System.exit(1);
            }
            for (int i = 0; i < recsize; i++) {
                if (readBuffer[i] != (byte) ((i%26) + 65)) {
                    System.out.println("INVALID CONTENT AT " + (rec * recsize + i));
                    System.out.println("expected:  " + (byte) ((i%26) + 65));
                    System.out.println("got     : " + readBuffer[i]);
                    System.exit(1);
                }
            }
        }
        
        raf.delete();
        final long tDelete = System.currentTimeMillis();
        double writeRate = ((double) fsize) / 1024.0 / (((double) (tWrite)) / 1000.0);
        double readRate = ((double) fsize) / 1024.0 / (((double) (tRead)) / 1000.0);
        System.out.format("fs: %8d   bs: %8d    write: %6d ms   %6.0f kb/s    read: %6d ms   %6.0f kb/s\n", fsize / 1024, recsize, tWrite, writeRate, tRead, readRate);
        return false;
    }

    private static boolean testRandom(int fsize, int recsize, InetSocketAddress mrcAddr, final String volname, final String path, RPCNIOSocketClient rpcClient, List<String> grs) throws ONCRPCException, InterruptedException, Exception, IOException {
        final int numRecs = fsize / recsize;
        int[] skips = new int[numRecs];
        if (numRecs == 0) {
            return true;
        }
        byte[] sendBuffer = new byte[recsize];
        for (int i = 0; i < recsize; i++) {
            sendBuffer[i] = (byte) ((i%26) + 65);
        }
        long tStart = System.currentTimeMillis();
        RandomAccessFile raf = new RandomAccessFile("rw", mrcAddr, volname + path, rpcClient, "root", grs);
        long tOpen = System.currentTimeMillis();
        long bytesWritten = 0;
        long tWrite = 0;
        //do writes
        for (int rec = 0; rec < numRecs; rec++) {
            skips[rec] = (int) (Math.random()*((double)recsize));
            raf.seek(raf.getFilePointer()+skips[rec]);
            long tmpStart = System.currentTimeMillis();
            bytesWritten += raf.write(sendBuffer, 0, recsize);
            tWrite += System.currentTimeMillis()-tmpStart;
        }
        if (bytesWritten != numRecs * recsize) {
            System.out.println("not all data was written!");
            System.exit(1);
        }
        raf.flush();
        raf.seek(0);
        final long tFlush = System.currentTimeMillis();
        long tRead = 0;
        //do writes
        byte[] readBuffer = new byte[recsize];
        for (int rec = 0; rec < numRecs; rec++) {
            raf.seek(raf.getFilePointer()+skips[rec]);
            long tmpStart = System.currentTimeMillis();
            final int bytesRead = raf.read(readBuffer, 0, recsize);
            tRead += System.currentTimeMillis()-tmpStart;
            if (bytesRead != recsize) {
                System.out.println("PREMATURE END-OF-FILE AT " + rec * recsize);
                System.out.println("expected " + recsize + " bytes");
                System.out.println("got " + bytesRead + " bytes");
                System.exit(1);
            }
            for (int i = 0; i < recsize; i++) {
                if (readBuffer[i] != (byte) ((i%26) + 65)) {
                    System.out.println("INVALID CONTENT AT " + (rec * recsize + i));
                    System.out.println("expected:  " + (byte) ((i%26) + 65));
                    System.out.println("got     : " + readBuffer[i]);
                    System.exit(1);
                }
            }
        }
        raf.delete();
        final long tDelete = System.currentTimeMillis();
        double writeRate = ((double) fsize) / 1024.0 / (((double) (tWrite)) / 1000.0);
        double readRate = ((double) fsize) / 1024.0 / (((double) (tRead)) / 1000.0);
        System.out.format("fs: %8d   bs: %8d    write: %6d ms   %6.0f kb/s    read: %6d ms   %6.0f kb/s\n", fsize / 1024, recsize, tWrite, writeRate, tRead, readRate);
        return false;
    }


    private static void usage() {
        System.out.println("usage: torture [options] <dir_url> <mrc_url>");
        System.out.println("  -v <volume name>  name of the volume on the mrc (default: test)");
        System.out.println("  -p <path>   filename to use for measurements (default: /torture.dat)");

        System.out
                .println("            In case of a secured URL ('https://...'), it is necessary to also specify SSL credentials:");
        System.out
                .println("              -c  <creds_file>         a PKCS#12 file containing user credentials");
        System.out
                .println("              -cpass <creds_passphrase>   a pass phrase to decrypt the the user credentials file");
        System.out
                .println("              -t  <trusted_CAs>        a PKCS#12 file containing a set of certificates from trusted CAs");
        System.out
                .println("              -tpass <trusted_passphrase> a pass phrase to decrypt the trusted CAs file");
        System.out.println("  -h        show usage info");
    }

}