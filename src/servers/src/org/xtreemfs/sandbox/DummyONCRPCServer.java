/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.xtreemfs.sandbox;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Iterator;
import org.xtreemfs.common.buffer.BufferPool;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.foundation.oncrpc.server.ONCRPCRequest;
import org.xtreemfs.foundation.oncrpc.server.RPCNIOSocketServer;
import org.xtreemfs.foundation.oncrpc.server.RPCServerRequestListener;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.foundation.oncrpc.utils.XDRUnmarshaller;
import org.xtreemfs.interfaces.AddressMapping;
import org.xtreemfs.interfaces.DIRInterface.ProtocolException;
import org.xtreemfs.interfaces.DIRInterface.xtreemfs_address_mappings_getRequest;
import org.xtreemfs.interfaces.DIRInterface.xtreemfs_address_mappings_getResponse;
import org.xtreemfs.interfaces.utils.ONCRPCRecordFragmentHeader;
import org.xtreemfs.interfaces.utils.ONCRPCRequestHeader;
import org.xtreemfs.interfaces.utils.ONCRPCResponseHeader;

/**
 *
 * @author bjko
 */
public class DummyONCRPCServer {

    public static void main(String[] args) {
        try {
            Logging.start(Logging.LEVEL_DEBUG);

            if ((args.length == 0) || (args[0].equals("s"))) {
                RPCServerRequestListener listener = new RPCServerRequestListener() {

                    @Override
                    public void receiveRecord(ONCRPCRequest rq) {
                        try {
                            System.out.println("request received");
                            ReusableBuffer buf = rq.getRequestFragment();

                            xtreemfs_address_mappings_getRequest rpcRequest = new xtreemfs_address_mappings_getRequest();
                            rpcRequest.unmarshal(new XDRUnmarshaller(buf));

                            xtreemfs_address_mappings_getResponse rpcResponse = new xtreemfs_address_mappings_getResponse();

                            if (rpcRequest.getUuid().equalsIgnoreCase("Yagga")) {
                                rpcResponse.getAddress_mappings().add(new AddressMapping("Yagga", 1, "rpc", "localhost", 12345, "*", 3600,
                                        "rpc://localhost:12345"));
                                System.out.println("response size is " + rpcResponse.getXDRSize());
                                rq.sendResponse(rpcResponse);
                            } else {
                                rq.sendGarbageArgs(null,new ProtocolException());
                            }
                        } catch (Exception ex) {
                            ex.printStackTrace();
                            System.exit(1);
                        }


                    }
                };
                RPCNIOSocketServer server = new RPCNIOSocketServer(12345, null, listener, null);
                server.start();
                server.waitForStartup();
            } else {
                Socket sock = new Socket("localhost", 12345);
                OutputStream out = sock.getOutputStream();
                InputStream in = sock.getInputStream();

                ONCRPCRequestHeader rqHdr = new ONCRPCRequestHeader(1, 20000000 + 1, 2, 4);

                ONCRPCBufferWriter writer = new ONCRPCBufferWriter(ONCRPCBufferWriter.BUFF_SIZE);

                xtreemfs_address_mappings_getRequest rq = new xtreemfs_address_mappings_getRequest("Yagga");

                final int fragHdr = ONCRPCRecordFragmentHeader.getFragmentHeader(rqHdr.getXDRSize() + rq.getXDRSize(), true);
                System.out.println("fragment size is " + fragHdr + "/" + (rqHdr.getXDRSize() + rq.getXDRSize()));
                System.out.println("fragment size is " + (fragHdr ^ (1 << 31)));
                writer.writeInt32(null,fragHdr);
                rqHdr.marshal(writer);

                rq.marshal(writer);
                writer.flip();
                System.out.println(writer);
                Iterator<ReusableBuffer> bufs = writer.getBuffers().iterator();
                while (bufs.hasNext()) {
                    final ReusableBuffer buf = bufs.next();
                    while (buf.hasRemaining()) {
                        out.write(buf.get());
                    }
                }
                out.flush();
                System.out.println("request sent");
                System.out.println(writer);

                ONCRPCResponseHeader rhdr = new ONCRPCResponseHeader();
                ReusableBuffer buf = BufferPool.allocate(4);
                buf.position(0);
                for (int i = 0; i < 4; i++) {
                    final int dataByte = in.read();
                    buf.put((byte) dataByte);
                }
                buf.position(0);

                final int respFragHdr = buf.getInt();
                final int fragmentSize = ONCRPCRecordFragmentHeader.getFragmentLength(respFragHdr);
                final boolean isLastFrag = ONCRPCRecordFragmentHeader.isLastFragment(respFragHdr);

                System.out.println("fragmentSize " + fragmentSize);

                buf = BufferPool.allocate(fragmentSize);
                buf.position(0);
                for (int i = 0; i < fragmentSize; i++) {
                    final int dataByte = in.read();
                    buf.put((byte) dataByte);
                }
                buf.position(0);

                rhdr.unmarshal(new XDRUnmarshaller(buf));

                System.out.println("bytes left: " + buf.remaining());


                xtreemfs_address_mappings_getResponse resp = new xtreemfs_address_mappings_getResponse();
                resp.unmarshal(new XDRUnmarshaller(buf));

                System.out.println("everything ok!");

                sock.close();
            }
        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(1);
        }
    }
}
