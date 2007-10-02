/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: getparams.java,v 1.3.6.1 2007/09/27 09:02:43 paule Exp $                                                       

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
import kvalobs.*;
import CKvalObs.CService.*;


public class getparams {
    
    public static void main(String[] args)
    {
	Param[] param;
	KvApp app=new KvApp(args, "kv-glacier", false);

	param=app.getKvParams();

	if(param==null){
	    System.out.println("Cant get param data from KVALOBS!");
	}else{
	    for(int i=0; i<param.length; i++){
		System.out.println(param[i].name 
				   + "  "
				   + param[i].paramID );
	    }
	}
	
	app.exit();
    }
}


	
