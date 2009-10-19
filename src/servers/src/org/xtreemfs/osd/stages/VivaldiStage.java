/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.xtreemfs.osd.stages;

import java.net.InetSocketAddress;
import java.util.concurrent.TimeUnit;

import org.xtreemfs.common.TimeSync;
import org.xtreemfs.common.buffer.BufferPool;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.util.OutputUtils;
import org.xtreemfs.dir.client.DIRClient;
import org.xtreemfs.foundation.oncrpc.utils.ONCRPCBufferWriter;
import org.xtreemfs.foundation.oncrpc.utils.XDRUnmarshaller;
import org.xtreemfs.interfaces.OSDInterface.xtreemfs_pingResponse;
import org.xtreemfs.interfaces.VivaldiCoordinates;
import org.xtreemfs.osd.OSDRequestDispatcher;
import org.xtreemfs.osd.striping.UDPMessage;

import org.xtreemfs.interfaces.OSDInterface.xtreemfs_pingRequest;

/**
 *
 * @author bjko
 */
public class VivaldiStage extends Stage {

    private static final int STAGEOP_COORD_XCHG_REQUEST = 1;

    private static final int TIMER_INTERVAL_IN_MS = 1000 * 60;

    private long lastTimerRun;

    private int  nextTimerRunInMS;
    
    private final OSDRequestDispatcher master;

    private final DIRClient            dirClient;
    
    public VivaldiStage(OSDRequestDispatcher master) {
        super("VivaldiSt");
        this.master = master;
        this.dirClient = master.getDIRClient();
    }

    @Override
    protected void processMethod(StageRequest method) {
        if (method.getStageMethod() == STAGEOP_COORD_XCHG_REQUEST) {

            UDPMessage msg = (UDPMessage)method.getArgs()[0];
            /*
            final ReusableBuffer data = msg.getPayload();
            //skip request type byte
            //data.position(1);
            VivaldiCoordinates vc = new VivaldiCoordinates();
            vc.unmarshal(new XDRUnmarshaller(data));
            
            BufferPool.free(data);
            */
            VivaldiCoordinates vc = ((xtreemfs_pingRequest)msg.getRequestData()).getCoordinates();
            System.out.println( String.format("Receiving (%.3f,%.3f)",vc.getX_coordinate(),vc.getY_coordinate()) );
            
            //do something...

            if (msg.isRequest()) {
                //send response if it was a request
                //FIXME: replace vc with your coordinates
                vc.setX_coordinate(6.66);
                vc.setY_coordinate(6.66);
                System.out.println( String.format("Sending (%.3f,%.3f)",vc.getX_coordinate(),vc.getY_coordinate()) );
                
                sendVivaldiCoordinates(msg, vc);
            }

        } else {
            throw new RuntimeException("programmatic error, unknown stage operation");
        }
    }

    private void executeTimer() {
        //FIXME: timer stuff here
        master.updateVivaldiCoordinates(new VivaldiCoordinates());
    }

    //------------------

    public static String coordinatesToString(VivaldiCoordinates vc) {
        StringBuffer sb = new StringBuffer(3*Long.SIZE/8);
        OutputUtils.writeHexLong(sb, Double.doubleToRawLongBits(vc.getX_coordinate()));
        OutputUtils.writeHexLong(sb, Double.doubleToRawLongBits(vc.getY_coordinate()));
        OutputUtils.writeHexLong(sb, Double.doubleToRawLongBits(vc.getLocal_error()));
        return sb.toString();
    }

    public static VivaldiCoordinates stringToCoordinates(String coordinates) {
        VivaldiCoordinates vc = new VivaldiCoordinates();
        vc.setX_coordinate(OutputUtils.readHexLong(coordinates, 0));
        vc.setY_coordinate(OutputUtils.readHexLong(coordinates, 8));
        vc.setLocal_error(OutputUtils.readHexLong(coordinates, 16));
        return vc;
    }

    public void receiveVivaldiMessage(UDPMessage msg) {
        enqueueOperation(STAGEOP_COORD_XCHG_REQUEST, new Object[]{msg}, null, null);
    }

    private void sendVivaldiCoordinates(UDPMessage request, VivaldiCoordinates myCoordinates) {
        final int payloadSize = myCoordinates.getXDRSize();
        //add one byte for UDP message type identifier
        xtreemfs_pingResponse resp = new xtreemfs_pingResponse(myCoordinates);
        UDPMessage msg = request.createResponse(resp);
        master.getUdpComStage().send(msg);
    }

    @Override
    public void run() {

        notifyStarted();

        // interval to check the OFT

        nextTimerRunInMS = TIMER_INTERVAL_IN_MS;
        lastTimerRun = 0;

        while (!quit) {
            try {
                final StageRequest op = q.poll(nextTimerRunInMS, TimeUnit.MILLISECONDS);

                nextTimerRunInMS = checkTimer();

                if (op != null) {
                    processMethod(op);
                }

            } catch (InterruptedException ex) {
                break;
            } catch (Exception ex) {
                notifyCrashed(ex);
                break;
            }
        }

        notifyStopped();
    }

    private int checkTimer() {
        final long now = TimeSync.getLocalSystemTime();
        
        if (now >= lastTimerRun+TIMER_INTERVAL_IN_MS) {
            lastTimerRun = now;
            executeTimer();
            return TIMER_INTERVAL_IN_MS;
        } else {
            final int nextCheck = (int)(now - lastTimerRun - TIMER_INTERVAL_IN_MS);
            return nextCheck;
        }
    }



}