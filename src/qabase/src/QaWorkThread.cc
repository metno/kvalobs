/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: QaWorkThread.cc,v 1.24.2.3 2007/09/27 09:02:38 paule Exp $                                                       

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
#include <sstream>
#include "../include/QaWorkThread.h"
#include "../include/qabaseApp.h"
#include "../include/QaWorkCommand.h"
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <puTools/miTime>

using namespace kvalobs;
using namespace std;
using namespace miutil;

QaWork::QaWork(QaBaseApp &app_, const std::string& logpath)
  : app(app_), logpath_(logpath)
{
  control.logpath(logpath);
}


void 
QaWork::operator()()
{ 
  	const int                             CON_IDLE_TIME=60;
  	const int                             WAIT_ON_QUE_TIMEOUT=1;
  	int                                   conIdleTime=0;
  	dnmi::thread::CommandBase             *cmd;
  	QaWorkCommand                         *work;
  	IkvStationInfoList                    it;
  	kvalobs::kvStationInfoList            retList;
  	std::string                           buf;
  	CKvalObs::CManager::CheckedInput_var  callback;
  	dnmi::db::Connection                  *con=0;
  	ostringstream                         ost;
  	bool                                  updateWorkQue=true;
                   
  	LOGINFO("QaWork: starting work thread!\n");

  	while(!app.shutdown()){
    	cmd=app.getInQue().get(WAIT_ON_QUE_TIMEOUT);

    	if(!cmd){
      		conIdleTime+=WAIT_ON_QUE_TIMEOUT;
      
      		if(conIdleTime>CON_IDLE_TIME){
	  
				if(con){
	  				LOGDEBUG("Closing the database connection!");
	  				app.releaseDbConnection(con);
	  				con=0;
				}
	
				conIdleTime=0;
      		}
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
      		} while(!app.shutdown());

      		if(!con){
				//We have failed to create a new database connection and we have got a 
				//shutdown condition. We use continue to evaluate the outer while loop 
				//that in turn will end this thread.
				continue; 
      		}
    	}
    
    	kvDbGate gate(con);

    	conIdleTime= 0;

    	LOGINFO("QaWork: command received....\n");

    	work=dynamic_cast<QaWorkCommand*>(cmd);
    
    	if(!work){
      		LOGERROR("QaWork: Unexpected command ....\n");
      		delete cmd;
      		continue;
    	}

    	it=work->getStationInfo().begin();

    	//The list wil have one and only one element when it is received from 
    	//kvManager.

    	if(it!=work->getStationInfo().end()){ //Paranoia
      		retList.clear();

      		if(work->getKey("logpath", buf)){
				control.logpath(buf);
      		}else{
				control.logpath(logpath_);
      		}
      
      		updateWorkQue=true;

      		if(work->getKey("update_workque", buf)){
				if(buf==std::string("false"))
	  				updateWorkQue=false;
      		}
      
      		if(updateWorkQue){
				LOGDEBUG("UPDATE: workque!");
      		}else{
				LOGDEBUG("NO UPDATE: workque!");
      		}


      		if(updateWorkQue){
				ost.str("");
				ost << "UPDATE workque SET qa_start='" << miTime::nowTime().isoTime() 
	    			<< "' WHERE stationid=" << it->stationID() 
	    			<< "  AND obstime='" << it->obstime().isoTime() 
	    			<< "' AND typeid=" << it->typeID();
	
				if(!gate.exec(ost.str())){
	  				LOGERROR("QaWorkThread: (qa_start) Cant update table workque." <<
		   					 "-- Stationid: " << it->stationID() << endl <<
		   					 "--   obstime: " << it->obstime()  << endl <<
		   					 "--    typeid: " << it->typeID() <<
		   					 "-- query: " << ost.str() << endl <<
		   					 "-- reason: " << gate.getErrorStr());
				}
      		}      
 
      		doWork(*it, retList, con);
      
      		if(!retList.empty()){
				if(updateWorkQue){
	  				ost.str("");
	  				ost << "UPDATE workque SET qa_stop='" << miTime::nowTime().isoTime() 
	      				<< "' WHERE stationid=" << it->stationID() 
	      				<< "  AND obstime='" << it->obstime().isoTime() 
	      				<< "' AND typeid=" << it->typeID();
	  
	  				if(!gate.exec(ost.str())){
	    				LOGERROR("QaWorkThread: (qa_stop) Cant update table workque." <<
		     					 "-- Stationid: " << it->stationID() << endl <<
		     					 "--   obstime: " << it->obstime()  << endl <<
		     					 "--    typeid: " << it->typeID() <<
		     					 "-- query: " << ost.str() << endl <<
		     					 "-- reason: " << gate.getErrorStr());
	  				}
				}
	  
				//Return the result to kvManager.
				callback=work->getCallback();
				app.sendToManager(retList, callback);
      		}
    	}
    
    	delete work;
  	}
  
  	// due for termination..
  	if(con){
    	LOGDEBUG("Closing the database connection before termination!");
    	app.releaseDbConnection(con);
  	}

  	LOGINFO("QaWork: Thread terminating!");
}



/**
 * The retList must contain the result that is to be returned to
 * the kvManager. The result may contain more parameters and there
 * may be results for additional stations. But the station that came
 * in and is to be prossecced must be at the head of the retlist. Other 
 * stations that is touched in the prossecing must be pushed at the tail. 
 */

void
QaWork::doWork(kvalobs::kvStationInfo &params_, 
	       	   kvalobs::kvStationInfoList &retList,
	       	   dnmi::db::Connection *con)
{
  	retList.push_back(params_);
  	kvalobs::kvStationInfo &params=*retList.begin();

  	LOGINFO("QaWork::doWork at:" << miutil::miTime::nowTime()
	  	 << "  Processing " << params.stationID() << " for time "
	  	 << params.obstime() << std::endl);

  	int result= control.runChecks(params,con);
}
