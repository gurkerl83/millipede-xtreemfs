package org.xtreemfs.interfaces;

import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.utils.*;
import yidl.Marshaller;
import yidl.Struct;
import yidl.Unmarshaller;




public class VivaldiCoordinates extends Struct
{
    public static final int TAG = 2009090231;
    
    public VivaldiCoordinates() {  }
    public VivaldiCoordinates( double x_coordinate, double y_coordinate, double local_error ) { this.x_coordinate = x_coordinate; this.y_coordinate = y_coordinate; this.local_error = local_error; }

    public double getX_coordinate() { return x_coordinate; }
    public void setX_coordinate( double x_coordinate ) { this.x_coordinate = x_coordinate; }
    public double getY_coordinate() { return y_coordinate; }
    public void setY_coordinate( double y_coordinate ) { this.y_coordinate = y_coordinate; }
    public double getLocal_error() { return local_error; }
    public void setLocal_error( double local_error ) { this.local_error = local_error; }

    // java.io.Serializable
    public static final long serialVersionUID = 2009090231;    

    // yidl.Object
    public int getTag() { return 2009090231; }
    public String getTypeName() { return "org::xtreemfs::interfaces::VivaldiCoordinates"; }
    
    public int getXDRSize()
    {
        int my_size = 0;
        my_size += Double.SIZE / 8; // x_coordinate
        my_size += Double.SIZE / 8; // y_coordinate
        my_size += Double.SIZE / 8; // local_error
        return my_size;
    }    
    
    public void marshal( Marshaller marshaller )
    {
        marshaller.writeDouble( "x_coordinate", x_coordinate );
        marshaller.writeDouble( "y_coordinate", y_coordinate );
        marshaller.writeDouble( "local_error", local_error );
    }
    
    public void unmarshal( Unmarshaller unmarshaller ) 
    {
        x_coordinate = unmarshaller.readDouble( "x_coordinate" );
        y_coordinate = unmarshaller.readDouble( "y_coordinate" );
        local_error = unmarshaller.readDouble( "local_error" );    
    }
        
    

    private double x_coordinate;
    private double y_coordinate;
    private double local_error;    

}

