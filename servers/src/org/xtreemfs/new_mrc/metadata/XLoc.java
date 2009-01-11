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
 * AUTHORS: Jan Stender (ZIB)
 */

package org.xtreemfs.new_mrc.metadata;


/**
 * Interface for accessing information about single replica (X-Location) of a
 * file. X-Locations are stored in a file's X-Locations List and contain
 * information about file data storage locations.
 */
public interface XLoc {
    
    /**
     * The number of OSDs in the X-Location.
     *  
     * @return the number of OSDs
     */
    public short getOSDCount();
    
    /**
     * Returns the OSD UUID at the given index position.
     * 
     * @param index the index of the OSD UUID
     * @return the OSD UUID at index position <code>position</code>
     */
    public String getOSD(int index);
    
    /**
     * Returns the striping policy assigned to the X-Location.
     * 
     * @return the striping policy
     */
    public StripingPolicy getStripingPolicy();
}