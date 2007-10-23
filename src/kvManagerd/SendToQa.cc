/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SendToQa.cc,v 1.2.2.2 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <unistd.h>
#include "PreProcessWorker.h"
#include <milog/milog.h>
#include <list>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvObsPgm.h>
#include "NewDataCommand.h"

using namespace std;
using namespace kvalobs;

PreProcessWorker::PreProcessWorker(ManagerApp               &app_,
				   dnmi::thread::CommandQue &inputQue_, 
				   dnmi::thread::CommandQue &outputQue_)
    :app(app_),inputQue(inputQue_), outputQue(outputQue_)
{
  jobList=boost::shared_ptr<PreProcessJobList>(new PreProcessJobList);
  oneTimeJobList=boost::shared_ptr<OneTimeJobList>(new OneTimeJobList);
}

PreProcessWorker::PreProcessWorker(const PreProcessWorker &p)
  :app(p.app), jobList(p.jobList), oneTimeJobList(p.oneTimeJobList),
   inputQue(p.inputQue),outputQue(p.outputQue)
{
}

PreProcessWorker::~PreProcessWorker()
{
}

void 
PreProcessWorker::addJob(PreProcessJob *job)
{
  boost::mutex::scoped_lock lock(mutex);

  jobList->push_back(PreProcessJobPtr(job));
}

void 
PreProcessWorker::addOneTimeJob(OneTimeJob *job)
{
  boost::mutex::scoped_lock lock(mutex);
  
  oneTimeJobList->push_back(OneTimeJobPtr(job));
}


void 
PreProcessWorker::
removeJob(PreProcessJob *job)
{
  boost::mutex::scoped_lock lock(mutex);

  IPreProcessJobList it=jobList->begin();

  for(;it!=jobList->end(); it++){
    if(it->get()==job){
      jobList->erase(it);
      return;
    }
  }

}

/*
 * postDataToOutputQue build a new StationInfoCommand from
 * the 'data' table. This is necessary since we do not know 
 * if the preprocessors have added data to the datasets.
 *
 * We delete the old StationInfoCommand from the datainput server.
 */

void
PreProcessWorker::postDataToOutputQue(StationInfoCommand   *infoCmd, 
				      dnmi::db::Connection &con)
{
    kvStationInfoList      &infoList=infoCmd->getStationInfo();
    IkvStationInfoList     it=infoList.begin();
    kvDbGate               gate(&con);
    list<kvData>           data;
    //    list<kvData>::iterator itData;
    NewDataCommand         *newDataCmd;

    try{
	newDataCmd=new NewDataCommand(app);
    }
    catch(...){
	LOGERROR("OUT OF MEMMORY!");
	delete infoCmd;
	return;
    }

    try{
	for(;it!=infoList.end(); it++){
	    if(!gate.select(data, 
			    kvQueries::selectDataFromType(
				it->stationID(),
				it->typeID(),
				it->obstime())))
		continue;
	    
	    
	    kvStationInfo info(it->stationID(),
			       it->obstime(),
			       it->typeID());

	    newDataCmd->addStationInfo(info);

	}

	LOGDEBUG("Post command to output que!");
	outputQue.postAndBrodcast(newDataCmd);
    }
    catch(...){
	delete newDataCmd;
	LOGERROR("PostDataToOutputQue: Unexpected exception!");
    }

    delete infoCmd;
}


/**
 * This is the main loop in the thread that controlls the 
 * preprocessing work. The diffrent preprocessings routines 
 * is in the jobList. The message that comes from the kvDataInputd
 * is in the inputQue and the preprocessed data is posted to the
 * outputQue.
 *
 * Maintance of the DBConnection:
 * We close the connection if it has been idle for longer 
 * than CON_IDLE_TIME seconds. It is, current, hardcoded to 60 
 * seconds.
 */

void 
PreProcessWorker::operator()()
{
  const                     int CON_IDLE_TIME=60;
  const                     int WAIT_ON_QUE_TIMEOUT=1;
  int                       conIdleTime=0;
  StationInfoCommand        *infoCmd;
  IPreProcessJobList        it;
  dnmi::thread::CommandBase *cmd;
  dnmi::db::Connection      *con=0;

  bool                      check_missing_data= false;
  const                     int PROC_MISSING_DATA_MINUTE= 30;
  miutil::miTime            nowtime= miutil::miTime::nowTime();
  miutil::miTime            runtime= nowtime;
  runtime.addHour(-1);

  milog::LogContext logContext("Preprocess");

  while(!app.shutdown()){
    if(oneTimeJobList->empty()){
      cmd=inputQue.get(WAIT_ON_QUE_TIMEOUT);
    }else{
      cmd=0;
    }
    
    if(!cmd){
      conIdleTime+=WAIT_ON_QUE_TIMEOUT;
      
      if(conIdleTime>CON_IDLE_TIME && oneTimeJobList->empty()){
	if(con){
	  LOGDEBUG("Closing the database connection!");
	  app.releaseDbConnection(con);
	  con=0;
	}
      
	conIdleTime=0;
      }

      if(oneTimeJobList->empty())
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

    conIdleTime=0;

    if(!oneTimeJobList->empty()){
      boost::mutex::scoped_lock lock(mutex);
      LOGINFO("Running oneTimeJob: " << oneTimeJobList->front()->jobName());
      oneTimeJobList->front()->runJob(&app, &outputQue, con);
      oneTimeJobList->pop_front();
    }

    if(!cmd)
      continue;

    // PreProcess data from the input queue...
    try{
      infoCmd=dynamic_cast<StationInfoCommand*>(cmd);
      
      if(!infoCmd)
	continue;
    }
    catch(...){
      LOGERROR("Invalid command!");
      continue;
    }
    

    LOGDEBUG("New data received!");
    
    it=jobList->begin();

    kvStationInfoList &infoList=infoCmd->getStationInfo();
    IkvStationInfoList itInfoList;

    if (!infoList.size())
      continue;
    
    boost::mutex::scoped_lock lck(mutex);

    for(;it!=jobList->end(); it++){
      itInfoList=infoList.begin();
      LOGDEBUG("StationInfo: stationID: " << itInfoList->stationID() <<
	       " typeID: " << itInfoList->typeID() << " obstime: " <<
	       itInfoList->obstime());
      
      for(;itInfoList!=infoList.end(); itInfoList++){
	  LOGDEBUG("Preprocessing: jobName: " << (*it)->jobName());
	  (*it)->doJob(itInfoList->stationID(), 
		       itInfoList->typeID(), 
		       itInfoList->obstime(), 
		       *con);
      }
    }

    postDataToOutputQue(infoCmd, *con);
  }

  if(con)
    app.releaseDbConnection(con);

  LOGINFO("Terminating!");
}



