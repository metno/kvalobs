/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceApp.cc,v 1.1.2.3 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvPsSubscribers.h>
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
   shutdown_(false), orbIsDown(false)
{
  LogContext context("ApplicationInit");

  string driver(getKvalobsPath()+"lib/db/"+dbDriver);
  
  setSigHandlers();

  LOGINFO("Loading driver for database engine <" << driver << ">!\n");

  if(!dbMgr.loadDriver(driver, dbDriverId)){
    LOGFATAL("Can't load driver <" << driver << endl 
	     << dbMgr.getErr() << endl 
	     << "Check if the driver is in the directory $KVALOBS/lib/db???");

    exit(1);
  }
  
  LOGINFO("Driver <" << dbDriverId<< "> loaded!\n");

  subscribers.readAllSubscribersFromFile(*this);  
}

ServiceApp
::~ServiceApp()
{
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
lookUpStation(const miutil::std::string &kvQuerie)
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
    LOGERROR("Can't create a database connection  (" 
	     << dbDriverId << ")" << endl << "Connect string: <" << dbConnect << ">!");
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
  LOGINFO("Released a database connection (" << dbDriverId 
       << ")!");
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
notifyAllKvHintSubscribers(bool kvup)
{
	if(kvup){	
	  	LOGINFO("Sending KvHintUp to all subscribers that is registred!");
  		subscribers.sendKvHintUp();
	}else{
		LOGINFO("Sending KvHintDown to all subscribers that is registred!");
  		subscribers.sendKvHintDown();
	}
	
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
    stInfo[k].obstime=CORBA::string_dup(it->obstime().isoTime().c_str());
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
	
	if(reaperObjList.size()>MAX_CLIENTS)
		return true;
		
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
  			
   LOGDEBUG("removeReaperObj: No ReaperObj foun in the reaperList! (This must be an error???)");
}


void 
ServiceApp::
cleanUpReaperObj()
{
  	//const int TIMEOUT=900; //15 minutes
  	const int TIMEOUT=300; // 5 minutes
  	time_t lastAccess;
  	time_t now;
  	bool running;
  	ostringstream ost;

  	time(&now);

  	boost::mutex::scoped_lock l(reaperMutex);
  	std::list<ReaperBase*>::iterator it=reaperObjList.begin();

  	LOGDEBUG("cleanUpReaperObj running!");

  	while(it!=reaperObjList.end()){
		running=(*it)->isRunning(lastAccess);
  		ost << "active: " << ((*it)->isActive()?"true":"false") 
  		    << " running: " << (running?"true":"false") 
  		    << " lastAccess: " << miutil::miTime(lastAccess) 
  		    << " timedif: " << now -lastAccess ;

    	if(!(*it)->isActive()){
      	//We remove this deativated object from the list.
      	//It has been deactivated by the client.
      	LOGINFO("COLLECT: A client has disconected a ReaperBase object!");
      	//(*it)->_remove_ref();
      	(*it)->removeRef();
      	it=reaperObjList.erase(it);
      	ost << " Inactive: removed";
    	}else if(!(*it)->isRunning(lastAccess)){
      	//Check if the object has timedout and we
      	//garbage collect it. We assume that the client has crashed
      	//or has forgotten about us. This is a bug in the client.

      	if((now-lastAccess)>=TIMEOUT){
				LOGWARN("TIMEOUT, a ReaperBase object is garbage collected!");
				(*it)->deactivate();
				//(*it)->_remove_ref();
				(*it)->removeRef();
				it=reaperObjList.erase(it);
				ost << " Timout: removed";
      	}else{
				it++;
      	}
    	}else{
      	it++;
    	}
    	
    	ost << endl;
  	}
  	
  	LOGDEBUG("Reaper: " << endl << ost.str());
}


void 
ServiceApp::
loadAllPersistentSubscribers()
{
	namespace  kvskel=CKvalObs::CService;
	
	dnmi::db::Connection *con=0;
	
	while(!shutdown() && !con){
		con=getNewDbConnection();
		
		if(!con && !shutdown())
			sleep(1);
	}
	
	if(!con) //shutdown
		return;
	
	kvalobs::kvDbGate gate(con);
	std::list<kvalobs::kvPsSubscribers> psList;
	
	if(!gate.select(psList)){
		LOGERROR("DBERROR: Cant read in the PsSubscribers." 
		          << endl << gate.getErrorStr());
		return;
	}
	
	for(std::list<kvalobs::kvPsSubscribers>::iterator it=psList.begin();
		 it!=psList.end(); it++){
		if(it->subscribertype()==0){ //Data subscriber.
			KvPsDataSubscriber *sub=0;
			
			try{
				CORBA::Object_var refObj=corbaRef(it->sior()); 
				kvskel::kvDataSubscriberExt_var subRef;
				
				subRef=kvskel::kvDataSubscriberExt::_narrow(refObj);
				sub=new 	KvPsDataSubscriber(subRef, *this, true, it->subscriberid());
				
				if(CORBA::is_nil(subRef))
					sub->subscribed(false);
				
				if(!subscribers.createPsDataSubscriber(sub)){
					LOGERROR("ERROR: Cant start an ps subscriber thread for <"<<it->name() << "> (" << 
								it->subscriberid() << ").");
					delete sub; 
				}else{
					LOGINFO("Start an ps subscriber thread for <"<<it->name() << "> (" << 
								it->subscriberid() << ").");
				}
			}
			catch(...){
				LOGERROR("EXCEPTION: Cant start an ps subscriber thread for <"<<it->name() << "> (" << 
								it->subscriberid() << ").");
				delete sub; 	
			}
		}else if(it->subscribertype()==1){ //Notify subscriber
			KvPsDataNotifySubscriber *sub=0;
			
			try{
				CORBA::Object_var refObj=corbaRef(it->sior());
				kvskel::kvDataNotifySubscriberExt_var subRef;
				
				subRef=kvskel::kvDataNotifySubscriberExt::_narrow(refObj);
				sub=new 	KvPsDataNotifySubscriber(subRef, *this, true, it->subscriberid());

				if(CORBA::is_nil(subRef))
					sub->subscribed(false);
				
				if(!subscribers.createPsDataNotifySubscriber(sub)){
					LOGERROR("ERROR: Cant start an ps subscriber thread for <"<<it->name() << "> (" << 
								it->subscriberid() << ").");
					delete sub; 
				}else{
					LOGINFO("Start an ps subscriber thread for <"<<it->name() << "> (" << 
								it->subscriberid() << ").");
				}
			}
			catch(...){
				LOGERROR("EXCEPTION: Cant start an ps subscriber thread for <"<<it->name() << "> (" << 
								it->subscriberid() << ").");
				delete sub; 	
			}
		}else{
			LOGWARN("Unknown sybscribertype <" << it->subscribertype() << " name <" <<
						it->name() << ">.");
		}
	}
	
	releaseDbConnection(con);
}

void 
ServiceApp::
removeAllUndefinedSubscribers()
{
}

bool 
ServiceApp::
savePsSubscriberSIOR(dnmi::db::Connection *dbCon,
						   const std::string &subid,
						   const std::string &sior)
{
	if(!dbCon){
    	LOGERROR("INTERNAL: Database connection is 0 (dbCon==0)!");
    	return false;
  	}
  	
	string name(kvalobs::kvPsSubscribers::nameFromSubscriberid(subid));
	ostringstream ost;
	ost << " WHERE name='" << name << "'";
  	
  	kvalobs::kvDbGate gate(dbCon);
  	  	
  	list<kvalobs::kvPsSubscribers> psSubscribers;
  
  	if(!gate.select(psSubscribers, ost.str())){
    	LOGERROR("Failed to retrive the persistent subscriber <" << name << 
             	" from the database!" << gate.getErrorStr());
          
    	return false;
  	}

  	if(psSubscribers.empty()){
  		LOGWARN("The persistent subscriber <" << name << "> dont exist in the database!");
  		return false;
	}
	
	psSubscribers.begin()->sior(sior);
	
	if(!gate.update(*psSubscribers.begin())){
		LOGWARN("Failed to update the SIOR to the persistent subscriber <" << name << 
              " in the database!" << gate.getErrorStr());
      return false;
	}else{
		LOGINFO("The SIOR for the persistent subscriber <" << name << "> is updated in the database!");
	}      
	
	return true;
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
