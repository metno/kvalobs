/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: getdata.java,v 1.1.2.2 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.kv2klgetdata;

import kvalobs.*;
import CKvalObs.CService.*;


public class getdata {
    
    public static void main(String[] args)
    {
	Param[] param;
	KvApp app=new KvApp(args, "kvalobs", false);
	WhichData[] whichData=new WhichData[1];
	KvDataIterator dataIter;
	ObsData[]      obsData;

	whichData[0]=new WhichData(18700, StatusId.All,
				   "2004-06-07 14:00:00", 
				   "");
	//whichData[1]=new WhichData(9580, StatusId.All, "2003-11-6 0:0:0", "");
	dataIter=app.getKvData(whichData);

	if(dataIter==null){
	    System.out.println("Cant get <kvData> from KVALOBS!");
	}else{
	    try{
		System.out.println("next dataset!");
		obsData=dataIter.next();
	    }
	    catch(KvNoConnection ex){
		System.out.println("Lost connection with kvalobs!");
		obsData=null;
	    }

	    while(obsData!=null){
		for(int i=0; i<obsData.length; i++){
		    DataElem[] elem=obsData[i].dataList;
		    
		    if(elem!=null){
			System.out.println("stationid: " + elem[0].stationID +
					   " obstime: " + elem[0].obstime +
					   " #params: " + elem.length);

			for(int k=0; k<elem.length; k++){
			    System.out.println(elem[k].stationID + " " +
					       elem[k].obstime + " " +
					       elem[k].paramID + " " +
					       elem[k].original);
			}
			
		    }else{
			System.out.println("Opppsss: NO PARAMS");
		    }
		}

		try{
		    System.out.println("next dataset!");
		    obsData=dataIter.next();
		}
		catch(KvNoConnection ex){
		    System.out.println("Lost connection with kvalobs!");
		    obsData=null;
		}
	    }

	    dataIter.destroy();
	}
	
	app.exit();
    }
}



	
