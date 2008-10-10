/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: FileUtilTest.java,v 1.1.2.3 2007/09/27 09:02:44 paule Exp $                                                       

  Copyright (C) 2007 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as 
  published by the Free Software Foundation; either version 2 
  of the License, or (at your option) any later version.
  
  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along 
  with KVALOBS; if not, write to the Free Software Foundation Inc., 
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
package metno.util; 

import java.util.*;
import java.text.*;
import java.sql.*;
import junit.framework.*;


public class MiGMTTimeTest extends TestCase{
   
    public void setUp(){
        
    }
    
    public void tearDown(){
    }

    
    public void testGMTNow(){
    	String epoch = "1970-01-01 00:00:00";
    	String t = "1970-01-10 02:00:00";
    	MiGMTTime tEpoch = new MiGMTTime();
    	MiGMTTime tt = new MiGMTTime();
    	
    	tEpoch.parse( epoch );
    	tt.parse(t);
    	
    	System.out.println("epoch : " + tEpoch );
    	System.out.println("epoch (millis): " + tEpoch.getTime().getTime() );
    	System.out.println("t : " + tt );
    	System.out.println("tt  (millis): " + tt.getTime().getTime() );
    	System.out.println("tt (millis2): " + (3600*24*9 + 3600*2)*1000 );
    	
    	Timestamp sqlT = new Timestamp( tt.getTime().getTime() );
    	System.out.println("sqlT: " + sqlT );
    	
    	TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
    	System.out.println("sqlT (GMT): " + sqlT );
    }
    
    public void testParse(){
    	String t = "1970-01-10 02:00:00";
    	MiGMTTime tt = new MiGMTTime();
    	
    	tt.parse(t);
    	
    	System.out.println("t (GMT) : " + tt.toString() );
    	System.out.println("t : " + tt.toString(MiGMTTime.FMT_ISO) );
    }

}