/*  Copyright (c) 2010 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

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
package org.xtreemfs.osd.rwre;

import java.net.InetSocketAddress;
import java.util.List;

public class ObjectFetchRecord {

    private static final long NO_TRUNCATE = -1;

    private final long objNumber;

    private long objVersion;

    private final List<InetSocketAddress> osds;

    private int  osdToUse;

    private final long newFileSize;

    private final long newTruncateEpoch;

    public ObjectFetchRecord(long objNo, long objVer, List<InetSocketAddress> osds) {
        super();
        this.objNumber = objNo;
        this.objVersion = objVer;
        this.osds = osds;
        osdToUse = 0;
        newFileSize = NO_TRUNCATE;
        newTruncateEpoch = 0;
    }

    public ObjectFetchRecord(long newFileSize, long newTruncateEpoch) {
        this.newFileSize = newFileSize;
        this.newTruncateEpoch = newTruncateEpoch;
        this.objNumber = -1;
        this.osds = null;
    }

    public boolean isTruncate() {
        return this.newFileSize != NO_TRUNCATE;
    }

    /**
     * @return the objNumber
     */
    public long getObjNumber() {
        return objNumber;
    }

    /**
     * @return the objVersion
     */
    public long getObjVersion() {
        return objVersion;
    }

    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        try {
            ObjectFetchRecord ofr = (ObjectFetchRecord) o;
            return ofr.objNumber == this.objNumber;
        } catch (ClassCastException ex) {
            return false;
        }
    }

    /**
     * @return the osds
     */
    public InetSocketAddress getNextOSD() {
        if (osdToUse < osds.size())
            return osds.get(osdToUse++);
        else
            return null;
    }

    List<InetSocketAddress> getOsds() {
        return osds;
    }

    /**
     * @param objVersion the objVersion to set
     */
    public void setObjVersion(long objVersion) {
        this.objVersion = objVersion;
    }

    public String toString() {
        return objNumber+"@"+objVersion+" osds: "+osds;
    }

    /**
     * @return the newFileSize
     */
    public long getNewFileSize() {
        return newFileSize;
    }

    /**
     * @return the newTruncateEpoch
     */
    public long getNewTruncateEpoch() {
        return newTruncateEpoch;
    }

}
