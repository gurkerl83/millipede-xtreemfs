/*
 * Copyright (c) 2010-2011 by Bjoern Kolbeck, Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

package org.xtreemfs.test.osd.rwre;

import java.net.InetSocketAddress;
import java.util.LinkedList;
import java.util.List;
import junit.framework.TestCase;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.xtreemfs.common.Capability;
import org.xtreemfs.common.ReplicaUpdatePolicies;
import org.xtreemfs.common.uuids.UUIDResolver;
import org.xtreemfs.foundation.buffer.BufferPool;
import org.xtreemfs.foundation.buffer.ReusableBuffer;
import org.xtreemfs.foundation.logging.Logging;
import org.xtreemfs.foundation.logging.Logging.Category;
import org.xtreemfs.foundation.pbrpc.client.PBRPCException;
import org.xtreemfs.foundation.pbrpc.client.RPCAuthentication;
import org.xtreemfs.foundation.pbrpc.client.RPCResponse;
import org.xtreemfs.foundation.pbrpc.generatedinterfaces.RPC.ErrorType;
import org.xtreemfs.foundation.util.FSUtils;
import org.xtreemfs.osd.OSD;
import org.xtreemfs.osd.OSDConfig;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.FileCredentials;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.OSDWriteResponse;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.Replica;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.SnapConfig;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.XCap;
import org.xtreemfs.pbrpc.generatedinterfaces.GlobalTypes.XLocSet;
import org.xtreemfs.pbrpc.generatedinterfaces.OSD.ObjectData;
import org.xtreemfs.pbrpc.generatedinterfaces.OSDServiceClient;
import org.xtreemfs.test.SetupUtils;
import org.xtreemfs.test.TestEnvironment;

/**
 *
 * @author bjko
 */
public class RWQuorumReplicationTest extends TestCase {

    private OSD[] osds;
    private OSDConfig[] configs;
    private TestEnvironment testEnv;

    private final static int NUM_OSDS = 3;
    private static final String fileId = "ABCDEF:1";

    public RWQuorumReplicationTest() {
        super();
        Logging.start(Logging.LEVEL_DEBUG, Category.all);
    }


    @Before
    public void setUp() throws Exception {

        java.io.File testDir = new java.io.File(SetupUtils.TEST_DIR);

        FSUtils.delTree(testDir);
        testDir.mkdirs();

        // startup: DIR
        testEnv = new TestEnvironment(new TestEnvironment.Services[] { TestEnvironment.Services.DIR_SERVICE,
                TestEnvironment.Services.TIME_SYNC, TestEnvironment.Services.UUID_RESOLVER,
                TestEnvironment.Services.OSD_CLIENT, TestEnvironment.Services.MRC,
                TestEnvironment.Services.MRC_CLIENT});
        testEnv.start();

        osds = new OSD[NUM_OSDS];
        configs = SetupUtils.createMultipleOSDConfigs(NUM_OSDS);
        for (int i = 0; i < osds.length; i++) {
            osds[i] = new OSD(configs[i]);
        }

    }

    @After
    public void tearDown() {
        if (osds != null) {
            for (OSD osd : this.osds) {
                if (osd != null)
                    osd.shutdown();
            }
        }

        testEnv.shutdown();
    }

    // TODO add test methods here.
    // The methods must be annotated with annotation @Test. For example:
    //
    // @Test
    // public void hello() {}

    @Test
    public void testReplicatedWrite() throws Exception {
        Capability cap = new Capability(fileId, 0, 60, System.currentTimeMillis()+10000, "", 0, false, SnapConfig.SNAP_CONFIG_SNAPS_DISABLED, 0, configs[0].getCapabilitySecret());
        List<Replica> rlist = new LinkedList();
        for (OSDConfig osd : this.configs) {
            Replica r = Replica.newBuilder().setStripingPolicy(SetupUtils.getStripingPolicy(1, 128)).setReplicationFlags(0).addOsdUuids(osd.getUUID().toString()).build();
            rlist.add(r);
        }
        
        UUIDResolver.addTestMapping("yaggablurp", "testposd.xtreemfs.com", 32640, false);

        //add a non-existent OSD
        Replica replica = Replica.newBuilder().setStripingPolicy(SetupUtils.getStripingPolicy(1, 128)).setReplicationFlags(0).addOsdUuids("yaggablurp").build();
        rlist.add(replica);

        XLocSet locSet = XLocSet.newBuilder().setReadOnlyFileSize(0).setReplicaUpdatePolicy(ReplicaUpdatePolicies.REPL_UPDATE_PC_WQRQ).setVersion(1).addAllReplicas(rlist).build();
        // set the first replica as current replica
        FileCredentials fc = FileCredentials.newBuilder().setXcap(cap.getXCap()).setXlocs(locSet).build();

        final OSDServiceClient client = testEnv.getOSDClient();

        final InetSocketAddress osd1 = new InetSocketAddress("localhost",configs[0].getPort());

        final InetSocketAddress osd2 = new InetSocketAddress("localhost",configs[1].getPort());


        ObjectData objdata = ObjectData.newBuilder().setChecksum(0).setZeroPadding(0).setInvalidChecksumOnOsd(false).build();
        ReusableBuffer rb = BufferPool.allocate(1024);
        rb.put("YaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYaggaYagga".getBytes());
        rb.limit(rb.capacity());
        rb.position(0);

        RPCResponse<OSDWriteResponse> r = client.write(osd1, RPCAuthentication.authNone, RPCAuthentication.userService,
                    fc, fileId, 0, 0, 0, 0, objdata, rb);
        try {
            r.get();
        } catch (Exception ex) {
            ex.printStackTrace();
            throw ex;
        }
        System.out.println("got response");
        r.freeBuffers();


        ReusableBuffer data = BufferPool.allocate(1024*8);
        r = client.write(osd2, RPCAuthentication.authNone, RPCAuthentication.userService,
                    fc, fileId, 0, 0, 0, 0, objdata, data);
        try {
            r.get();
            fail("expected redirect");
        } catch (PBRPCException ex) {
            if (ex.getErrorType() != ErrorType.REDIRECT)
                fail("expected redirect");
            System.out.println("got response: "+ex);
        }
        r.freeBuffers();

        rb = BufferPool.allocate(1024);
        rb.put("MoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeepMoeep".getBytes());
        rb.limit(rb.capacity());
        rb.position(0);

        r = client.write(osd1, RPCAuthentication.authNone, RPCAuthentication.userService,
                    fc, fileId, 0, 0, 1024, 0, objdata, rb);
        r.get();
        System.out.println("got response");
        r.freeBuffers();

        //read from slave
        System.out.println("//// START READ ////");
        RPCResponse<ObjectData> r2 = client.read(osd2, RPCAuthentication.authNone, RPCAuthentication.userService,
                    fc, fileId, 0, -1, 0, 2048);
        try {
            r2.get();
            fail("expected redirect");
        } catch (PBRPCException ex) {
            if (ex.getErrorType() != ErrorType.REDIRECT)
                fail("expected redirect");
            System.out.println("got response: "+ex);
        }
        r2.freeBuffers();


        r2 = client.read(osd1, RPCAuthentication.authNone, RPCAuthentication.userService,
                    fc, fileId, 0, -1, 0, 2048);
        ObjectData od = r2.get();
        rb = r2.getData();
        assertEquals(rb.get(0),(byte)'Y');
        assertEquals(rb.get(1),(byte)'a');
        r2.freeBuffers();

        XCap newCap = fc.getXcap().toBuilder().setTruncateEpoch(1).build();
        fc = fc.toBuilder().setXcap(newCap).build();

        RPCResponse r3 = client.truncate(osd1, RPCAuthentication.authNone, RPCAuthentication.userService,
                    fc, fileId, 128*1024*2);
        r3.get();
        r3.freeBuffers();

    }



}