/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: App.cc,v 1.19.2.14 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <string>
#include <sstream>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvskel/kvsynopd.hh>
#include "obsevent.h"
#include "ValidData.h"
#include "tblWaiting.h"
#include "Data.h"
#include "App.h"
#include "StationInfoParse.h"
#include "tblKeyVal.h"
#include "getDataReceiver.h"
#include "GetDataThread.h"

using namespace std;
using namespace miutil;
using namespace kvalobs;
using namespace milog;
using namespace miutil::conf;
using boost::mutex;
using namespace kvservice;


bool
App::
createGlobalLogger(const std::string &id)
{
  	try{
    	FLogStream *logs=new FLogStream(2, 204800); //200k
    	std::ostringstream ost;
    
    	ost << kvpath() << "/var/log/kvsynop/" << id << ".log";
    
    	if(logs->open(ost.str())){
      		if(!LogManager::createLogger(id, logs)){
				delete logs;
				return false;
      		}

      		return true;
    	}else{
      		LOGERROR("Cant open the logfile <" << ost.str() << ">!");
      		delete logs;
      		return false;
    	}
  	}
  	catch(...){
    	LOGERROR("Cant create a logstream for LOGID " << id);
    	return false;
  	}
}

App::
App(int argn, char **argv, const std::string &kvpath,   
    const std::string &confFile_, miutil::conf::ConfSection *conf):
  kvservice::KvApp(argn, argv, conf), 
  startTime_(miutil::miTime::nowTime()),
   confFile(confFile_), 
  kvpath_(kvpath),
  hasStationWaitingOnCacheReload(false),
  acceptAllTimes_(false)
{
  ValElementList valElem;
  string         val;

  LogContext context("ApplicationInit");

  createGlobalLogger("GetData");
  createGlobalLogger("DelayCtl");
  createGlobalLogger("main");


  //If a station is set up whith this types delay them if
  //they has not at least 4 hours with data.
  continuesTypeID_.push_back(311);
  continuesTypeID_.push_back(310);
  continuesTypeID_.push_back(3);
  continuesTypeID_.push_back(330);

  valElem=conf->getValue("accept_all_obstimes");

  if(!valElem.empty()){
    string t=valElem[0].valAsString();
    
    if(!t.empty() && (t[0]=='t' || t[0]=='T'))
        acceptAllTimes_=true;
  }

  if(acceptAllTimes_){
    IDLOGINFO("main", "Accepting all obstimes.");
  }else{
    IDLOGINFO("main", "Rejecting obstimes that is too old or to early.");
  }


  valElem=conf->getValue("database.driver");

  if(valElem.empty()){
    LOGFATAL("No <database.driver> in the configurationfile!");
    exit(1);
  }

  dbDriver=valElem[0].valAsString();
  
  LOGINFO("Loading driver for database engine <" << dbDriver << ">!\n");
  
  if(!dbMgr.loadDriver(dbDriver, dbDriverId)){
    LOGFATAL("Can't load driver <" << dbDriver << endl 
	     << dbMgr.getErr() << endl 
	     << "Check if the driver is in the directory $KVALOBS/lib/db???");

    exit(1);
  }

  valElem=conf->getValue("database.dbconnect");

  if(valElem.empty()){
    LOGFATAL("No <database.dbconnect> in the configurationfile!");
    exit(1);
  }

  dbConnect=valElem[0].valAsString();
  
  LOGINFO("Connect string <" << dbConnect << ">!\n");

  valElem=conf->getValue("corba.kvpath");

  if(valElem.empty()){
    mypathInCorbaNS=kvpathInCorbaNameserver();
  }else{
    mypathInCorbaNS=valElem[0].valAsString();
  }

  if(!mypathInCorbaNS.empty() && 
     mypathInCorbaNS[mypathInCorbaNS.length()-1]!='/')
    mypathInCorbaNS+='/';

  if(!readStationInfo(conf)){
    LOGFATAL("Exit! No configuration!");
    exit(1);
  }
  
  readWaitingElementsFromDb();

  //We dont need conf any more.
  delete conf;

}

App::
~App()
{
}



void 
App::
readWaitingElementsFromDb()
{
  dnmi::db::Connection *con=getNewDbConnection();
  list<TblWaiting> data;
  list<TblWaiting>::iterator it;

  if(!con)
    return;

  kvDbGate gate(con);
  gate.busytimeout(120);

  if(gate.select(data, " order by delaytime")){
    for(it=data.begin(); it!=data.end(); it++){
      StationInfoPtr info=findStationInfoWmono(it->wmono());
     
      if(info){
	try{
	  waitingList.push_back(WaitingPtr(new Waiting(it->delaytime(), 
					    it->obstime(), 
					    info)));
	}
	catch(...){
	  LOGFATAL("NOMEM: while reading 'waiting' from database!");
	  exit(1);
	}
      }else{
	gate.remove(*it);
      }
    }
  }else{
    LOGERROR("ERROR (Init): While reading 'waiting' from database!" << endl
	     << gate.getErrorStr());
  }

  releaseDbConnection(con);
}  

void           
App::
continuesTypeID(const std::list<int> &continuesTimes)
{
  mutex::scoped_lock lock(mutex);
  continuesTypeID_=continuesTimes;
}

std::list<int> 
App::
continuesTypeID()
{ 
  mutex::scoped_lock lock(mutex);

  return continuesTypeID_;
}


/**
 * Return true if the station has only typeid that is NOT in the list
 * continuesTypeID.
 */
bool           
App::
onlyNoContinuesTypeID(StationInfoPtr st)
{
  mutex::scoped_lock lock(mutex);

  StationInfo::TLongList tp=st->typepriority();
  StationInfo::ITLongList itp;

  for(list<int>::iterator it=continuesTypeID_.begin();
      it!=continuesTypeID_.end(); it++){
    
    for(itp=tp.begin(); itp!=tp.end(); itp++){
      if(*it==*itp)
	return false;
    }
  }
   
  return true;
}


bool 
App::
isContinuesType(int typeID)
{
  mutex::scoped_lock lock(mutex);

  for(list<int>::iterator it=continuesTypeID_.begin();
      it!=continuesTypeID_.end(); it++){
    
    if(*it==typeID){
      return true;
    }
  }
   
  return false;
}

bool
App::
readStationInfo(miutil::conf::ConfSection *conf)
{
  StationInfoParse theParser;
  std::list<StationInfoPtr> tmpList;

  if(!theParser.parse(conf, tmpList)){
    LOGFATAL("Cant parse the SYNOP configuration.");
    return false;
  }    

  stationList=tmpList;
  
  return true;
}

bool
App::
readStationInfo(std::list<StationInfoPtr> &stList)const
{
  StationInfoParse theParser;

  LOGDEBUG2("Reading conf from file!" << endl <<
	    "<"<<confFile<<">" <<endl);

  miutil::conf::ConfSection *conf=KvApp::readConf(confFile);

  if(!conf)
    return false;

  stList.clear();

  if(!theParser.parse(conf, stList)){
    LOGWARN("Cant parse the SYNOP configuration!" << endl 
	    << "File: <" << confFile << ">" << endl );
    return false;
  }    

  return true;
}



bool
App::
listStations(kvsynopd::StationInfoList &list)
{
  ostringstream ost;

  list.length(stationList.size());

  IStationList it=stationList.begin();

  for(CORBA::Long i=0; it!=stationList.end(); it++, i++){
    list[i].wmono=(*it)->wmono();
    
    StationInfo::TLongList stid=(*it)->stationID(); 
    StationInfo::ITLongList itl=stid.begin();
    list[i].stationIDList.length(stid.size());

    for(int j=0; itl!=stid.end(); itl++, j++){
      list[i].stationIDList[j]=*itl;
    }
    
    ost.str("");
    ost << **it;

    list[i].info=CORBA::string_dup(ost.str().c_str());
  }
  
  return true;
}


dnmi::db::Connection*
App::
getNewDbConnection()
{
  dnmi::db::Connection *con;
  
  con=dbMgr.connect(dbDriverId, dbConnect);
  
  if(!con){
    LOGERROR("Can't create a database connection  (" 
	     << dbDriverId << ")" << endl << "Connect string: <" << dbConnect << ">!");
    return 0;
  }
  
  LOGINFO("New database connection (" << dbDriverId 
	    << ") created!");
  return con;
}

void                  
App::
releaseDbConnection(dnmi::db::Connection *con)
{
  dbMgr.releaseConnection(con);
}


StationInfoPtr 
App::
findStationInfo(long stationid)
{
  IStationList it=stationList.begin();

  for(;it!=stationList.end(); it++){
    if((*it)->hasStationId(stationid)){
      break;
    }
  }

  if(it!=stationList.end())
    return *it;

  return StationInfoPtr();
}


StationInfoPtr 
App::
findStationInfoWmono(int wmono)
{
  IStationList it=stationList.begin();

  for(;it!=stationList.end(); it++){
    if((*it)->wmono()==wmono){
      break;
    }
  }
  
  if(it!=stationList.end())
    return *it;

  return StationInfoPtr();
}




WaitingPtr
App::
addWaiting(WaitingPtr w, bool replace, dnmi::db::Connection *con)
{
  IWaitingList it;

  mutex::scoped_lock lock(mutex);
  
  for(it=waitingList.begin(); it!=waitingList.end(); it++){
    if(w->info()->wmono()==(*it)->info()->wmono() &&
       w->obstime()==(*it)->obstime()){

      if(replace){
	if(w->delay()!=(*it)->delay()){
	  LOGINFO("Replace delay element for: " << w->info()->wmono() 
		  << " obstime: "  << w->obstime() << " delay to: " 
		  << w->delay());
	  *it=w;
	  w->addToDb(con);
	}
      }
      return *it;
    }
  }

  LOGINFO("Add delay element for: " << w->info()->wmono() 
	  << " obstime: "  << w->obstime() << " delay to: " 
	  << w->delay());
  
  w->addToDb(con);
  //We have now record for this station and obstime in the waitingList.

  if(waitingList.empty()){
    waitingList.push_back(w);
    return WaitingPtr();
  }

  for(it=waitingList.begin(); it!=waitingList.end(); it++){
    if(w->delay()<=(*it)->delay()){
      break;
    }
  }
  
  waitingList.insert(it, w);

  return WaitingPtr();
}

WaitingPtr 
App::    
getWaiting(const miutil::miTime &obstime,
	   int                  wmono,
	   dnmi::db::Connection *con)
{
  mutex::scoped_lock lock(mutex);
  IWaitingList it;

  for(it=waitingList.begin(); it!=waitingList.end(); it++){
    if((*it)->info()->wmono()==wmono &&
       (*it)->obstime()==obstime){
	 (*it)->removeFrom(con);
	  WaitingPtr w=*it;
	  waitingList.erase(it);
    
	  return w;
    }
  }

  return WaitingPtr();
}
  


WaitingList    
App::
getExpired()
{
  WaitingList  wl;
  IWaitingList it;
  IWaitingList itTmp;
  miTime       now;
  ostringstream ost;
  bool          msg=false;

  mutex::scoped_lock lock(mutex);
  
  milog::LogContext context("Delay");

  now=miTime::nowTime();
  it=waitingList.begin();

  while(it!=waitingList.end() && (*it)->delay()<=now){
    LOGDEBUG("getExpired: loop");
    if(!msg){
      ost << "Expired delay for stations at time: " << now << endl;
      msg=true;
    }
    ost << "-- " << (*it)->info()->wmono() << " obstime: " << (*it)->obstime() 
	<< " delay to: " << (*it)->delay() << endl;
    wl.push_back(*it);
    waitingList.erase(it);
    it=waitingList.begin();
  }

  if(msg){
    LOGINFO(ost.str());
  }

  if(!wl.empty()){
    dnmi::db::Connection *con=getNewDbConnection();
    
    if(con){
      for(it=wl.begin(); it!=wl.end(); it++){
	(*it)->removeFrom(con);
      }

      releaseDbConnection(con);
    }
  }

  return wl;
}

kvsynopd::DelayList* 
App::
getDelayList(miutil::miTime &nowTime)
{
  kvsynopd::DelayList *dl;
    
  mutex::scoped_lock lock(mutex);

  nowTime=miTime::nowTime();

  try{
    dl=new kvsynopd::DelayList();
  }
  catch(...){
    return 0;
  }

  if(waitingList.empty())
    return dl;

  dl->length(waitingList.size());
  IWaitingList it=waitingList.begin();
  CORBA::Long  i;

  for(it=waitingList.begin(), i=0;
      it!=waitingList.end();
      it++, i++){
    
    try{
      (*dl)[i].wmono=(*it)->info()->wmono();
      (*dl)[i].obstime=(*it)->obstime().isoTime().c_str(); 
      (*dl)[i].delay=(*it)->delay().isoTime().c_str();
    }
    catch(...){
      break;
    }
  }

  return dl;
}


void           
App::
removeWaiting(int wmono, 
	      const miutil::miTime &obstime, 
	      dnmi::db::Connection *con)
{
  IWaitingList it;
  
  mutex::scoped_lock lock(mutex);
  
  for(it=waitingList.begin(); it!=waitingList.end(); it++){
    if(wmono==(*it)->info()->wmono() &&
       obstime==(*it)->obstime()){

      if(!(*it)->removeFrom(con)){
	LOGWARN("Cant remove waiting element from database: wmono: " 
		<< wmono << " obstime: " << obstime);
      
      }
      
      LOGINFO("Removed waiting element wmono: " 
	      << wmono << " obstime: " << obstime << endl
	      << " with delay: " << (*it)->delay());

      waitingList.erase(it);
      
      return;
    }
  }
}

void           
App::
removeWaiting(WaitingPtr w, dnmi::db::Connection *con)
{
  LOGINFO("remove: " << w->info()->wmono() << " obstime: " << w->obstime() 
	  << " delay to: " << w->delay() << " from database!");

  w->removeFrom(con);
}

miutil::miTime 
App::
checkpoint()
{
  dnmi::db::Connection *con=getNewDbConnection();
  list<TblKeyVal> data;
  list<TblKeyVal>::iterator it;

  if(!con){
    LOGERROR("DBERROR: Cant create a connection to the database!");
    return miTime();
  }

  kvDbGate gate(con);

  gate.busytimeout(120);

  if(!gate.select(data, "WHERE key=\'checkpoint\'")){
    LOGERROR("DBERROR: cant obtain checkpoint!");
    releaseDbConnection(con);
    return miTime();
  }

  releaseDbConnection(con);

  if(data.empty()){
    LOGINFO("No checkpont!");
    return miTime();
  }

  return miTime(data.front().val());
}

void           
App::
createCheckpoint(dnmi::db::Connection *con,
		      const miutil::miTime &cpoint)
{
  kvDbGate gate(con);
  gate.busytimeout(120);

  if(cpoint.undef()){
    LOGERROR("Checkpoint: undef checkpont time!");
    return;
  }
  
  if(!gate.insert(TblKeyVal("checkpoint", cpoint.isoTime()), true)){
    LOGERROR("Failed to create checkpoint! (" <<cpoint <<")");
  }else{
    LOGINFO("Checkpoint created at: " << cpoint);
  }
}

void           
App::
createCheckpoint(const miutil::miTime &cpoint)
{
  if(cpoint.undef()){
    LOGERROR("Checkpoint: undef checkpont time!");
    return;
  }
  
  dnmi::db::Connection *con=getNewDbConnection();

  if(!con){
    LOGERROR("DBERROR: checkpoint: Cant create a connection to the database!");
    return;
  }

  createCheckpoint(con, cpoint);
  
  releaseDbConnection(con);
}

StationInfoPtr 
App::
replaceStationInfo(StationInfoPtr newInfoPtr)
{
  mutex::scoped_lock lock(mutex);

  IStationList it=stationList.begin();
  
  for(;it!=stationList.end(); it++){
    if((*it)->wmono()==newInfoPtr->wmono()){
      
      //Set the cacheReload of the new StationInfo to the same as 
      //reload.
      newInfoPtr->cacheReloaded48((*it)->cacheReloaded48());
      StationInfoPtr info=*it;
      *it=newInfoPtr;
      return info;
    }
  }
  
  return StationInfoPtr();
}

bool
App:: 
addStationInfo(StationInfoPtr newInfoPtr)
{
  
  IStationList it=stationList.begin();
  
  for(;it!=stationList.end(); it++){
    if((*it)->wmono()==newInfoPtr->wmono()){
      return false;
    }
  }
  
  stationList.push_back(newInfoPtr);
  
  return true;
}


bool
App:: 
getSavedSynopData(int wmono,
		  const miutil::miTime &obstime,
		  std::list<TblSynop> &tblSynop,
		  dnmi::db::Connection &con)
{
  kvDbGate       gate(&con);
  ostringstream  ost;
  
  gate.busytimeout(120);

  ost << "WHERE wmono=" << wmono << " AND obstime=\'" 
      << obstime << "\'";
  
  if(!gate.select(tblSynop, ost.str())){
    LOGERROR("DBERROR: getSavedSynopData: " << gate.getErrorStr());
    return false;
  }

  return true;
}


bool 
App::
saveSynopData(const TblSynop &tblSynop,
	      dnmi::db::Connection &con)
{
  kvDbGate       gate(&con);
  
  gate.busytimeout(120);

  if(!gate.insert(tblSynop, true)){
    LOGERROR("DBERROR: saveSynopData: " << gate.getErrorStr());
    return false;
  }

  return true;
}


bool 
App::          
getDataFrom(const miutil::miTime &t,
	    int                  wmono,
	    int                  hours,
	    dnmi::thread::CommandQue &que)
{
  LogContext lContext("getDataFrom"); 
  
  LOGINFO("Get data from server, start time: " << t );

  GetData *getData;

  try{
    getData=new GetData(*this, t, wmono, hours, que);
  }
  catch(...){
    LOGERROR("NO MEM!");
    return false;
  }
  
  try{
    //Create and start a background thread to receive the 
    //data from kvalobs.
    getData->setThread(new boost::thread(*getData));
  }
  catch(...){
    LOGERROR("NO MEM!");
    delete getData;
    return false;
  }

  mutex::scoped_lock lock(mutex);

  getDataThreads.push_back(getData);

  return true;
}


bool
App::
joinGetDataThreads(bool waitToAllIsJoined, const std::string &logid)
{
  mutex::scoped_lock lock(mutex);
  std::list<GetData*>::iterator it=getDataThreads.begin();
  bool   joined=false;

  IDLOGDEBUG(logid, "# " << getDataThreads.size() << " getDataThreads!");

  for(;it!=getDataThreads.end(); it++){
    if(waitToAllIsJoined){
      (*it)->join();
      delete *it;
      it=getDataThreads.erase(it);
      joined=true;
    }else{
      if((*it)->joinable()){
	(*it)->join();
	delete *it;
	it=getDataThreads.erase(it);
	joined=true;
      }
    }
  }

  return joined;
}


void 
App::
cacheReloaded(int wmono)
{
  mutex::scoped_lock lock(mutex);

  IStationList it=stationList.begin();

  for(;it!=stationList.end(); it++){
    if((*it)->wmono()==wmono){
      (*it)->cacheReloaded48(true);
      return;
    }
  }
}

App::StationList 
App::
reloadCache(int wmono)
{
  mutex::scoped_lock lock(mutex);
  StationList myStationList;
  
  IStationList it=stationList.begin();

  if(wmono<0){
    for(;it!=stationList.end(); it++){
      (*it)->cacheReloaded48(false);
      myStationList.push_back(*it);
    }
  }else{
    for(;it!=stationList.end(); it++){
      if((*it)->wmono()==wmono){
	(*it)->cacheReloaded48(false);
	myStationList.push_back(*it);
	break;
      }
    }
  }

  if(!myStationList.empty()){
    hasStationWaitingOnCacheReload=true;
  }
  
  return myStationList;
}

kvsynopd::ReloadList* 
App::
listCacheReload()
{
  mutex::scoped_lock lock(mutex);
  kvsynopd::ReloadList *retlist;
  
  StationList myStationList;

  try{
    retlist=new kvsynopd::ReloadList();
  }
  catch(...){
    return 0;
  }
  
  IStationList it=stationList.begin();
  
  for(;it!=stationList.end(); it++){
    if(!(*it)->cacheReloaded48()){
      myStationList.push_back(*it);
    }
  }

  if(!myStationList.empty()){
    int  count;
    retlist->length(myStationList.size());

    it=myStationList.begin();
    
    for(CORBA::Long i=0; it!=myStationList.end(); it++, i++){
      count=0;
      
      for(list<ObsEvent*>::iterator eit=obsEventWaitingOnCacheReload.begin();
	  eit!=obsEventWaitingOnCacheReload.end(); 
	  eit++){
	if((*eit)->stationInfo()->wmono()==(*it)->wmono())
	  count++;
      }
 
      (*retlist)[i].wmono=(*it)->wmono();
      (*retlist)[i].eventsWaiting=count;
    }
  } 
  
  return retlist;
}


void 
App::
addObsEvent(ObsEvent *event,
	    dnmi::thread::CommandQue &que)
{
  mutex::scoped_lock lock(mutex);

  if(!hasStationWaitingOnCacheReload){
    try{
      que.postAndBrodcast(event);
    }
    catch(...){
      delete event;
    }

    return;
  }
  
  for(IStationList it=stationList.begin();
      it!=stationList.end(); 
      it++){
    if(event->stationInfo()->wmono()==(*it)->wmono()){
      if((*it)->cacheReloaded48()){
      	//The cache for the station is reloaded post the event to 
      	//the event que and return.
      	try{
      	  que.postAndBrodcast(event);
      	}
      	catch(...){
          delete event;
        }
        return;
      }else{
        //Break out of the loop and add the event to
        //the obsEventWaitingOnCacheReload.
        break;
      }
    }
  }
  
  for(std::list<ObsEvent*>::iterator it=obsEventWaitingOnCacheReload.begin();
      it!=obsEventWaitingOnCacheReload.end(); 
      it++){
    if((*it)->obstime()==event->obstime() &&
       (*it)->stationInfo()->wmono()==event->stationInfo()->wmono()){
      delete *it;
      *it=event;
      return;
    }
  }
  
  obsEventWaitingOnCacheReload.push_back(event);
}


void 
App::
checkObsEventWaitingOnCacheReload(dnmi::thread::CommandQue &que,
				  const std::string &logid)
{
  mutex::scoped_lock lock(mutex);
  
  IDLOGDEBUG(logid, "CheckObsEventWaitingOnCacheReload called!");

  if(!hasStationWaitingOnCacheReload){
    IDLOGDEBUG(logid, "No Station waiting on reload!!");
    return;
  }
  
  for(std::list<ObsEvent*>::iterator it=obsEventWaitingOnCacheReload.begin();
      it!=obsEventWaitingOnCacheReload.end(); 
      it++){
    
    for(IStationList sit=stationList.begin();
	sit!=stationList.end(); 
	sit++){
      if((*it)->stationInfo()->wmono()==(*sit)->wmono()){
	if((*sit)->cacheReloaded48()){
	  IDLOGINFO(logid,"The cache is reloaded for station: " << 
		    (*sit)->wmono() << " obstime: "  << (*it)->obstime());
	 

	  ObsEvent *event=*it;
	  it=obsEventWaitingOnCacheReload.erase(it);
	  //The cache for the station is reloaded post the event to 
	  //the event que.
	  try{
	    que.postAndBrodcast(event);
	  }
	  catch(...){
	    LOGERROR("Cant post event to the eventque for the station: " <<
		     (*sit)->wmono());
	    IDLOGERROR(logid,
		       "Cant post event to the eventque for the station: " <<
		       (*sit)->wmono());
	    
	    delete event;
	  }
	}

	//Break out of the loop.
	break;
      }
    }
  }
  
  //If the list obsEventWaitingOnCacheReload is empty
  //check the stationList to see if all stations is reloaded. If so
  //set hasStationWaitingOnCacheReload to false.
  if(obsEventWaitingOnCacheReload.empty()){
    bool allReloaded=true;
    ostringstream ost;

    for(IStationList it=stationList.begin();
	it!=stationList.end(); 
	it++){
      if(!(*it)->cacheReloaded48()){
	ost << (*it)->wmono() << " ";
	allReloaded=false;
      }
    }

    if(!allReloaded){
      IDLOGDEBUG(logid, "This stations is not reloaded with new data:" << endl
		 << ost.str());
    }else{
      IDLOGINFO(logid, "All stations reloaded with data from kvalobs.");
      hasStationWaitingOnCacheReload=false;
    }
  }
}
