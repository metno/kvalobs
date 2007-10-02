/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testTime.java,v 1.1.6.3 2007/09/27 09:02:43 paule Exp $                                                       

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
import java.util.*;
import java.text.*;
import miutil.MiTime;
import miutil.MiGMTTime;

public class testTime{
    
    public static void main(String[] args)
    {
    	MiGMTTime gmtNow=new MiGMTTime();

    	System.out.println(gmtNow);	

    	gmtNow.addDay(-1);

    	System.out.println(gmtNow);
    	
    	String dt="2006-01-31 12:44:00";
    	
    	if(!gmtNow.parse(dt)){
    		System.out.println("Error parse: "+dt);
    	}else{
    	   	System.out.println(dt);
    	   	System.out.println(gmtNow);
    	}
    }
}


	
