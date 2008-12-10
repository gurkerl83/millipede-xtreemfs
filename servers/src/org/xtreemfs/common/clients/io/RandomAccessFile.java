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
 * AUTHORS: Nele Andersen (ZIB), Björn Kolbeck (ZIB)
 */

package org.xtreemfs.common.clients.io;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.URL;
import java.util.List;
import java.util.Map;

import org.xtreemfs.common.Capability;
import org.xtreemfs.common.auth.NullAuthProvider;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.clients.HttpErrorException;
import org.xtreemfs.common.clients.RPCResponse;
import org.xtreemfs.common.clients.mrc.MRCClient;
import org.xtreemfs.common.clients.osd.OSDClient;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.striping.Location;
import org.xtreemfs.common.striping.Locations;
import org.xtreemfs.common.striping.StripingPolicy;
import org.xtreemfs.common.uuids.ServiceUUID;
import org.xtreemfs.foundation.json.JSONException;
import org.xtreemfs.foundation.json.JSONString;
import org.xtreemfs.foundation.pinky.HTTPHeaders;
import org.xtreemfs.foundation.speedy.MultiSpeedy;
import org.xtreemfs.utils.utils;

public class RandomAccessFile implements ObjectStore {

    private MultiSpeedy         speedy;

    private MRCClient           mrcClient;

    private OSDClient           osdClient;

    private Capability          capability;

    private Locations           locations;

    private Location            selectedReplica;

    private StripingPolicy      selectedReplicaStripingPolicy;

    private List<ServiceUUID>   selectedReplicaOSDs;

    private int                 selectedReplicaStripeSize;

    private String              fileId;

    private String              pathName;

    private InetSocketAddress   mrcAddress;

    private String              authString;

    private ByteMapper          byteMapper;

    private String              newFileSizeHdr;

    private long                filePos;

    private Map<String, String> capAndXLoc;

    private long                capTime;

    public RandomAccessFile(String mode, InetSocketAddress mrcAddress, String pathName,
        MultiSpeedy speedy, String authString, StripingPolicy spolicy) throws Exception {

        this.speedy = speedy;
        this.pathName = pathName;
        this.mrcAddress = mrcAddress;
        this.authString = authString;

        if (speedy == null)
            Logging.logMessage(Logging.LEVEL_DEBUG, this, "speedy is null");

        // use the shared speedy to create an MRC and OSD client
        mrcClient = new MRCClient(speedy, 30000);
        osdClient = new OSDClient(speedy, 30000);

        // create a new file if necessary
        try {
            if (mode.contains("c")) {
                mode = "w";
                mrcClient.createFile(mrcAddress, pathName, authString);
            }
        } catch (HttpErrorException ex) {
            // ignore them
        }

        capAndXLoc = mrcClient.open(mrcAddress, pathName, mode, authString);

        // set and read striping policy
        locations = new Locations(new JSONString(capAndXLoc.get(HTTPHeaders.HDR_XLOCATIONS)));
        capability = new Capability(capAndXLoc.get(HTTPHeaders.HDR_XCAPABILITY));

        capTime = System.currentTimeMillis();

        setReplicaNo(0);

        fileId = capability.getFileId();
        newFileSizeHdr = null;
        filePos = 0;
    }

    public RandomAccessFile(String mode, InetSocketAddress mrcAddress, String pathName,
        MultiSpeedy speedy, String authString) throws Exception {
        this(mode, mrcAddress, pathName, speedy, authString, null);
    }

    public RandomAccessFile(String mode, InetSocketAddress mrcAddress, String pathName,
        MultiSpeedy speedy) throws Exception {
        this(mode, mrcAddress, pathName, speedy, NullAuthProvider.createAuthString(System
                .getProperty("user.name"), MRCClient.generateStringList(System
                .getProperty("user.name"))));
    }

    public RandomAccessFile(String mode, URL mrcURL, String pathName, MultiSpeedy speedy)
        throws Exception {
        this(mode, new InetSocketAddress(mrcURL.getHost(), mrcURL.getPort()), pathName, speedy);
    }

    public RandomAccessFile(String mode, String pathName, MultiSpeedy speedy) throws Exception {
        this(mode, new URL(utils.getxattr(pathName, "xtreemfs.url")), pathName, speedy);
    }

    /**
     *
     * @param resultBuffer
     *            - the buffer into which the data is read.
     * @param offset
     *            - the start offset of the data.
     * @param bytesToRead
     *            - the maximum number of bytes read.
     * @return - the total number of bytes read into the buffer, or -1 if there
     *         is no more data because the end of the file has been reached.
     * @throws Exception
     * @throws IOException
     */
    public int read(byte[] resultBuffer, int offset, int bytesToRead) throws Exception {

        int tmp = byteMapper.read(resultBuffer, offset, bytesToRead, filePos);
        filePos += tmp;
        return tmp;
    }

    /**
     *
     * @param objectNo
     *            - relative object number.
     * @return the number of bytes in the object
     * @throws HttpErrorException
     * @throws IOException
     * @throws JSONException
     * @throws InterruptedException
     */
    public int readObject(int objectNo) throws HttpErrorException, IOException, JSONException,
        InterruptedException {

        // check whether capability needs to be renewed
        checkCap();

        RPCResponse response = null;

        try {
            int current_osd_index = selectedReplicaStripingPolicy.getOSDByObject(objectNo);
            InetSocketAddress current_osd_address = selectedReplicaOSDs.get(current_osd_index)
                    .getAddress();

            response = osdClient.get(current_osd_address, locations, capability, fileId, objectNo);

            String header = response.getHeaders().getHeader(HTTPHeaders.HDR_XINVALIDCHECKSUM);
            if (header != null && header.equalsIgnoreCase("true"))
                throw new IOException("object " + objectNo + " has an invalid checksum");

            ReusableBuffer data = response.getBody();
            if (data == null)
                return 0;

            data.flip();

            return data.limit();
        } finally {
            if (response != null)
                response.freeBuffers();
        }
    }

    /**
     *
     * @param objectNo
     *            - relative object number.
     * @param firstByteInObject
     *            - the first byte to be read.
     * @param bytesInObject
     *            - the maximal number of bytes to be read.
     * @return a ReusableBuffer containing the data which was read.
     */
    public ReusableBuffer readObject(long objectNo, long firstByteInObject, long bytesInObject)
        throws IOException, JSONException, InterruptedException, HttpErrorException {

        int current_osd_index = selectedReplicaStripingPolicy.getOSDByObject(objectNo);
        InetSocketAddress current_osd_address = selectedReplicaOSDs.get(current_osd_index)
                .getAddress();

        RPCResponse response = osdClient.get(current_osd_address, locations, capability, fileId,
            objectNo, firstByteInObject, bytesInObject - 1);

        ReusableBuffer data = response.getBody();
        if (data == null) {
            return null;
        }
        data.flip();
        return data;
    }

    /**
     * Writes bytesToWrite bytes from the writeFromBuffer byte array starting at
     * offset to this file.
     *
     * @param writeFromBuffer
     * @param offset
     * @param bytesToWrite
     * @return the number of bytes written
     * @throws Exception
     */
    public int write(byte[] writeFromBuffer, int offset, int bytesToWrite) throws Exception {

        int tmp = byteMapper.write(writeFromBuffer, offset, bytesToWrite, filePos);
        filePos += bytesToWrite;
        return tmp;
    }

    /**
     * Writes...
     *
     * @param firstByteInObject
     *            - the start offset in the file
     * @param objectNo
     *            - the relative object number
     * @param data
     *            - the data to be written.....
     */
    public void writeObject(long firstByteInObject, long objectNo, ReusableBuffer data)
        throws IOException, JSONException, InterruptedException, HttpErrorException {

        // check whether capability needs to be renewed
        checkCap();

        int current_osd_index = selectedReplicaStripingPolicy.getOSDByObject(objectNo);
        InetSocketAddress current_osd_address = selectedReplicaOSDs.get(current_osd_index)
                .getAddress();

        RPCResponse response = osdClient.put(current_osd_address, locations, capability, fileId,
            objectNo, firstByteInObject, data);

        response.waitForResponse();
        final String tmp = response.getHeaders().getHeader(HTTPHeaders.HDR_XNEWFILESIZE);
        if (tmp != null)
            newFileSizeHdr = tmp;
    }

    public String getStripingPolicy() {
        return selectedReplica.getStripingPolicy().toString();
    }

    public long getStripeSize() {
        // the stripe size of a file is constant.
        return selectedReplicaStripingPolicy.getStripeSize(0);
    }

    public long getStripeSize(long objectNo) {
        return selectedReplicaStripingPolicy.getStripeSize(objectNo);
    }

    public List<ServiceUUID> getOSDs() {
        return selectedReplicaOSDs;
    }

    public long length() throws Exception {
        return (Long) mrcClient.stat(mrcAddress, pathName, false, true, false, authString).get(
            "size");
    }

    public long noOfObjects() throws Exception {
        return (length() / selectedReplicaStripeSize) + 1;
    }

    public ServiceUUID getOSDId(long objectNo) {
        long osd = selectedReplicaStripingPolicy.getOSDByObject(objectNo);
        return selectedReplicaOSDs.get((int) osd);
    }

    public Locations getLocations() {
        return locations;
    }

    public Capability getCapability() {
        return capability;
    }

    public String getFileId() {
        return fileId;
    }

    public String getPath() {
        return pathName;
    }

    public void seek(long pos) {
        filePos = pos;
    }

    public long getFilePointer() {
        return filePos;
    }

    public void flush() throws Exception {
        if (newFileSizeHdr != null)
            this.mrcClient.updateFileSize(mrcAddress, capability.toString(), newFileSizeHdr,
                authString);
    }

    public void delete() throws Exception {
        mrcClient.delete(mrcAddress, pathName, authString);
        RPCResponse r = osdClient.delete(selectedReplicaOSDs.get(0).getAddress(), locations,
            capability, fileId);
        r.waitForResponse();
    }

    public void finalize() {
        if (speedy == null) {
            mrcClient.shutdown();
            osdClient.shutdown();
        }
    }

    private void setReplicaNo(int no) {

        selectedReplica = locations.getLocation(no);
        selectedReplicaStripingPolicy = selectedReplica.getStripingPolicy();
        selectedReplicaOSDs = selectedReplica.getOSDs();
        selectedReplicaStripeSize = (int) selectedReplicaStripingPolicy.getStripeSize(0);

        byteMapper = ByteMapperFactory.createByteMapper(selectedReplicaStripingPolicy
                .getPolicyName(), selectedReplicaStripeSize, this);
    }

    private void checkCap() throws IOException {

        long time = System.currentTimeMillis();

        if (time - capTime > (Capability.DEFAULT_VALIDITY - 60) * 1000) {
            try {
                capAndXLoc = mrcClient.renew(mrcAddress, capAndXLoc, authString);
                capTime = time;
            } catch (Exception e) {
                throw new IOException(e);
            }
        }
    }

}
