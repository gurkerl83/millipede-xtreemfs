/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.xtreemfs.utils;

import java.net.InetSocketAddress;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.foundation.oncrpc.client.ONCRPCClient;
import org.xtreemfs.foundation.oncrpc.client.RPCNIOSocketClient;
import org.xtreemfs.foundation.oncrpc.client.RPCResponse;
import org.xtreemfs.foundation.oncrpc.client.RPCResponseDecoder;
import org.xtreemfs.foundation.oncrpc.utils.XDRUnmarshaller;
import org.xtreemfs.interfaces.NettestInterface.NettestInterface;
import org.xtreemfs.interfaces.NettestInterface.nopRequest;
import org.xtreemfs.interfaces.NettestInterface.nopResponse;

/**
 *
 * @author bjko
 */
public class NettestClient extends ONCRPCClient {

    public NettestClient(RPCNIOSocketClient client, InetSocketAddress defaultServer) {
        super(client, defaultServer,0,NettestInterface.getVersion());
    }

    public RPCResponse xtreemfs_nettest_nop(InetSocketAddress server) {
        nopRequest rq = new nopRequest();

        RPCResponse r = sendRequest((server == null) ? this.getDefaultServerAddress() : server, rq.getTag(), rq, new RPCResponseDecoder() {

            @Override
            public Object getResult(ReusableBuffer data) {
                final nopResponse resp = new nopResponse();
                resp.unmarshal(new XDRUnmarshaller(data));
                return null;
            }
        });
        return r;
    }

}