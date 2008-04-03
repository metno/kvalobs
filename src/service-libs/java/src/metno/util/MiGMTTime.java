/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: MiGMTTime.java,v 1.1.2.2 2007/09/27 09:02:43 paule Exp $                                                       

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
import java.sql.Timestamp;

public class MiGMTTime extends MiTime{
    
    public MiGMTTime(){
        super(TimeZone.getTimeZone("GMT"));
    };
    
    public MiGMTTime(MiTime miTime){
        super(miTime);
        cal.setTimeZone(TimeZone.getTimeZone("GMT"));
    };
    
    
    public MiGMTTime(MiGMTTime miTime){
        super(TimeZone.getTimeZone("GMT"));
        set(miTime);
    };
    
    public MiGMTTime(Calendar c){
        super(c);
        cal.setTimeZone(TimeZone.getTimeZone("GMT"));
    };
    
    public MiGMTTime(Date d){
        super(d);
        cal.setTimeZone(TimeZone.getTimeZone("GMT"));
    }
    
    public MiGMTTime(int year, int month, int day){
        super(TimeZone.getTimeZone("GMT"));
        set(year, month, day);
    }
    
    public MiGMTTime(int year, int month, int day, int hour, int min, int sec){
        super(TimeZone.getTimeZone("GMT"));
        set(year, month, day, hour, min, sec);	
    }
    
    public Timestamp getTimestamp() {
    	TimeZone old = TimeZone.getDefault();
    	TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
    	
    	Timestamp ts = new Timestamp( cal.getTimeInMillis() );
    	TimeZone.setDefault( old );
    	return ts;
    }
}
