/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: getdataWithListener.java,v 1.2.6.1 2007/09/27 09:02:43 paule Exp $                                                       

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
import miutil.MiGMTTime;

public class getdataWithListener {
    
    public static void main(String[] args)
    {
	Param[] param;
	KvApp app=new KvApp(args, "kvalobs", false);
	WhichData[] whichData=new WhichData[1];
	KvDataSubscribeInfo dataSubscribeInfo=new KvDataSubscribeInfo();
	DataReceiver dataReceiver=new DataReceiver();
	String subscriberid;

	MiGMTTime now=new MiGMTTime();
	
	now.addDay(-1);


	//Hent et døgn med data fra kvalobs. Gjelder for alle stasjoner.
	whichData[0]=new WhichData(0, StatusId.All,
				   now.toString(), 
				   "");

	if(!app.getKvData(whichData, dataReceiver)){
	    System.out.println("getKvData: failed!");
	    app.exit();
	    return;
	}
	    
	System.out.println("getKvData: a background thread is started!");
	

	if(false){
	    subscriberid=app.subscribeKvDataListener(dataSubscribeInfo,
						     dataReceiver);  
	    
	    if(subscriberid==null){
		System.out.println("Cant subscribe on <KvData>!");
		app.exit();
	    }
	    
	    System.out.println("Ssubscribe on <KvData>, subscriberid <" 
			       + subscriberid+"> Ctrl+c for å avslutte!");
	}

	app.run();
	
    }
}


	
