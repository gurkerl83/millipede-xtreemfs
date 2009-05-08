/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.xtreemfs.mrc.replication;

import java.net.InetAddress;

import org.xtreemfs.interfaces.ReplicaSet;

/**
 * Selects OSDs according to their DNS names matching the client's name. The
 * OSDs are sorting according to the number of characters that match in both
 * names (starting from the last character).
 * 
 * @author bjko
 */
public class SimpleReplicaSelectionPolicy implements ReplicaSelectionPolicy {
    
    public static final short POLICY_ID = (short) 1; //(short) ReplicaSelectionPolicyType.REPLICA_SELECTION_POLICY_SIMPLE.intValue();
    
    @Override
    public ReplicaSet getSortedReplicaList(ReplicaSet replicas, InetAddress clientAddr) {
        return replicas;
    }
    
}