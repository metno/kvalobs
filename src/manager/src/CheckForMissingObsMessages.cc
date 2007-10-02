/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CheckForMissingObsMessages.cc,v 1.5.6.1 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <CheckForMissingObsMessages.h>
#include <miTime>
#include <milog.h>
#include <kvDbGate.h>
#include <kvObsPgm.h>
#include <kvData.h>
#include <kvQueries.h>
  
using namespace kvalobs;

CheckForMissingObsMessages::
CheckForMissingObsMessages(ManagerApp               &app_,
			   dnmi::thread::CommandQue &outputQue_)
  :app(app_), outputQue(outputQue_)
{
}
  

CheckForMissingObsMessages::
CheckForMissingObsMessages(const CheckForMissingObsMessages &c)
  :app(c.app), outputQue(c.outputQue)
{
}

CheckForMissingObsMessages::
~CheckForMissingObsMessages()
{
}
   
void 
CheckForMissingObsMessages::
operator()()
{
  dnmi::db::Connection      *con=0;
  const                     int PROC_MISSING_DATA_MINUTE= 30;
  miutil::miTime            nowtime= miutil::miTime::nowTime();
  miutil::miTime            runtime= nowtime;
  
  runtime.addHour(-1);

  milog::LogContext logContext("CheckForMissingObsMessages");

  while(!app.shutdown()){
       
    // We will periodically check for missing stations in the data using the
    // the observation program (obs_pgm) as reference
    // - use proper time to ensure that the check is run every hour
    //  (offset from 00 hours is 'PROC_MISSING_DATA_MINUTE')
    nowtime= miutil::miTime::nowTime();

    if ((nowtime.min() >= PROC_MISSING_DATA_MINUTE &&
	 nowtime.hour() != runtime.hour()) || 
	miutil::miTime::hourDiff(nowtime,runtime)>2){
      runtime= nowtime;
    } else {
       sleep(1);      
       continue;
    }

    if(!con){
      //Will try to create a new connection to the database, we will
      //not continue before a connection is created or the application
      //is shutdown.

      do{
	con=app.getNewDbConnection();
	
	if(con){
	  LOGDEBUG("Created a new connection to the database!");
	  break;
	}
	
	LOGINFO("Can't create a connection to the database, retry in 5 seconds ..");
	sleep(5);
      }while(!app.shutdown());

      if(!con){
	//We have failed to create a new database connection and we have got a 
	//shutdown condition. We use continue to evaluate the outer while loop 
	//that in turn will end this thread.
	continue; 
      }
    }
    
    // Find missing stations in the data (for the last XX hours)
    // and use the preprocessing routine for finding missing
    // observations to take care of the actual data input
    LOGDEBUG("Check for MISSING data!");
    findMissingData(runtime, con);
    app.releaseDbConnection(con);
    con=0;

    LOGDEBUG("Check for MISSING data completed!");
  }
  
  LOGINFO("Terminating!!");
}


/*
  Find missing stations in the data for the last XX hours, using the 
  observation program as reference.

  Since this process can take a long time to complete we send a 
  StationInfoCommand  to the outputque after every MAX_STATIONS is added to 
  the command. Then check if we are in shutdown, if so return. 
*/


void 
CheckForMissingObsMessages::
findMissingData(const miutil::miTime& runtime,
		dnmi::db::Connection *con)
{
  //We send a StationInfoCommand for every
  //MAX_STATIONS. Sin
  const int MAX_STATIONS=10;
  int   stationCount=0;
  StationInfoCommand  *cmd=0;
  miutil::miTime obstime(runtime.date(),miutil::miClock(runtime.hour(),0,0));
  
  // init database connection
  kvDbGate dbGate(con);
  bool result;

  // fetch the observation program - sorted on stationid
  std::list<kvalobs::kvObsPgm> obspgmlist;

  // check each NUM_HOURS backwards..
  const int NUM_HOURS = 6;
  
  for (int iprev=0; iprev<NUM_HOURS; iprev++){
    obspgmlist.clear();
    result=dbGate.select(obspgmlist,
			 kvQueries::selectObsPgm(obstime));
    
    if(!result){
      LOGERROR("SELECT: (obsPgm" << dbGate.getErrorStr());
     
      if(cmd){
	postCommandToQue(cmd);
      }
      return;
    }

    // check observation program for stations that should have data for
    // obstime
    std::list<kvalobs::kvObsPgm>::const_iterator itop;
    long sid=-1;

    for (itop=obspgmlist.begin(); itop!=obspgmlist.end(); itop++){
      // check if this obspgm is active now..
      if (!itop->isOn(obstime))
	continue;
      
      // once per station
      if (itop->stationID()==sid)
	continue;
      
      sid= itop->stationID();
      
      // then fetch all observations matching stationId, obstime
      std::list<kvalobs::kvData> datalist;
      
      result = dbGate.select(datalist,
			     kvQueries::selectData(sid,
						   obstime,
						   obstime));

      if(!result){
	LOGERROR("SELECT (data): " << dbGate.getErrorStr());
	
	if(cmd){
	  postCommandToQue(cmd);
	}
	return;
      }
      
      if (datalist.size() > 0)
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
      
      
      LOGINFO("Missing observation,station:" << sid << " for obstime:" << obstime
	      << "(Add to missing-data-check)" << std::endl);
      cmd->addStationInfo( kvStationInfo(sid, obstime, kvData::kv_typeid));
      
      stationCount++;

      if(stationCount>=MAX_STATIONS){
	postCommandToQue(cmd);
	cmd=0;
	stationCount=0;
	
	if(app.shutdown())
	  return;
      }
	
    }
    
    // step one hour back - and repeat
    obstime.addHour(-1);
  }
  
  if(cmd){
    postCommandToQue(cmd);
  }

  
  LOGINFO("FindMissingData FINISHED" << std::endl);
}

void
CheckForMissingObsMessages::
postCommandToQue(kvalobs::StationInfoCommand *cmd)
{

  try{
    LOGDEBUG("Sending data to PreProcessWorker!");
    outputQue.postAndBrodcast(cmd);
  }
  catch(...){
    LOGERROR("Cant add the Command to the que. (NOMEM!)");
    delete cmd;
  }
}
