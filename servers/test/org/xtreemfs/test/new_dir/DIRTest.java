/*  Copyright (c) 2009 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

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

package org.xtreemfs.test.new_dir;

import java.io.IOException;
import java.net.InetSocketAddress;
import junit.framework.TestCase;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.xtreemfs.common.TimeSync;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.foundation.oncrpc.client.RPCNIOSocketClient;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.interfaces.AddressMapping;
import org.xtreemfs.interfaces.AddressMappingSet;
import org.xtreemfs.interfaces.Constants;
import org.xtreemfs.interfaces.KeyValuePair;
import org.xtreemfs.interfaces.KeyValuePairSet;
import org.xtreemfs.interfaces.ServiceRegistry;
import org.xtreemfs.interfaces.ServiceRegistrySet;
import org.xtreemfs.interfaces.Exceptions.ConcurrentModificationException;
import org.xtreemfs.dir.DIRConfig;
import org.xtreemfs.dir.DIRRequestDispatcher;
import org.xtreemfs.dir.client.DIRClient;
import org.xtreemfs.test.SetupUtils;
import org.xtreemfs.test.TestEnvironment;

/**
 *
 * @author bjko
 */
public class DIRTest extends TestCase {

    DIRRequestDispatcher dir;

    DIRConfig            config;

    TestEnvironment      testEnv;

    public DIRTest() throws IOException {
        config = SetupUtils.createDIRConfig();
        Logging.start(Logging.LEVEL_DEBUG);
    }

    @Before
    public void setUp() throws Exception {

        dir = new DIRRequestDispatcher(config);
        dir.startup();
        dir.waitForStartup();

        testEnv = new TestEnvironment(new TestEnvironment.Services[]{TestEnvironment.Services.DIR_CLIENT,
                    TestEnvironment.Services.TIME_SYNC, TestEnvironment.Services.RPC_CLIENT
        });
        testEnv.start();

        
    }

    @After
    public void tearDown() throws Exception {
        dir.shutdown();

        dir.waitForShutdown();

        testEnv.shutdown();
        
    }

    //@Test
    public void testGlobalTime() throws Exception {

        RPCResponse<Long> r = testEnv.getDirClient().global_time_get(null);
        Long response = r.get();

    }

    @Test
    public void testAddressMapping() throws Exception {

        DIRClient client = testEnv.getDirClient();

        AddressMappingSet set = new AddressMappingSet();
        AddressMapping mapping = new AddressMapping("uuid1", 0, "oncrpc", "localhost", 12345, "*", 3600);
        set.add(mapping);

        RPCResponse<Long> r1 = client.address_mappings_set(null, set);
        r1.get();
        r1.freeBuffers();

        r1 = client.address_mappings_set(null, set);
        try {
            r1.get();
            fail();
        } catch (ConcurrentModificationException ex) {
            //expected exception because of version mismatch
        }

        RPCResponse<AddressMappingSet> r2 = client.address_mappings_get(null, "uuid1");
        AddressMappingSet response = r2.get();
        assertEquals(response.size(),1);
        assertEquals(response.get(0).getUuid(),"uuid1");
        assertEquals(response.get(0).getProtocol(),"oncrpc");
        assertEquals(response.get(0).getAddress(),"localhost");
        assertEquals(response.get(0).getVersion(),1);

        RPCResponse r3 = client.address_mappings_delete(null, "uuid1");
        r3.get();

    }

    @Test
    public void testRegistry() throws Exception {

        DIRClient client = testEnv.getDirClient();

        KeyValuePairSet kvset = new KeyValuePairSet();
        kvset.add(new KeyValuePair("bla", "yagga"));
        ServiceRegistry sr = new ServiceRegistry("uuid1", 0, Constants.SERVICE_TYPE_MRC, "mrc @ farnsworth", kvset);

        RPCResponse<Long> r1 = client.service_register(null, sr);
        r1.get();
        r1.freeBuffers();

        r1 = client.service_register(null, sr);
        try {
            r1.get();
            fail();
        } catch (ConcurrentModificationException ex) {
            //expected exception because of version mismatch
        }

        RPCResponse<ServiceRegistrySet> r2 = client.service_get_by_uuid(null, "uuid1");
        ServiceRegistrySet response = r2.get();
        

        RPCResponse r3 = client.service_deregister(null, "uuid1");
        r3.get();

    }

}