package org.xtreemfs.osd;

public interface OSDStatusListener {

    
    public void OSDConfigChanged(OSDConfig config);
    
    public void numBytesTXChanged(long numBytesTX);
    
    public void numBytesRXChanged(long numBytesRX);
    
    public void numReplBytesRXChanged(long numReplBytesRX);
    
    public void numObjsTXChanged(long numObjsTX);
    
    public void numObjsRXChanged(long numObjsRX);
    
    public void numReplObjsRX(long numReplObjsRX);
}
