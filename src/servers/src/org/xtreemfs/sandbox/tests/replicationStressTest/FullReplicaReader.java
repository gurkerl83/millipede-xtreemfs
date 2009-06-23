/*  Copyright (c) 2009 Barcelona Supercomputing Center - Centro Nacional
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
package org.xtreemfs.sandbox.tests.replicationStressTest;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.net.InetSocketAddress;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.CopyOnWriteArrayList;

import org.xtreemfs.common.clients.io.RandomAccessFile;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.logging.Logging.Category;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.common.xloc.Replica;
import org.xtreemfs.common.xloc.StripingPolicyImpl;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.interfaces.ObjectList;
import org.xtreemfs.interfaces.UserCredentials;
import org.xtreemfs.osd.client.OSDClient;

/**
 * a reader for reading replicas which are marked as full replicas (preferred)
 * <br>09.06.2009
 */
public class FullReplicaReader extends ReplicationStressTestReader {
    
    private OSDClient osdClient;
    private static HashSet<Replica> completedReplicas = new HashSet<Replica>();
    
    /**
     * @throws Exception
     * 
     */
    public FullReplicaReader(int threadNo, InetSocketAddress mrcAddress,
            CopyOnWriteArrayList<FileInfo> fileList, Random random, UserCredentials userCredentials)
            throws Exception {
        super(threadNo, mrcAddress, fileList, random, userCredentials);
        this.osdClient = new OSDClient(client);
    }

    /**
     * read/replicate files
     * 
     * @return throughput
     */
    public double readFile(FileInfo file) throws Exception {
        double factor = random.nextDouble();
        factor = (factor - 0.6 > 0) ? (factor - 0.6) : factor; // rather smaller partsizes
        int partSize = (int) Math.round(ReplicationStressTest.PART_SIZE * factor);
        long timeRequiredForReading = 0;

        java.io.RandomAccessFile originalFile = null;
        try {
            originalFile = new java.io.RandomAccessFile(ReplicationStressTest.tmpDir
                    + ReplicationStressTest.tmpFilename, "r");
            RandomAccessFile raf = new RandomAccessFile("r", mrcAddress, ReplicationStressTest.VOLUME_NAME
                    + ReplicationStressTest.DIR_PATH + file.filename, client, userCredentials);

            long filesize = raf.length();
            StripingPolicyImpl sp = raf.getCurrentlyUsedReplica().getStripingPolicy();
            long lastObject = sp.getObjectNoForOffset(filesize - 1);

            // prepare spot-sample-ranges for reading file
            final int SPOT_SAMPLE_COUNT = 10;
            List<Long> startOffsets = new LinkedList<Long>();
            for (int i = 0; i < SPOT_SAMPLE_COUNT; i++) {
                long startOffset;
                do { // get a part in range of filedata
                    startOffset = random.nextLong() % filesize;
                } while (startOffset + partSize > filesize || startOffset < 0); // try again
                startOffsets.add(startOffset);
            }
            
            // sleep a short time, so the background replication could begin
            Thread.sleep(SLEEP_TIME);

            // read file
            for (Long startOffset : startOffsets) {
                byte[] result = new byte[partSize];
                byte[] expectedResult = new byte[partSize];

                // read
                try {
                    // monitoring: time (latency/throughput)
                    long timeBefore = System.currentTimeMillis();
                    file.readFromXtreemFS(result, raf, startOffset);
                    timeRequiredForReading += System.currentTimeMillis() - timeBefore;
                } catch (Exception e) {
                    // TODO: catch exception, if request is rejected because of change of XLocations version
                    ReplicationStressTest.containedErrors = true;
                    file.readFromDisk(expectedResult, originalFile, startOffset, filesize);

                    log(e.getCause().toString(), file, startOffset, startOffset + partSize, filesize, result,
                            expectedResult);
                    break;
                }

                // ASSERT the byte-data
                file.readFromDisk(expectedResult, originalFile, startOffset, filesize);
                if (!Arrays.equals(result, expectedResult)) {
                    ReplicationStressTest.containedErrors = true;
                    log("Read wrong data.", file, startOffset, startOffset + partSize, filesize, result,
                            expectedResult);
                }
            }
            
            // wait some time so the background replication could work
            Thread.sleep(SLEEP_TIME);
            
            // check if replica is complete => background replication should work
            // get object lists from all OSDs (by RAF)
            long objects = 0; // sum of all saved objects from all OSDs of this replica
            for (ServiceUUID osd : raf.getCurrentlyUsedReplica().getOSDs()) {
                RPCResponse<ObjectList> response = osdClient.internal_getObjectList(osd.getAddress(), raf
                        .getFileId(), raf.getCredentials());
                ObjectList list = response.get();
                response.freeBuffers();

                long[] objectList = deserializeObjectList(list);
                objects += objectList.length;
            }

            Logging.logMessage(Logging.LEVEL_DEBUG, Category.test, this,
                    "Replica %s (Head-OSD) of file %s has replicated %d objects (= %d KB of %d KB).", raf
                            .getCurrentlyUsedReplica().getHeadOsd().toString(), file.filename, objects,
                    (objects * sp.getStripeSizeForObject(1)) / 1024,
                    filesize / 1024);

            if (objects == lastObject + 1 && !raf.getCurrentlyUsedReplica().isFull()) { // replication is completed; all objects are replicated
                if (!completedReplicas.contains(raf.getCurrentlyUsedReplica())) {
                    completedReplicas.add(raf.getCurrentlyUsedReplica());
                    Logging.logMessage(Logging.LEVEL_DEBUG, Category.test, this,
                            "Replication of replica %s (Head-OSD) for file %s (all objects) is complete.",
                            raf.getCurrentlyUsedReplica().getHeadOsd().toString(), file.filename);
                }
            } else
                completedReplicas.remove(raf.getCurrentlyUsedReplica());

            return ((SPOT_SAMPLE_COUNT * partSize / 1024.0) / (timeRequiredForReading / 1000.0)); // KB/s
        } finally {
            if (originalFile != null)
                originalFile.close();
        }
    }

    private long[] deserializeObjectList(ObjectList objectList) throws IOException, ClassNotFoundException {
        long[] list = null;
        ByteArrayInputStream bis = new ByteArrayInputStream(objectList.getObject_list().array());
        ObjectInputStream ois = new ObjectInputStream(bis);
        Object o = ois.readObject();
        ois.close();
        bis.close();

        // TODO: support more types
        list = (long[]) o;
        return list;
    }
}