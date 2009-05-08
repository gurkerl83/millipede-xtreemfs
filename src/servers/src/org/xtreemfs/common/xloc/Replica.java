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
 * AUTHORS: Björn Kolbeck (ZIB), Christian Lorenz (ZIB)
 */

package org.xtreemfs.common.xloc;

import java.util.ArrayList;
import java.util.List;

import org.xtreemfs.common.uuids.ServiceUUID;

/**
 *
 * @author bjko
 */
public class Replica {

    List<ServiceUUID>  osds;

    StripingPolicyImpl stripingPolicy;

    org.xtreemfs.interfaces.Replica replica;

    public Replica(org.xtreemfs.interfaces.Replica replica) {
        this.replica = replica;
    }

    public StripingPolicyImpl getStripingPolicy() {
        if (stripingPolicy == null)
            stripingPolicy = StripingPolicyImpl.getPolicy(replica);
        return stripingPolicy;
    }

    public List<ServiceUUID> getOSDs() {
        if (osds == null) {
            osds = new ArrayList(replica.getOsd_uuids().size());
            for (String osd : replica.getOsd_uuids()) {
                ServiceUUID uuid = new ServiceUUID(osd);
                osds.add(uuid);
            }
        }
        return osds;
    }

    public int getReplicationFlags() {
        return replica.getReplication_flags();
    }

    public boolean isStriped() {
        return getStripingPolicy().getWidth() > 1;
    }

/*    public boolean isHeadOsd(ServiceUUID localOSD) {
        return this.osds.get(0).equals(localOSD);
    }*/

    public ServiceUUID getHeadOsd() {
        return new ServiceUUID(replica.getOsd_uuids().get(0));
    }

    public String toString() {
        return "Replica "+stripingPolicy+", "+replica+" "+
                "OSD: "+osds;
    }

    /**
     * Provides the responsible OSD for this object.
     * 
     * @param objectNo
     * @return
     */
    public ServiceUUID getOSDForObject(long objectNo) {
        return getOSDs().get(getStripingPolicy().getOSDforObject(objectNo));
    }
    
    /**
     * Provides the responsible OSD for this offset.
     * 
     * @param objectID
     * @return
     */
    public ServiceUUID getOSDForOffset(long offset) {
        return getOSDs().get(getStripingPolicy().getOSDforOffset(offset));
    }

    /**
     * Checks if the given OSD is part of this Replica.
     * @param osd
     * @return
     */
    public boolean containsOSD(ServiceUUID osd) {
        boolean contained = false;
        for(ServiceUUID o : getOSDs()) {
            contained = osd.equals(o);
            if(contained)
                break;
        }
        return contained;
    }
}