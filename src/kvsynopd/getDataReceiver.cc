/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: getDataReceiver.cc,v 1.1.2.6 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <list>
#include <sstream>
#include <milog/milog.h>
#include "StationInfo.h"
#include "getDataReceiver.h"
#include "Data.h"
#include "obsevent.h"

using namespace std;
using namespace milog;
using namespace kvservice;  

bool 
GetKvDataReceiver::
next(kvservice::KvObsDataList &dl)
{
  	int  nObs=0;
  	LogContext lContext("GetKvDataReceiver");
 

  	for(kvservice::IKvObsDataList it=dl.begin(); 
      	it!=dl.end(); 
      	it++){

		KvObsData::kvDataList::iterator dit=it->dataList().begin();    

    	if(dit==it->dataList().end())
      		continue;

    	nObs++;
   
    	int            sid=dit->stationID();
    	miutil::miTime obstime=dit->obstime();
    
    	if(logid.empty()){
      		LOGINFO("Data received: stationID: " << sid 
	      		 << " obstime: " << obstime 
	      		 << " parameters: " << it->dataList().size() << endl);
    	}else{
      		IDLOGINFO(logid,"Data received: stationID: " << sid 
				  << " obstime: " << obstime 
				  << " parameters: " << it->dataList().size() << endl);
    	}

    	StationInfoPtr station=app.findStationInfo(sid);
    
    	if(!station){
      		LOGDEBUG("Unexpected station: " <<  it->stationid());

      		if(!logid.empty())
				IDLOGDEBUG(logid, "Unexpected station: " <<  it->stationid());

      		continue;
    	}

    	ObsEvent        *event=0;
      
    	if(obstime>=fromTime){
      		try{
				event=new ObsEvent(obstime, station);
      		} 
      		catch(...){
				LOGERROR("No mem!");
	
				if(!logid.empty())
	  				IDLOGERROR(logid, "No mem!");
	
				return false;
      		}
    	}

    	int             invalidData=0;
    	std::list<Data> myDataList;
     
    	for(;dit!=it->dataList().end(); dit++){
      		if(!station->hasTypeId(dit->typeID())){
				continue;
      		}
      
      		if(event)
      			event->addTypeidReceived(dit->stationID(), dit->typeID());

      		myDataList.push_back(Data(*dit));
    	}
    
    	int nData=0;
    
    	if(myDataList.size()>0){
      		if(!gate.insert(myDataList, true)){
				LOGERROR("Cant insert data: \n  stationid: " << sid 
		   			  << " obstime: " << obstime
		   			  << "  reason: " << gate.getErrorStr() << endl);

				IDLOGERROR(logid,"Cant insert data: \n  stationid: " << 
		   				   sid<< " obstime: " << obstime <<
		   				   "  reason: " << gate.getErrorStr() << endl);
      		}else{
				IDLOGDEBUG(logid,"stationid: " << sid << " obstime: " <<
		    			   obstime << "  inserted: #" << myDataList.size() <<
		     			   " params!" <<endl);

				nData=myDataList.size();
      		}
    	}
    
    	if(nData>0 && event){
      		if(logid.empty()){
				LOGDEBUG("Postevent: stationid=" << sid << " obstime=" << 
		 				 obstime);
      		}else{
				IDLOGDEBUG(logid,"Postevent: stationid=" << sid << " obstime=" << 
		   				   obstime);
      		}
      
      		app.addObsEvent(event, que);
    	}else{
      		if(invalidData==it->dataList().size()){
				LOGDEBUG("Rejected data only!");
				IDLOGDEBUG(logid,"Rejected data only!");
      		}
      		delete event;
    	}
  	}

  	if(nObs==0){
    	LOGDEBUG("No data received!");
    	IDLOGDEBUG(logid,"No data received!");
  	}else{
    	LOGINFO("#" << nObs << " observation proccessed!");
    	IDLOGINFO(logid, "#" << nObs << " observation proccessed!");
  	}

  	return true;
}
