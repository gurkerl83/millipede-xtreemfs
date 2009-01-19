/*  Copyright (c) 2008 Consiglio Nazionale delle Ricerche and
 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

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
 * AUTHORS: Eugenio Cesario (CNR), Björn Kolbeck (ZIB), Christian Lorenz (ZIB)
 */

package org.xtreemfs.osd.storage;

import java.io.File;
import java.io.FileFilter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.security.NoSuchAlgorithmException;

import org.xtreemfs.common.buffer.BufferPool;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.common.checksums.ChecksumAlgorithm;
import org.xtreemfs.common.checksums.ChecksumFactory;
import org.xtreemfs.common.checksums.StringChecksumAlgorithm;
import org.xtreemfs.common.checksums.algorithms.JavaHash;
import org.xtreemfs.common.checksums.algorithms.SDBM;
import org.xtreemfs.common.clients.osd.ConcurrentFileMap;
import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.common.striping.StripeInfo;
import org.xtreemfs.common.striping.StripingPolicy;
import org.xtreemfs.osd.OSDConfig;

/**
 *
 * @author clorenz
 */
public class HashStorageLayout extends StorageLayout {

    /** 32bit algorithm */
    public static final String JAVA_HASH            = "Java-Hash";

    /** 64bit algorithm */
    public static final String SDBM_HASH            = "SDBM";

    public static final int   SUBDIRS_16            = 15;

    public static final int   SUBDIRS_256           = 255;

    public static final int   SUBDIRS_4096          = 4095;

    public static final int   SUBDIRS_65535         = 65534;

    public static final int   SUBDIRS_1048576       = 1048575;

    public static final int   SUBDIRS_16777216      = 16777215;

    public static final String DEFAULT_HASH         = JAVA_HASH;

    private static final int  DEFAULT_SUBDIRS       = SUBDIRS_256;

    private static final int  DEFAULT_MAX_DIR_DEPTH = 4;

    private static final char VERSION_SEPARATOR     = '.';

    private static final char CHECKSUM_SEPARATOR    = '-';

    private int               prefixLength;

    private StringChecksumAlgorithm hashAlgo;

    private int               hashCutLength;

    private ChecksumAlgorithm checksumAlgo;

    private long              _stat_fileInfoLoads;

    // object list of file IDs, ordered by volume ID
    private ConcurrentFileMap fileMap;
    
    /** Creates a new instance of HashStorageLayout */
    public HashStorageLayout(OSDConfig config, MetadataCache cache) throws IOException {
        this(config, cache, DEFAULT_HASH, DEFAULT_SUBDIRS, DEFAULT_MAX_DIR_DEPTH);
    }

    /**
     * Creates a new instance of HashStorageLayout. If some value is incorrect,
     * the default value will be used.
     *
     * @param config
     * @param hashAlgo
     * @param maxSubdirsPerDir
     * @param maxDirDepth
     * @throws IOException
     */
    public HashStorageLayout(OSDConfig config, MetadataCache cache, String hashAlgo,
        int maxSubdirsPerDir, int maxDirDepth) throws IOException {

        super(config, cache);

        if (hashAlgo == JAVA_HASH) {
        	this.hashAlgo = new JavaHash();
        }else if (hashAlgo == SDBM_HASH) {
            this.hashAlgo = new SDBM();
        }

        if (config.isUseChecksums()) {

            // get the algorithm from the factory
            try {
				checksumAlgo = ChecksumFactory.getInstance().getAlgorithm(config.getChecksumProvider());
			} catch (NoSuchAlgorithmException e) {
                Logging.logMessage(Logging.LEVEL_ERROR, this,
                        "could not instantiate checksum algorithm '" + config.getChecksumProvider()
                            + "'");
                    Logging.logMessage(Logging.LEVEL_ERROR, this, "OSD checksums will be switched off");
			}
        }

        if (maxSubdirsPerDir != 0) {
            this.prefixLength = Integer.toHexString(maxSubdirsPerDir).length();
        } else {
            this.prefixLength = Integer.toHexString(DEFAULT_SUBDIRS).length();
        }

        if (maxDirDepth != 0) {
            this.hashCutLength = maxDirDepth * this.prefixLength;
        } else {
            this.hashCutLength = DEFAULT_MAX_DIR_DEPTH * this.prefixLength;
        }

        _stat_fileInfoLoads = 0;
    }

    public ReusableBuffer readObjectNotPOSIX(String fileId, long objNo, int version, String checksum,
        StripingPolicy sp, long osdNumber) throws IOException {
        ReusableBuffer bbuf = null;

        String fileName = generateAbsolutObjectPath(fileId, objNo, version, checksum);

        File file = new File(fileName);

        if (file.exists()) {

            RandomAccessFile f = new RandomAccessFile(fileName, "r");

            if (f.length() > 0) {
                // read object data
                bbuf = BufferPool.allocate((int) f.length());
                f.getChannel().read(bbuf.getBuffer());

            } else {
                // zero padding...
                bbuf = BufferPool.allocate((int) sp.getStripeSize(objNo));
                for (int i = 0; i < sp.getStripeSize(objNo); i++) {
                    bbuf.put((byte) 0);
                }
            }

            f.close();
            bbuf.position(0);

        }
        return bbuf;
    }

    public boolean checkObject(ReusableBuffer obj, String checksum) {

        // calculate and compare the checksum if checksumming is enabled
        if (checksumAlgo != null && checksum != null) {

            // calculate the checksum
            checksumAlgo.update(obj.getBuffer());
            String calcedChecksum = checksumAlgo.getValue();

            if (Logging.isDebug())
                Logging.logMessage(Logging.LEVEL_DEBUG, this, "calc'ed checksum: " + calcedChecksum
                    + ", stored checksum: " + checksum);

            // test the checksum
            return calcedChecksum.equals(checksum);
        }

        return true;
    }

    public void writeObject(String fileId, long objNo, ReusableBuffer data, int version,
        int offset, String currentChecksum, StripingPolicy sp, long osdNumber) throws IOException {

        // ignore empty writes
        if (data.capacity() > 0) {

            String relPath = generateRelativeFilePath(fileId);
            new File(this.storageDir + relPath).mkdirs();

            try {
                // write file
                String filename = generateAbsoluteObjectPath(relPath, objNo, version,
                    currentChecksum);
                File file = new File(filename);
                RandomAccessFile f = new RandomAccessFile(file, "rw");

                data.position(0);
                f.seek(offset);
                f.getChannel().write(data.getBuffer());
                f.close();

                if (version > 0) {
                    // delete old file
                    String fileNameOld = generateAbsoluteObjectPath(relPath, objNo, version - 1,
                        currentChecksum);
                    File oldFile = new File(fileNameOld);
                    oldFile.delete();
                }

            } catch (FileNotFoundException ex) {
                throw new IOException("unable to create file directory or object: "
                    + ex.getMessage());
            }
        }
    }

    public String createChecksum(String fileId, long objNo, ReusableBuffer data, int version,
        String currentChecksum) throws IOException {

        String relPath = generateRelativeFilePath(fileId);
        new File(this.storageDir + relPath).mkdirs();

        try {

            // if OSD checksums are enabled, calculate the checksum
            if (checksumAlgo != null) {

                String filename = generateAbsoluteObjectPath(relPath, objNo, version,
                    currentChecksum);
                File file = new File(filename);

                // if not data is provided, fetch the object from disk
                if (data == null) {

                    RandomAccessFile f = new RandomAccessFile(file, "rw");
                    data = BufferPool.allocate((int) f.length());
                    f.getChannel().read(data.getBuffer());
                    f.close();

                    checksumAlgo.update(data.getBuffer());
                    BufferPool.free(data);
                }

                // otherwise, calculate the checksum directly from the given
                // buffer
                else
                    checksumAlgo.update(data.getBuffer());

                // calculate the checksum and wrap it into a buffer, in
                // order to write it to the object file
                String checksum = checksumAlgo.getValue();

                // encode the checksum in the file name by renaming the file
                // accordingly
                file.renameTo(new File(
                    generateAbsoluteObjectPath(relPath, objNo, version, checksum)));

                return checksum;
            }

        } catch (FileNotFoundException ex) {
            throw new IOException("unable to create file directory or object: " + ex.getMessage());
        }

        return null;
    }

    public String createPaddingObject(String fileId, long objNo, StripingPolicy sp, int version,
        long size) throws IOException {

        String relPath = generateRelativeFilePath(fileId);
        new File(this.storageDir + relPath).mkdirs();

        // calculate the checksum for the padding object if necessary
        String checksum = null;
        if (checksumAlgo != null) {
            byte[] content = new byte[(int) size];
            checksumAlgo.update(ByteBuffer.wrap(content));
            checksum = checksumAlgo.getValue();
        }

        // write file
        String filename = generateAbsoluteObjectPath(relPath, objNo, version, checksum);
        if (size == sp.getStripeSize(objNo)) {
            File f = new File(filename);
            f.createNewFile();
        } else {
            RandomAccessFile raf = new RandomAccessFile(filename, "rw");
            raf.setLength(size);
            raf.close();
        }

        return checksum;

    }

    public void deleteAllObjects(String fileId) throws IOException {
        File fileDir = new File(generateAbsoluteFilePath(fileId));
        File[] objs = fileDir.listFiles();
        if (objs == null)
            return;
        for (File obj : objs) {
            obj.delete();
        }
    }

    public void deleteFile(String fileId) throws IOException {
        File fileDir = new File(generateAbsoluteFilePath(fileId));
        File[] objs = fileDir.listFiles();
        if (objs == null)
            return;
        for (File obj : objs) {
            obj.delete();
        }

        // delete all empty dirs along the path
        del(fileDir);
    }

    private void del(File parent) {
        if (parent.list().length > 1 || (parent.getAbsolutePath() + "/").equals(this.storageDir)) {
            return;
        } else {
            parent.delete();
            del(parent.getParentFile());
        }
    }

    public void deleteObject(String fileId, final long objNo) throws IOException {
        File fileDir = new File(generateAbsoluteFilePath(fileId));
        File[] objs = fileDir.listFiles(new FileFilter() {

            public boolean accept(File pathname) {
                return pathname.getName().startsWith("" + objNo);
            }
        });
        for (File obj : objs) {
            obj.delete();
        }
    }

    public void deleteObject(String fileId, final long objNo, final int version) throws IOException {
        File fileDir = new File(generateAbsoluteFilePath(fileId));
        File[] objs = fileDir.listFiles(new FileFilter() {

            public boolean accept(File pathname) {
                return pathname.getName().startsWith("" + objNo)
                    && pathname.getName().endsWith("" + version);
            }
        });
        for (File obj : objs) {
            obj.delete();
        }
    }

    public boolean fileExists(String fileId) {
        File dir = new File(generateAbsoluteFilePath(fileId));
        return dir.exists();
    }

    protected FileInfo loadFileInfo(String fileId, StripingPolicy sp) throws IOException {

        _stat_fileInfoLoads = 0;

        FileInfo info = new FileInfo();

        File fileDir = new File(generateAbsoluteFilePath(fileId));
        if (fileDir.exists()) {

            String[] objs = fileDir.list();
            String lastObject = null;
            long lastObjNum = -1;
            // long lastObjNumVer = -1;

            for (String obj : objs) {
                if (obj.startsWith("."))
                    continue; // ignore special files (metadata, .tepoch)
                int cpos = obj.indexOf(VERSION_SEPARATOR);
                int cpos2 = obj.indexOf(CHECKSUM_SEPARATOR);
                String tmp = obj.substring(0, cpos);
                long objNum = Long.valueOf(tmp);
                tmp = cpos2 == -1 ? obj.substring(cpos + 1) : obj.substring(cpos + 1, cpos2);
                int objVer = Integer.valueOf(tmp);
                if (objNum > lastObjNum) {
                    lastObject = obj;
                    lastObjNum = objNum;
                }

                String checksum = null;
                if (cpos2 != -1)
                    checksum = obj.substring(cpos2 + 1);

                Integer oldver = info.getObjVersions().get(objNum);
                if ((oldver == null) || (oldver < objVer)) {
                    info.getObjVersions().put(objNum, objVer);
                    info.getObjChecksums().put(objNum, checksum);
                }
            }
            if (lastObjNum > -1) {
                File lastObjFile = new File(fileDir.getAbsolutePath() + "/" + lastObject);
                long lastObjSize = lastObjFile.length();
                // check for empty padding file
                if (lastObjSize == 0) {
                    lastObjSize = sp.getStripeSize(lastObjSize);
                }
                long fsize = lastObjSize;
                if (lastObjNum > 0) {
                    fsize += sp.getLastByte(lastObjNum - 1) + 1;
                }
                assert (fsize >= 0);
                info.setFilesize(fsize);
                info.setLastObjectNumber(lastObjNum);
            } else {
                // empty file!
                info.setFilesize(0l);
                info.setLastObjectNumber(-1);
            }

            // read truncate epoch from file
            File tepoch = new File(fileDir, TEPOCH_FILENAME);
            if (tepoch.exists()) {
                RandomAccessFile rf = new RandomAccessFile(tepoch, "r");
                info.setTruncateEpoch(rf.readLong());
                rf.close();
            }

        } else {
            info.setFilesize(0);
            info.setLastObjectNumber(-1);
        }

        return info;
    }

    public void setTruncateEpoch(String fileId, long newTruncateEpoch) throws IOException {
        File parent = new File(generateAbsoluteFilePath(fileId));
        if (!parent.exists())
            parent.mkdirs();
        File tepoch = new File(parent, TEPOCH_FILENAME);
        RandomAccessFile rf = new RandomAccessFile(tepoch, "rw");
        rf.writeLong(newTruncateEpoch);
        rf.close();
    }

    private String generateAbsoluteFilePath(String fileId) {
        return this.storageDir + generateRelativeFilePath(fileId);
    }

    private String generateAbsolutObjectPath(String fileId, long objNo, int version, String checksum) {
        StringBuilder path = new StringBuilder(generateAbsoluteFilePath(fileId));
        path.append(generateFilename(objNo, version, checksum));
        return path.toString();
    }

    private String generateAbsoluteObjectPath(String relativeFilePath, long objNo, int version,
        String checksum) {
        StringBuilder path = new StringBuilder(this.storageDir);
        path.append(relativeFilePath);
        path.append(generateFilename(objNo, version, checksum));
        return path.toString();
    }

    private String generateRelativeFilePath(String fileId) {
        StringBuilder path = generateHashPath(fileId);
        path.append(fileId);
        path.append("/");
        return path.toString();
    }

    private StringBuilder generateFilename(long objNo, int version, String checksum) {
        StringBuilder filename = new StringBuilder();
        filename.append(objNo);
        filename.append(VERSION_SEPARATOR);
        filename.append(version);
        if (checksum != null) {
            filename.append(CHECKSUM_SEPARATOR);
            filename.append(checksum);
        }
        return filename;
    }

    /**
     * generates the path for the file with an "/" at the end
     *
     * @param fileId
     * @return
     */
    private StringBuilder generateHashPath(String fileId) {
        StringBuilder hashPath = new StringBuilder(128);
        String hash = hash(fileId);
        int i = 0, j = prefixLength;

        while (j < hash.length()) {
            hashPath.append(hash.subSequence(i, j));
            hashPath.append("/");

            i += prefixLength;
            j += prefixLength;
        }
        if (j < hash.length() + prefixLength) {
            hashPath.append(hash.subSequence(i, hash.length()));
            hashPath.append("/");
        }
        return hashPath;
    }

    /**
     * computes the hash for the File
     *
     * @param str
     * @return
     */
    private String hash(String str) {
    	this.hashAlgo.digest(str);
    	String res = this.hashAlgo.getValue();

        if (res.length() > this.hashCutLength) {
            res = res.substring(0, this.hashCutLength);
        }
        return res;
    }

    public long getFileInfoLoadCount() {
        return _stat_fileInfoLoads;
    }
   
    /**
     * traverse the file tree and fill the fileMap</br>
     * - <code>directory</code> is the root node where the algorithm starts</br>
     * - saves the metaInfos to global <code>fileMap</code></br>
     * - ignores files that start with '.'</br>
     * 
     * @param  directory
     * @throws IOException thrown by fileMap.insert()
     */
    private void traverseFileTree(String directory) throws IOException{
        int PREVIEW_LENGTH = 15;
        
        FileReader fReader;
        File f = new File(directory);
        File[] sub = f.listFiles(); //there will be no error, if the directory is empty
        
        File newestFirst = null;
        File newestLast = null;
        Long objectSize = 0L;
        
        // go through all subs
        for(int i=0;i<sub.length;i++){
            if (sub[i] == null || sub[i].getName().startsWith(".") || sub[i].getName().endsWith(".ser"));//do nothing
            
            // get the file with the newest version  
            else if (sub[i].isFile()) {   
                try{
                    if (newestFirst == null){
                        newestFirst = newestLast = sub[i];
                        objectSize = sub[i].length();
                    }else if (getVersion(sub[i])>getVersion(newestFirst)){
                        newestFirst = newestLast = sub[i];
                        objectSize = (objectSize>=sub[i].length()) ? objectSize : sub[i].length();
                    }else if (getVersion(sub[i])==getVersion(newestFirst)){
                        if (getObjectNo(sub[i])<getObjectNo(newestFirst)){
                            newestFirst = sub[i];
                        }else if (getObjectNo(sub[i])>getObjectNo(newestLast)){
                            newestLast = sub[i];
                        }
                        objectSize = (objectSize>=sub[i].length()) ? objectSize : sub[i].length();
                    }
                }catch(NumberFormatException ne){
                    Logging.logMessage(Logging.LEVEL_WARN, this, "CleanUp: an illegal file was discovered and ignored.");
                }
            // use the next directory as root      
            }else if(sub[i].isDirectory()) { 
                traverseFileTree(sub[i].getAbsolutePath());
            }
            else assert false; //should never be reached
        }    

        if (newestFirst!=null){
            // get a preview from the file
            fReader = new FileReader(newestFirst);
            char[] preview = new char[PREVIEW_LENGTH];
            fReader.read(preview);
            fReader.close();
        
            // get the metaInfo from the root-directory
            int stripCount = getObjectNo(newestLast);
            long fileSize = (stripCount==1) ? newestFirst.length() : (objectSize*stripCount)+newestLast.length();
            
            // insert the data into the fileMap
            fileMap.insert(f.getName(),fileSize,String.valueOf(preview),objectSize); 
        }                        
    }
    
    /**
     * 
     * @param f
     * @return the VersionNo of the given File.
     * @throws NumberFormatException
     */
    private int getVersion(File f) throws NumberFormatException{
        String name = f.getName();
        if (name.indexOf(CHECKSUM_SEPARATOR)!=-1)
            return Integer.parseInt(name.substring(
                                   name.indexOf(VERSION_SEPARATOR)+1, 
                                   name.indexOf(CHECKSUM_SEPARATOR)));
        else return Integer.parseInt(name.substring(
                                    name.indexOf(VERSION_SEPARATOR)+1, 
                                    name.length()));
    }    
    
    /**
     * 
     * @param f
     * @return the ObjectNo of the given File.
     * @throws NumberFormatException
     */
    private int getObjectNo(File f) throws NumberFormatException{
        return Integer.parseInt(f.getName().substring(0, f.getName().indexOf(VERSION_SEPARATOR)));
    } 
    
    /**
     * 
     * @return list of all available files on the storage device ordered by volume IDs
     * @throws IOException 
     * 
     * @throws IOException - thrown by traverseFileTree()
     */
    public ConcurrentFileMap getAllFiles() throws IOException{
        this.fileMap = new ConcurrentFileMap();
        traverseFileTree(this.storageDir);
        return this.fileMap;
    }
}
