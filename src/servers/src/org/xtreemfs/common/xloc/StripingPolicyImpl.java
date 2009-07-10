/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.xtreemfs.common.xloc;

import java.util.Iterator;

import org.xtreemfs.interfaces.Replica;
import org.xtreemfs.interfaces.StripingPolicy;
import org.xtreemfs.interfaces.StripingPolicyType;

/**
 * 
 * @author bjko
 */
public abstract class StripingPolicyImpl {

    protected final StripingPolicy policy;

    StripingPolicyImpl(Replica replica) {
        policy = replica.getStriping_policy();
    }

    public static StripingPolicyImpl getPolicy(Replica replica) {
        if (replica.getStriping_policy().getType() == StripingPolicyType.STRIPING_POLICY_RAID0) {
            return new RAID0Impl(replica);
        } else {
            throw new IllegalArgumentException("unknown striping polciy requested");
        }
    }

    /**
     * returns the width (number of OSDs) of the striping policy
     * 
     * @return
     */
    public int getWidth() {
        return policy.getWidth();
    }

    public int getPolicyId() {
        return policy.getType().intValue();
    }

    public StripingPolicy getPolicy() {
        return this.policy;
    }

    /**
     * returns the object number for the given offset
     * 
     * @param fileOffset
     * @return
     */
    public abstract long getObjectNoForOffset(long fileOffset);

    /**
     * returns the index of the OSD for the given offset
     * 
     * @param fileOffset
     * @return
     */
    public abstract int getOSDforOffset(long fileOffset);

    /**
     * returns the index of the OSD for the given object
     * 
     * @param objectNo
     * @return
     */
    public abstract int getOSDforObject(long objectNo);

    public abstract long getRow(long objectNo);

    /**
     * returns the first offset of this object
     * 
     * @param objectNo
     * @return
     */
    public abstract long getObjectStartOffset(long objectNo);

    /**
     * returns the last offset of this object
     * 
     * @param objectNo
     * @return
     */
    public abstract long getObjectEndOffset(long objectNo);

    public abstract int getStripeSizeForObject(long objectNo);

    public abstract boolean isLocalObject(long objNo, int relativeOsdNo);

    /**
     * Returns a virtual iterator which iterates over all objects the given OSD should save. It starts with
     * the correct object in the row of startObjectNo (inclusive) and ends with endObjectNo (maybe inclusive).
     * 
     * @param osdIndex
     * @param filesize
     * @return
     */
    public abstract Iterator<Long> getObjectsOfOSD(final int osdIndex, final long startObjectNo,
            final long endObjectNo);
}
