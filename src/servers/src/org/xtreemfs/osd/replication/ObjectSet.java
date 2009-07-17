/*  Copyright (c) 2009 Barcelona Supercomputing Center - Centro Nacional
    de Supercomputacion and Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

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
 * AUTHORS: Christian Lorenz (ZIB)
 */
package org.xtreemfs.osd.replication;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.BitSet;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Random;
import java.util.zip.DeflaterInputStream;
import java.util.zip.DeflaterOutputStream;

/**
 * Stores the objects in a Java BitSet. <br>
 * 29.06.2009
 */
public class ObjectSet implements Iterable<Long> {
    public static final int DEFAULT_INITIAL_SIZE = 1024;

    /**
     * contains the objects
     */
    protected BitSet        objects;

    /**
     * only every x object will be saved, so the set contains less free entries
     */
    protected int           stripeWidth;

    /**
     * the first object number which should be stored
     */
    protected int           firstObjectNo;

    protected Random        random;

    /**
     * Creates an ObjectSet with a default size of 1024. All objects can be stored to this ObjectSet.
     */
    public ObjectSet() {
        this(1, 0, DEFAULT_INITIAL_SIZE);
    }

    /**
     * Creates an ObjectSet with the given size. All objects can be stored to this ObjectSet.
     * 
     * @param initialSize
     */
    public ObjectSet(int initialSize) {
        this(1, 0, initialSize);
    }

    /**
     * Creates an ObjectSet with a default size of 1024. It uses the stripe width information to store the
     * appropriate objects in a compact manner. Other objects could not be stored.
     * 
     * @param stripeWidth
     * @param firstObjectNo
     *            the first object of the file which should be stored in this object set
     */
    public ObjectSet(int stripeWidth, int firstObjectNo) {
        this(stripeWidth, firstObjectNo, DEFAULT_INITIAL_SIZE);
    }

    /**
     * Creates an ObjectSet with the given size. It uses the stripe width information to store the appropriate
     * objects in a compact manner. Other objects could not be stored.
     * 
     * @param stripeWidth
     * @param firstObjectNo
     *            the first object of the file which should be stored in this object set
     * @param initialSize
     */
    public ObjectSet(int stripeWidth, int firstObjectNo, int initialSize) {
        if (stripeWidth <= 0)
            throw new IllegalArgumentException("stripeWidth must be > 0.");
        if (firstObjectNo < 0)
            throw new IllegalArgumentException("firstObjectNo must be >= 0.");

        this.stripeWidth = stripeWidth;
        this.firstObjectNo = firstObjectNo;
        this.objects = new BitSet(initialSize);
    }

    public ObjectSet(int stripeWidth, int firstObjectNo, byte[] serializedBitSet) throws ClassCastException,
            IOException, ClassNotFoundException {
        // TODO: if the set is too large, the set is compressed
        this.objects = deserialize(serializedBitSet);

        this.stripeWidth = stripeWidth;
        this.firstObjectNo = firstObjectNo;
    }

    /**
     * 
     * @see java.util.Set#add(java.lang.Object)
     */
    public boolean add(Long object) {
        if (!objects.get((int) (object / stripeWidth))) {
            objects.set((int) (object / stripeWidth));
            return true;
        } else
            return false;
    }

    /**
     * 
     * @see java.util.Set#contains(java.lang.Object)
     */
    public boolean contains(Long object) {
        return objects.get((int) (object / stripeWidth));
    }

    /**
     * 
     * @return
     */
    public Long getFirst() {
        return (long) (firstObjectNo + (objects.nextSetBit(0) * stripeWidth));
    }

    /**
     * 
     * @return
     */
    public Long getRandom() {
        // initialize on first call
        if (random == null)
            random = new Random();

        int index;
        do {
            index = objects.nextSetBit(random.nextInt(objects.size()));
        } while (index == -1);
        return (long) (firstObjectNo + (index * stripeWidth));
    }

    /**
     * 
     * @see java.util.Set#iterator()
     */
    public Iterator<Long> iterator() {
        return new Iterator<Long>() {
            int currentPosition = -1;

            @Override
            public Long next() {
                currentPosition = objects.nextSetBit(currentPosition + 1);
                if (currentPosition == -1)
                    throw new NoSuchElementException("iteration has no more elements");
                return (long) (firstObjectNo + (currentPosition * stripeWidth));
            }

            @Override
            public boolean hasNext() {
                return (objects.nextSetBit(currentPosition + 1) == -1) ? false : true;
            }

            @Override
            public void remove() {
                objects.clear(currentPosition);
            }
        };
    }

    /**
     * 
     * @see java.util.Set#remove(java.lang.Object)
     */
    public boolean remove(Long object) {
        if (objects.get((int) (object / stripeWidth))) {
            objects.clear((int) (object / stripeWidth));
            return true;
        } else
            return false;
    }

    /**
     * 
     * @see java.util.Set#isEmpty()
     */
    public boolean isEmpty() {
        return size() == 0;
    }

    /**
     * 
     * @see java.util.Set#size()
     */
    public int size() {
        return objects.cardinality();
    }

    /**
     * 
     * @see java.util.Set#clear()
     */
    public void clear() {
        objects.clear();
    }

    /**
     * Checks if the contents of the sets are the same.<br>
     */
    public boolean equals(Object obj) {
        if (obj instanceof ObjectSet) {
            ObjectSet otherSet = (ObjectSet) obj;
            return stripeWidth == otherSet.stripeWidth && firstObjectNo == otherSet.firstObjectNo
                    && objects.equals(otherSet.objects);
        } else
            return false;
    }

    /**
     * Generates the intersection of this set and the given set. Modifies this set, so only the objects which
     * are contained in both sets remain.
     * 
     * @param otherSet
     * @return
     */
    public boolean intersection(ObjectSet otherSet) {
        int previousLength = objects.cardinality();
        objects.and(otherSet.objects);
        return (objects.cardinality() != previousLength) ? true : false;
    }

    /**
     * Generates the union of this set and the given set. Modifies this set, so all objects which are
     * contained in one of the two sets remain.
     * 
     * @param otherSet
     * @return
     */
    public boolean union(ObjectSet otherSet) {
        int previousLength = objects.cardinality();
        objects.or(otherSet.objects);
        return (objects.cardinality() != previousLength) ? true : false;
    }

    public boolean complement(Long lastObject) {
        int indexOfLastObject = (int) (lastObject / stripeWidth);
        int previousLength = objects.size();

        if (objects.size() - 1 < indexOfLastObject)
            objects.clear(indexOfLastObject);
        objects.flip(0, objects.size());

        return (objects.size() != previousLength) ? true : false;
    }

    /*
     * serialization
     */
    public int getStripeWidth() {
        return stripeWidth;
    }

    public int getFirstObjectNo() {
        return firstObjectNo;
    }

    public byte[] getSerializedBitSet() throws IOException {
        // TODO: compress if the set is too large
        return serialize(objects);
    }

    /**
     * Serializes the given BitSet.
     * 
     * @param set
     * @return
     * @throws IOException
     */
    protected static byte[] serialize(BitSet set) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        ObjectOutputStream oos = new ObjectOutputStream(bos);
        oos.writeObject(set);
        oos.flush();
        oos.close();
        bos.close();
        return bos.toByteArray();
    }

    /**
     * Serializes and compresses the given BitSet.
     * 
     * @param set
     * @return
     * @throws IOException
     */
    protected static byte[] serializeAndCompress(BitSet set) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        DeflaterOutputStream gz = new DeflaterOutputStream(bos);
        ObjectOutputStream oos = new ObjectOutputStream(gz);
        oos.writeObject(set);
        oos.flush();
        oos.close();
        gz.close();
        bos.close();
        return bos.toByteArray();
    }

    /**
     * Deserializes the given object.
     * 
     * @param set
     * @return
     * @throws IOException
     * @throws ClassNotFoundException
     * @throws ClassCastException
     */
    protected static BitSet deserialize(byte[] set) throws IOException, ClassNotFoundException,
            ClassCastException {
        ByteArrayInputStream bis = new ByteArrayInputStream(set);
        ObjectInputStream ois = new ObjectInputStream(bis);
        Object o = ois.readObject();
        ois.close();
        bis.close();

        return (BitSet) o;
    }

    /**
     * Deserializes and decompresses the given object.
     * 
     * @param set
     * @return
     * @throws IOException
     * @throws ClassNotFoundException
     * @throws ClassCastException
     */
    protected static BitSet deserializeAndDecompress(byte[] set) throws IOException, ClassNotFoundException,
            ClassCastException {
        ByteArrayInputStream bis = new ByteArrayInputStream(set);
        DeflaterInputStream gz = new DeflaterInputStream(bis);
        ObjectInputStream ois = new ObjectInputStream(gz);
        Object o = ois.readObject();
        ois.close();
        bis.close();

        return (BitSet) o;
    }
}