/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvCheckedDataThread.cc,v 1.2.2.1 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <iostream>
#include <milog/milog.h>
#include <kvalobs/kvWorkelement.h>
#include <kvalobs/kvDbGate.h>
#include "kvCheckedDataThread.h"
#include "CheckedDataCommandBase.h"

using namespace std;

  //idName brukes kun i logmeldinger og utskrift til skjerm.
KvCheckedDataThread::
KvCheckedDataThread(ManagerApp &app_, 
		    dnmi::thread::CommandQue &inputQue_)
  :app(app_), inputQue(inputQue_)
{
}
	   

void
KvCheckedDataThread:: 
operator()()
{
  const int                 WAIT_ON_KVSERVICE=60;
  const int                 CON_IDLE_TIME=60;
  const int                 WAIT_ON_QUE_TIMEOUT=1;
  int                       conIdleTime=0;
  dnmi::thread::CommandBase *cmd; 
  CheckedDataCommandBase    *newCmd;
  int                       waitCount=-1;
  dnmi::db::Connection      *dbCon=0;
  CheckedDataHelper         checkedDataHelper(app);

  milog::LogContext logContext("CheckedDataThread");
  LOGINFO("KvCheckedDataThread: starting!\n");

  cleanupWorkque();

  if(!processWaitingObs(checkedDataHelper)){
    waitCount=0;
  }

  while(!app.shutdown()){
    if(waitCount>-1){
      waitCount++;

      if(waitCount<WAIT_ON_KVSERVICE){
	if(dbCon){
	  app.releaseDbConnection(dbCon);
	  dbCon=0;
	}
	sleep(1);
	continue;
      }else{
	if(processWaitingObs(checkedDataHelper)){
	  waitCount=-1;
	}else{
	  waitCount=0;
	  continue;
	}
      }
    }
    
    cmd=inputQue.get(WAIT_ON_QUE_TIMEOUT);

    if(!cmd){
      conIdleTime+=WAIT_ON_QUE_TIMEOUT;
      
      if(conIdleTime>CON_IDLE_TIME){
	if(dbCon){
	  LOGDEBUG("Closing the database connection!");
	  app.releaseDbConnection(dbCon);
	  dbCon=0;
	}

	conIdleTime=0;
      }
      
      continue;
    }
    
    if(!dbCon){
      dbCon=getNewConnection();
      
      if(!dbCon){
	//We have failed to create a new database connection and we have got a 
	//shutdown condition. We use continue to evaluate the outer while loop 
	//that in turn will end this thread.
	continue; 
      }
    }

    conIdleTime=0;
    checkedDataHelper.connection(dbCon);
    
    newCmd=dynamic_cast<CheckedDataCommandBase*>(cmd);
    
    if(!newCmd){
      LOGERROR("Unknown Command." << endl);
      delete cmd;
      continue;
    }

    newCmd->helper(&checkedDataHelper);
    newCmd->execute();
    
    if(!checkedDataHelper.serviceAlive()){
      waitCount=0;
    }else{
      waitCount=-1;
    }
    
    delete newCmd;
    
  }

  if(dbCon)
    app.releaseDbConnection(dbCon);
  
  LOGINFO("Terminated!\n");

}



/*
 * This method is only run when we start up and when
 * kvServiced has been down.
 */
bool
KvCheckedDataThread::
processWaitingObs(CheckedDataHelper &helper)
{
  const int LIMIT=10;
  dnmi::db::Connection *con=getNewConnection();
  ostringstream ost;
  kvalobs::kvStationInfoList  stList;
  kvalobs::kvStationInfoList  sendtList;
  kvalobs::IkvStationInfoList  sit;
  list<kvalobs::kvWorkelement> workList;
  list<kvalobs::kvWorkelement>::iterator it;
  bool ok;

  if(!con) //In shutdown
    return false;

  milog::LogContext logContext("processWaitingObs");

  helper.connection(con);

  kvalobs::kvDbGate gate(con);

  do{

    ost.str("");

    ost << "WHERE qa_stop IS NOT NULL AND " 
	<< "(service_start IS NULL OR service_stop IS NULL) "  
	<< "ORDER BY stationid, typeid, obstime DESC "
	<< "LIMIT " << LIMIT;
    
    LOGDEBUG("SELECT: [" << ost.str() << "]" << endl);
    
    stList.clear();

    if(!gate.select(workList, ost.str(), "workque")){
      LOGERROR("DBERROR: Cant retrive data from workque!" <<
	       "Querry: " << ost.str() << endl <<
	       "Reason: " << gate.getErrorStr());
      app.releaseDbConnection(con);
      return false;
    }

    removeFromSendtList(sendtList, workList);
    
    it=workList.begin();
    
    for(; it!=workList.end(); it++){
      //Check in the prevList that we dont send data
      //twice.
      for(sit=sendtList.begin(); sit!=sendtList.end(); sit++){
	if(it->stationID()==sit->stationID() &&
	   it->obstime() == sit->obstime() &&
	   it->typeID() == sit->typeID())
	  break;
      }

      if(sit==sendtList.end()){
	stList.push_front(kvalobs::kvStationInfo(it->stationID(),
						 it->obstime(),
						 it->typeID()));
	sendtList.push_back(kvalobs::kvStationInfo(it->stationID(),
						 it->obstime(),
						 it->typeID()));
      }
    }
    
    if(!stList.empty())
      ok=helper.sendDataToService(stList);
    else if(!workList.empty())
      sleep(1);
    
    
  }while(workList.size()==LIMIT && ok && !app.shutdown());

  app.releaseDbConnection(con);
  return ok;
}


/**
 * removeFromSendtList removes all elements in sendtList that is 
 * NOT in workList.
 */
void 
KvCheckedDataThread::
removeFromSendtList(kvalobs::kvStationInfoList &sendtList,
		    const list<kvalobs::kvWorkelement> &workList)
{

  kvalobs::IkvStationInfoList  it=sendtList.begin();
  kvalobs::IkvStationInfoList  tmpIt;
  list<kvalobs::kvWorkelement>::const_iterator cIt;

  while(it!=sendtList.end()){
    for(cIt=workList.begin(); cIt!=workList.end(); cIt++){
      if(it->stationID()==cIt->stationID() &&
	 it->obstime() == cIt->obstime() &&
	 it->typeID() == cIt->typeID())
	break;
    }
    
    if(cIt==workList.end()){
      tmpIt=it;
      it++;
      sendtList.erase(tmpIt);
    }else{
      it++;
    }
  }
}

void 
KvCheckedDataThread::
cleanupWorkque()
{
  const int  LIMIT=100;
  ostringstream ost;
  int errorCount;
  list<kvalobs::kvWorkelement> workList;
  list<kvalobs::kvWorkelement>::iterator it;
  dnmi::db::Connection *con=getNewConnection();

  if(!con) 
    return;

  kvalobs::kvDbGate gate(con);

  ost << "WHERE  service_stop IS NOT NULL "  
      << "ORDER BY stationid, typeid, obstime DESC "
      << "LIMIT " << LIMIT ;

  do{
   workList.clear();
    errorCount=0;

    if(!gate.select(workList, ost.str(), "workque")){
      LOGERROR("DBERROR: Cant retrive data from workque!" <<
	       "Querry: " << ost.str() << endl <<
	       "Reason: " << gate.getErrorStr());
    }
    

    for(it=workList.begin(); it!=workList.end(); it++){
      if(!gate.remove(*it, "workque")){
	errorCount++;
	LOGERROR("DBERROR: cant remove from workque: " << endl <<
		 "-- Stationid: " << it->stationID() << " obstime: " <<
		 it->obstime() << " typeid: " << it->typeID() <<
		 endl << "Reason: " << gate.getErrorStr());
      }

      if(!gate.insert(*it, "workstatistik")){
	LOGERROR("DBERROR: cant insert into workstatistik: " << endl <<
		 "-- Stationid: " << it->stationID() << " obstime: " <<
 		 it->obstime() << " typeid: " << it->typeID() <<
		 endl << "Reason: " << gate.getErrorStr());
      }
    }
  }while(workList.size()==LIMIT && errorCount==0);

  app.releaseDbConnection(con);

  LOGINFO("Cleaned up workque: errorCount=" << errorCount);

}


dnmi::db::Connection*
KvCheckedDataThread::
getNewConnection()
{
  dnmi::db::Connection *dbCon;

  do{
    dbCon=app.getNewDbConnection();
    
    if(dbCon){
      LOGDEBUG("New connection to the database created!");
      return dbCon;
    }
    
    LOGINFO("Can't create a connection to the database, retry in 5 seconds ..");
    sleep(5);
  }while(!app.shutdown());
  
  return 0;
}
