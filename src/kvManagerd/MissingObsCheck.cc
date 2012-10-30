/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: MissingObsCheck.cc,v 1.1.2.2 2007/09/27 09:02:34 paule Exp $                                                       

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
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvWorkelement.h>
#include <puTools/miClock.h>
#include "MissingObsCheck.h"

using namespace kvalobs;
using namespace std;

void 
MissingObsCheck::
postCommandToQue(kvalobs::StationInfoCommand *cmd)
{
	kvalobs::kvStationInfoList stationInfoList=cmd->getStationInfo();
  	IkvStationInfoList it=stationInfoList.begin();
  	kvDbGate gate(con);
  	boost::posix_time::ptime undefTime;
  	boost::posix_time::ptime tbtime = boost::posix_time::microsec_clock::universal_time();

  	for(;it!=stationInfoList.end(); it++){
    	if(!gate.insert(kvWorkelement(it->stationID(), 
				  			               it->obstime(), 
				  								it->typeID(), 
				  								tbtime, 
				  								10,
				  								undefTime,
				  								undefTime, 
				  								undefTime, 
				  								undefTime, 
				  								undefTime), 
		    				 true)){
      	LOGERROR("addStationInfo: can't save kvWorkelement into the" <<endl <<
	       			"the table 'workque' in  the database!\n" <<
	       			"[" << gate.getErrorStr() << "]");
    	}
  	}

  	try{
    	LOGDEBUG("Sending data to PreProcessWorker!");
    	outputQue.postAndBrodcast(cmd);
  	}
  	catch(...){
    	LOGERROR("Cant add the Command to the que. (NOMEM!)");
    	delete cmd;
  	}
}

bool 
MissingObsCheck::
checkObstime(const boost::posix_time::ptime &runTime,
                    boost::posix_time::ptime &obstime,
	                    const long lateobsInMinute)
{
	int min=20;
	int h, m;
	
	h=lateobsInMinute/60;
	m=lateobsInMinute%60;
	
	boost::posix_time::ptime checkTime=boost::posix_time::ptime(runTime.date(),
	                                        boost::posix_time::time_duration(runTime.time_of_day().hours(), 0, 0));
	
	checkTime -= boost::posix_time::hours(h);
	checkTime += boost::posix_time::minutes(m);
	
	obstime=boost::posix_time::ptime(checkTime.date(), boost::posix_time::time_duration(checkTime.time_of_day().hours(), 0, 0));
	
	return checkTime<runTime;
}


MissingObsCheck::
MissingObsCheck(dnmi::db::Connection     &con_,
		   		 dnmi::thread::CommandQue &outputQue_,
		   		 GenCache                 *genCache_,
		   		 ShutdownHook             shutdown_)
	:con(&con_), outputQue(outputQue_), genCache(*genCache_), shutdown(shutdown_)
{
}

MissingObsCheck::
~MissingObsCheck()
{
}



void 
MissingObsCheck::
findMissingData(const boost::posix_time::ptime& runtime,
  	   	       const boost::posix_time::ptime& lastSearchForMissing)
{
	//We send a StationInfoCommand for every
  	//MAX_STATIONS. Sin
  	const int MAX_STATIONS=10;
  	int   stationCount=0;
  	StationInfoCommand  *cmd=0;
  	boost::posix_time::ptime searchUntilObstime(lastSearchForMissing);
  	boost::posix_time::ptime obstime;
  	boost::posix_time::ptime nowObstime(runtime.date(), boost::posix_time::time_duration(runtime.time_of_day().hours(), 0, 0));
  	int            nTimes;
    
  	// init database connection
  	kvDbGate dbGate(con);
  	bool result;

  	// fetch the observation program - sorted on stationid
  	list<kvalobs::kvObsPgm> obspgmlist;
  	list<kvalobs::kvTypes>  typeslist;
	
	searchUntilObstime=boost::posix_time::ptime(lastSearchForMissing.date(),
	                                  boost::posix_time::time_duration(lastSearchForMissing.time_of_day().hours(), 0, 0));
	
	

	nTimes = (nowObstime - searchUntilObstime).total_seconds() / 3600;
	//nTimes=boost::posix_time::ptime::hourDiff(nowObstime, searchUntilObstime);

	LOGINFO("Start search: " << runtime << endl <<
	        "Last search:  " << lastSearchForMissing << endl <<
	        "obstime:      " << nowObstime << endl << 
	        "Search until obstime: " << searchUntilObstime << endl <<
	        "nTimes:       " << nTimes );
  	
  	if(!dbGate.select(typeslist)){
  		LOGERROR("SELECT: (types) " << dbGate.getErrorStr());
  		return;
  	}

  	list<kvalobs::kvTypes>::const_iterator  itTypes;
  
  	for(itTypes=typeslist.begin(); 
  		  itTypes!=typeslist.end(); itTypes++){
  		  	
	  	if(!checkObstime(runtime, obstime, itTypes->lateobs())){
	  		//Adjust the obstime with one hour earlyer.
	  		obstime -= boost::posix_time::hours(1);
	  	}
  		
  		LOGINFO("Searching for typeid: " << itTypes->typeID() << " lateobs: " << itTypes->lateobs() << endl
  		   		<<  "obstime: " << obstime);
  		
  		for(int i=0; i<nTimes; i++){
  			obspgmlist.clear();
    		result=dbGate.select(obspgmlist,
				 						kvQueries::selectObsPgmByTypeid(itTypes->typeID(),
				 						                                obstime));
    
    		if(!result){
      		LOGERROR("SELECT: (obsPgm)" << dbGate.getErrorStr());
     
      		if(cmd)
					postCommandToQue(cmd);

	      	return;
    		}

			LOGINFO("Searching for typeid: " << itTypes->typeID() << " obstime: " << obstime << 
			        " obspgmlist size: " << obspgmlist.size());

    		// check observation program for stations that should have data for
    		// obstime
    		list<kvalobs::kvObsPgm>::const_iterator itop;
    		long sid=-1;
    		long tid=-1;

    		for (itop=obspgmlist.begin(); 
    		     itop!=obspgmlist.end(); itop++){
      		
      		if(itop->collector())
      			continue;
      		
      		// check if this obspgm is active now..
      		if (!itop->isOn(obstime))
					continue;
      
      		// once per station and typeid
      		if (itop->stationID()==sid && itop->typeID()==tid)
					continue;

      		tid=itop->typeID();
      		sid=itop->stationID();

      
      		// then fetch all observations matching stationId, typeid and obstime
      		list<kvalobs::kvData> datalist;
      
      		result = dbGate.select(datalist,
			   	  						  kvQueries::selectDataFromType(sid,
							   		  										  	  tid,
							   		                                   obstime));

      		if(!result){
					LOGERROR("SELECT (data): " << dbGate.getErrorStr());
	
					if(cmd)
	  					postCommandToQue(cmd);
				
					return;
      		}
      
      		if (datalist.size() > 0)
					continue;

      		//We dont generate missing for generated types only.
      		if(genCache.isGenerated(sid, tid, con))
					continue;
      
      		if(!cmd){
					try{
	  					cmd= new kvalobs::StationInfoCommand();
					}
					catch(...){
	  					LOGERROR("MOMEM: Cant alocate <kvalobs::StationInfoCommand>!");
	  					return;
					}
      		}
      
      
      		LOGINFO("Missing observation: stationid: " << sid << " typeid: " << 
	      				tid << endl << "For obstime: " << obstime << endl);

      		cmd->addStationInfo(kvStationInfo(sid, obstime, tid));
      
      		stationCount++;

      		if(stationCount>=MAX_STATIONS){
					postCommandToQue(cmd);
					cmd=0;
					stationCount=0;
	
					if(shutdown)
						if(shutdown())
	  						return;
      		}
    		}
    
    		// step one hour back - and repeat
    		obstime -= boost::posix_time::hours(-1);
  		}
	}
  
  	if(cmd)
   	postCommandToQue(cmd);
  
  	LOGINFO("FindMissingData FINISHED" << endl);
	
}   
