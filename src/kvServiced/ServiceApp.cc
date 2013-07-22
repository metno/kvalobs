/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceApp.cc,v 1.2.2.3 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <kvskel/commonStationInfo.hh>
#include <miutil/timeconvert.h>
#include <milog/milog.h>
#include <puTools/miTime.h>
#include <kvalobs/kvPath.h>
#include "ServiceApp.h"
#include "ReaperBase.h"

namespace{
	volatile sig_atomic_t sigTerm=0;
	void     sig_term(int);
	void     setSigHandlers();
};

using namespace std;
using namespace dnmi::db;
using namespace milog;

ServiceApp::
ServiceApp(int argn, char **argv, 
	   const std::string &driver_,
	   const std::string &connect_,
	   const char *opt[][2])
  :KvApp(argn, argv, opt), dbDriver(driver_), dbConnect(connect_), 
   shutdown_(false), orbIsDown(false), subscribers( *this )
{
  LogContext context("ApplicationInit");

  string driver(kvPath("pkglibdir")+"/db/"+dbDriver);
  
  setSigHandlers();

  LOGINFO("Loading driver for database engine <" << driver << ">!\n");

  if(!dbMgr.loadDriver(driver, dbDriverId)){
    LOGFATAL("Can't load driver <" << driver << endl 
	     << dbMgr.getErr() << endl 
	     << "Check if the driver is in the directory $KVALOBS/lib/db???");

    exit(1);
  }
  
  if( setAppNameForDb && !appName.empty() )
     dbMgr.setAppName( appName );


  LOGINFO("Driver <" << dbDriverId<< "> loaded!\n");
  
  
}

ServiceApp::
~ServiceApp()
{
}


bool
ServiceApp::
createGlobalLogger( std::string &id, milog::LogLevel ll)
{
   try{

      if( ll == milog::NOTSET )
         ll = milog::DEBUG;

       if( LogManager::hasLogger(id) )
         return true;

      FLogStream *logs=new FLogStream(2, 204800); //200k
      std::ostringstream ost;

      if( id == "__##kvManagerd@@very_secret_hash:-)##__" )
         id = "kvManagerd";

      ost << kvPath("logdir") << "/kvService/" << id << ".log";

      if(logs->open(ost.str())){
         logs->loglevel( ll );
         if(!LogManager::createLogger(id, logs)){
            LOGERROR("Failed to create logger <" << id << ">!");
            delete logs;
            id.erase();
            return false;
         }
         return true;
      }else{
         id.erase();
         LOGERROR("Cant open the logfile <" << ost.str() << ">!");
         delete logs;
         return false;
      }
   }
   catch(...){
      LOGERROR("Cant create a logstream for LOGID " << id);
      id.erase();
      return false;
   }
}


bool 
ServiceApp::
isOk()const
{
  //For now!
  return KvApp::isOk();
}


kvalobs::kvStation 
ServiceApp::
lookUpStation(const std::string &kvQuerie)
{
  return kvalobs::kvStation();
}


dnmi::db::Connection*
ServiceApp::
getNewDbConnection()
{
  Connection *con;
  
  con=dbMgr.connect(dbDriverId, dbConnect);

  if(!con){
    LOGERROR( "Can't create a database connection  (" 
	           << dbDriverId << ")" << endl << "Connect string: <" 
	           << dbConnect << ">!");
    return 0;
  }

  LOGINFO("New database connection (" << dbDriverId 
       << ") created!");
  return con;
}

void                 
ServiceApp::
releaseDbConnection(dnmi::db::Connection *con)
{
  dbMgr.releaseConnection(con);
}


bool 
ServiceApp::
shutdown() 
{ 
  
  if(shutdown_ || sigTerm){

    boost::mutex::scoped_lock l(mutex);
    
    if(!orbIsDown){
      LOGDEBUG("shutdown CORBA!\n");
      orbIsDown=true;
      subscribers.sendKvHintDown();
      getOrb()->shutdown(false);
    }
  }
  
  return shutdown_ || sigTerm;
}

void
ServiceApp::
notifyAllKvHintSubscribers()
{
  LOGINFO("Sending KvHintUp to all subscribers that is registred!");
  
  subscribers.sendKvHintUp();
}

bool
ServiceApp::
sendToManager(kvalobs::kvStationInfoList &retList,
	      CKvalObs::CManager::CheckedInput_ptr callback)
{
  kvalobs::IkvStationInfoList it;
  CKvalObs::StationInfoList   stInfo;
  CORBA::Long                 k;
  
  if(retList.empty())
    return true;
  
  stInfo.length(retList.size());
  it=retList.begin();
  
  for(k=0;it!=retList.end(); it++, k++){
    stInfo[k].stationId=it->stationID();
    stInfo[k].obstime=CORBA::string_dup(to_kvalobs_string(it->obstime()).c_str());
    stInfo[k].typeId_=it->typeID();
  }
  
  if(!CORBA::is_nil(callback)){
    try{
      callback->checkedData(stInfo);
      return true;
    }
    catch(CORBA::TRANSIENT &ex){
      LOGERROR("WARNING:(sendToManager) Exception CORBA::TRANSIENT!\n");
    }
    catch(CORBA::COMM_FAILURE &ex){
      LOGERROR("WARNING:(sendToManager) Exception CORBA::COMM_FAILURE!\n");
    }
    catch(...){
      LOGERROR("WARNING:(sendToManager) Exception unknown!\n");
    }
  }
  
  return false;    
}

bool 
ServiceApp::
isMaxClientReached()
{
	boost::mutex::scoped_lock l(reaperMutex);
	
	LOGDEBUG("isMaxClientReached: # : " << reaperObjList.size() << " max: " << MAX_CLIENTS );
	if( reaperObjList.size() > MAX_CLIENTS) {
		LOGWARN("Max number of clients reached (" << MAX_CLIENTS <<").");
		return true;
	}
		
	return false;
}
		

void 
ServiceApp::
addReaperObj(ReaperBase *rb)
{
  boost::mutex::scoped_lock l(reaperMutex);
  //rb->_add_ref();
  rb->addRef();
  reaperObjList.push_back(rb);
}


void 
ServiceApp::
removeReaperObj(ReaperBase *rb)
{
  	boost::mutex::scoped_lock l(reaperMutex);
  	
  	std::list<ReaperBase*>::iterator it=reaperObjList.begin();
	
  	for(;it!=reaperObjList.end(); it++){
  		if(*it==rb){
  			reaperObjList.erase(it);
  			//rb->_remove_ref();
  			rb->removeRef();
  			LOGDEBUG("removeReaperObj: An ReaperObj is removed from the reaperList!");
  			return;
  		}
  	}
  			
   LOGDEBUG("removeReaperObj: No ReaperObj found in the reaperList! (This must be an error!)");
}

void 
ServiceApp::
cleanUpReaperObj()
{
  	//const int TIMEOUT=900; //15 minutes
  	const int TIMEOUT=60; //1 minutes
  	time_t lastAccess;
  	time_t now;
  	bool running;

  	time(&now);

  	boost::mutex::scoped_lock l(reaperMutex);
  	std::list<ReaperBase*>::iterator it=reaperObjList.begin();

  	//LOGDEBUG("cleanUpReaperObj running!");

  	while(it!=reaperObjList.end()){
  		running=(*it)->isRunning(lastAccess);
  		LOGDEBUG( "cleanUpReaperObj: active: " << ((*it)->isActive()?"true":"false") 
  					 << " running: " << (running?"true":"false") 
  					 << " lastAccess: " << miutil::miTime(lastAccess) 
  		    		 << " timedif: " << now -lastAccess                );
  		    
    	if( !(*it)->isActive() ) {
    		//We remove this deactivated object from the list.
    		//It has been deactivated by the client.
    		if( (*it)->isTimedout() ) {
    			LOGWARN("COLLECT: A ReaperBase object has timedout.!");
    		} else{
    			LOGINFO("COLLECT: A client has disconected a ReaperBase object!");
    		}
    		//(*it)->_remove_ref();
    		ReaperBase *obj = *it;
    		int n = reaperObjList.size();
    		it = reaperObjList.erase(it);
    		LOGDEBUG("COLLECT before # : " << n << " after # : " << reaperObjList.size() << " max: " << MAX_CLIENTS );
    		obj->cleanUp();
    		obj->removeRef();
    	} else if( !(*it)->isRunning(lastAccess) ){
      	//Check if the object has timedout and we
      	//garbage collect it. We assume that the client has crashed
      	//or has forgotten about us. This is a bug in the client.

      	if((now-lastAccess)>=TIMEOUT){
				LOGWARN("TIMEOUT, a ReaperBase object is scheduled for garbage collection!");
				//Only deactivate the object here. It is removed from
				//reaperObjList the next time around.
				(*it)->deactivate();
				(*it)->setTimedout();
      	}else{
				it++;
      	}
    	}else{
      	it++;
    	}
  	}
  	
}

namespace{

void
setSigHandlers()
{
  sigset_t     oldmask;
  struct sigaction act, oldact;
  
 
  act.sa_handler=sig_term;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  
  if(sigaction(SIGTERM, &act, &oldact)<0){
      LOGFATAL("ERROR: Can't install signal handler for SIGTERM\n");
      exit(1);
  }
  
  act.sa_handler=sig_term;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  
  if(sigaction(SIGINT, &act, &oldact)<0){
    LOGFATAL("ERROR: Can't install signal handler for SIGINT\n");
    exit(1);
  }
}

void 
sig_term(int i)
{
  //CERR("sig_term("<<i<<"): called!\n");
    sigTerm=1;
}
};
