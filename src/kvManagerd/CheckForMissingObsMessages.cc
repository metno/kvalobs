/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CheckForMissingObsMessages.cc,v 1.3.2.10 2007/09/27 09:02:34 paule Exp $                                                       

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
#include <stdlib.h>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvKeyVal.h>
#include <functional>
#include <boost/bind.hpp>
#include "CheckForMissingObsMessages.h"
#include "MissingObsCheck.h"  
#include <kvalobs/kvPath.h>

using namespace kvalobs;
using namespace std;

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

boost::posix_time::ptime
CheckForMissingObsMessages::
lastMissingRuntime(dnmi::db::Connection *con)
{
	const int NUM_HOURS = 6;
	
	
  	if(lastMissingRunTime_.is_not_a_date_time()){
  		std::list<kvKeyVal> keys;
  		
  		kvDbGate gate(con);
  		
  		lastMissingRunTime_=boost::posix_time::microsec_clock::universal_time();
  		lastMissingRunTime_ -= boost::posix_time::hours(NUM_HOURS);
  		
  		if(!gate.select(keys, kvQueries::selectKeyValues(	"kvManagerd", 	"LastMissingRun"))){
  			LOGWARN("Cant access the key 'kvManagerd.LastMissingRun' from the 'key_val' table!" << endl << 
  						  "Reason: " << gate.getErrorStr());
  		}else if(keys.empty()){
  			kvKeyVal keyVal("kvManagerd", "LastMissingRun", to_simple_string(lastMissingRunTime_));
  			
  			if(!gate.insert(keyVal, true)){
  				LOGWARN("Cant insert the key 'kvManagerd.LastMissingRun' into the 'key_val' table!" << endl <<
  				        "Reason: " <<  gate.getErrorStr());
  			}else{
  				LOGINFO("Created new key 'kvManagerd.LastMissingRun' in the 'key_val' table!");
  			}
  		}else{
  			lastMissingRunTime_=boost::posix_time::time_from_string(keys.begin()->val());
  			boost::posix_time::ptime tmp(boost::posix_time::microsec_clock::universal_time());
  			tmp -= boost::gregorian::days(7);
  			
  			LOGINFO("key_val table: Key 'kvManagerd.LastMissingRun': " <<lastMissingRunTime_);
  			
  			if(lastMissingRunTime_<tmp){
  				LOGINFO("The saved 'kvManagerd.LastMissingRun'(" << lastMissingRunTime_ <<") is to old."<< endl
  				        << "Setting it to: " << tmp);
  				lastMissingRunTime_=tmp;
  			}
  		}
  	}
	
	return lastMissingRunTime_;
}

void
CheckForMissingObsMessages::
lastMissingRuntime(dnmi::db::Connection *con, 
                   const boost::posix_time::ptime &newLastMissingRuntime)
{
	kvDbGate gate(con);
	
	kvKeyVal keyVal("kvManagerd", "LastMissingRun", to_simple_string(newLastMissingRuntime));
	
	lastMissingRunTime_=newLastMissingRuntime;
	
	if(!gate.update(keyVal)){
		LOGWARN("Cant update 'kvManagerd.LastMissingRun' in table 'key_val'!");
	}
}


   
void 
CheckForMissingObsMessages::
operator()()
{
	const int PROC_MISSING_DATA_MINUTE= 30;
  	dnmi::db::Connection *con=0;
  	boost::posix_time::ptime        nowtime= boost::posix_time::microsec_clock::universal_time();
  	boost::posix_time::ptime        runtime= nowtime;
  
  	
  	runtime -= boost::posix_time::hours(1);

  	milog::LogContext logContext("CheckForMissingObsMessages");

  	while(!app.shutdown()){
       
    	// We will periodically check for missing stations in the data using the
    	// the observation program (obs_pgm) as reference
    	// - use proper time to ensure that the check is run every hour
    	//  (offset from 00 hours is 'PROC_MISSING_DATA_MINUTE')
    	nowtime= boost::posix_time::microsec_clock::universal_time();

    	if((nowtime.time_of_day().minutes() >= PROC_MISSING_DATA_MINUTE &&
	 		  nowtime.time_of_day().hours() != runtime.time_of_day().hours()) ||
    			nowtime - runtime > boost::posix_time::hours(2) ) {
      	runtime= nowtime;
    	}else{
       	sleep(1);      
       	continue;
    	}
    
    	if(!app.checkForMissingObs()){
        	LOGWARN("Search for missing observation messages is disabled!");
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
findMissingData(const boost::posix_time::ptime& runtime,
					 dnmi::db::Connection *con)
{
 	string logfile;
   boost::posix_time::ptime startAt;
   boost::posix_time::ptime stopAt;
   logfile = kvPath("logdir") + "/kvManagerd-missing.log";

	milog::SetResetDefaultLoggerHelper loggerHelper(logfile, 1048576);

   boost::posix_time::ptime lastMissingRun=lastMissingRuntime(con);

	MissingObsCheck check(*con, outputQue, app.genCache(), 
								boost::bind(&ManagerApp::shutdown, &app));
	
	startAt = boost::posix_time::microsec_clock::universal_time();
	check.findMissingData(runtime, lastMissingRun);
	stopAt = boost::posix_time::microsec_clock::universal_time();
	
	LOGINFO("Missing search: started: " << startAt << " Stopped: " << stopAt
	        << " elpased time: " << stopAt - startAt);

	lastMissingRuntime(con, runtime);
}

