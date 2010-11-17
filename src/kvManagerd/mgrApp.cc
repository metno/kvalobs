/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: mgrApp.cc,v 1.3.2.4 2007/09/27 09:02:35 paule Exp $                                                       

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
#include "mgrApp.h"
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvPath.h>


namespace{
  volatile sig_atomic_t sigTerm=0;
  void     sig_term(int);
  void     setSigHandlers();
};

using namespace std;
using namespace dnmi::db;
using namespace milog;
using namespace kvalobs;

ManagerApp::
ManagerApp(int argn, char **argv, 
	   const std::string &driver_,
	   const std::string &connect_,
	   const char *opt[][2])
  :KvApp(argn, argv, opt), dbDriver(driver_), dbConnect(connect_), 
   refQa(CKvalObs::CQaBase::QaBaseInput::_nil()),
   refCheckedInput(CKvalObs::CManager::CheckedInput::_nil()),
   shutdown_(false), orbIsDown(false), ok_(true), status_("No error!"),
   checkForMissingObs_(true)
{
  LogContext context("ApplicationInit");

  string driver( kvPath("pkglibdir")+"/db/"+dbDriver);
  
  setSigHandlers();

  LOGINFO("Loading driver for database engine <" << driver << ">!\n");

  if(!dbMgr.loadDriver(driver, dbDriverId)){
    LOGFATAL("Can't load driver <" << driver << endl 
	     << dbMgr.getErr() << endl 
	     << "Check if the driver is in the directory $KVALOBS/lib/db???");

    exit(1);
  }
  
  LOGINFO("Driver <" << dbDriverId<< "> loaded!\n");
  
  
}

ManagerApp::
~ManagerApp()
{
}

bool 
ManagerApp::
isOk()const
{
  //For now!
  return KvApp::isOk();
}


void 
ManagerApp::
setServiceCheckedInput(CKvalObs::CManager::CheckedInput_ptr ptr)
{
    refServiceCheckedInput=ptr;
}

void 
ManagerApp::
setCheckedInput(CKvalObs::CManager::CheckedInput_ptr ptr)
{
  refCheckedInput=ptr;
}


bool 
ManagerApp::
isGenerated(long stationid, 
	    long typeid_,   
	    dnmi::db::Connection *con)
{
	return genCache_.isGenerated(stationid, typeid_, con);
}

bool 
ManagerApp::
sendDataToQA(const kvalobs::kvStationInfo &info_,
			 bool &bussy)
{
  kvalobs::kvStationInfo     info=const_cast<kvalobs::kvStationInfo&>(info_);
  //  kvalobs::CIkvParamInfoList it;
  CKvalObs::StationInfo      stInfo;
  CORBA::Long i;
  bool forceNS=false;
  bool usedNS=false;
  CKvalObs::CQaBase::QaBaseInput_ptr qa;
  micutil::KeyValList        keyValList;  //Is not used by kvManagerd at the momment
  CORBA::Boolean  qaBussy;   
  bussy=false;

  if(CORBA::is_nil(refCheckedInput)){
    LOGFATAL("There is no instance of the " <<
	     "interface CKvalObs::CManager::CheckedInput. This is a BUG!!!");
    return false;
  }

  stInfo.stationId=info.stationID();
  stInfo.obstime=CORBA::string_dup(info.obstime().isoTime().c_str());
  stInfo.typeId_=info.typeID();

  try{
    for(int i=0; i<2; i++){
      qa=lookUpQabase(forceNS, usedNS);

      try{
	if(!qa->newDataCB(stInfo, refCheckedInput, keyValList, qaBussy)){
	  if(qaBussy)
	    bussy=true;
	}
	  
	return true;
      }
      catch(CORBA::TRANSIENT &ex){
	LOGWARN("(sendDataToQA) Exception CORBA::TRANSIENT!" << endl
		<< "The reason can be a network problem!");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGERROR("Can't send data to kvQaBase! Is kvQaBase running!");
      }
      catch(...){
	LOGERROR("Can't send data to kvQaBase. Reason unknown!\n");
	return false;
      }

      if(usedNS){
	LOGERROR("Can't send data to kvQabased, is kvQaBase running!\n");
	return false;
      }
      forceNS=true;

    }
  }
  catch(LookUpException &ex){
    LOGERROR("Can't contact CORBA nameservice. Is the CORBA nameservice running."
	     << endl << "(" <<ex.what() << ")" << endl);
    return false;
  }
  catch(...){
    LOGERROR("Cant send data to kvQaBase, hmmm, very strange, a unkown exception!\n");
    return false;
  }
   
  //Shall newer happend!
  return false;

}



bool 
ManagerApp::
sendDataToKvService(const kvalobs::kvStationInfoList &info_,
		    bool &bussy)
{
  kvalobs::kvStationInfoList info=const_cast<kvalobs::kvStationInfoList&>(info_);
  //  kvalobs::CIkvParamInfoList it;
  CKvalObs::StationInfoList  stInfoList;
  CORBA::Boolean             serviceBussy;
  bool                       forceNS=false;
  bool                       usedNS=false;
  CKvalObs::CService::DataReadyInputExt_ptr service;
  
  bussy=false;

  if(info.empty()){
    LOGDEBUG("No data to send to kvServiced.");
    return true;
  }

  stInfoList.length(info.size());
  kvalobs::IkvStationInfoList itInfoList=info.begin();

  for(CORBA::Long k=0; itInfoList!=info.end(); k++, itInfoList++){
    stInfoList[k].stationId=itInfoList->stationID();
    stInfoList[k].obstime=CORBA::string_dup(itInfoList->obstime().isoTime().c_str());
    stInfoList[k].typeId_=itInfoList->typeID();
  }

  try{
    for(int i=0; i<2; i++){
      service=lookUpKvService(forceNS, usedNS);

      try{
	if(!service->dataReadyExt( "__##kvManagerd@@very_secret_hash:-)##__",
	                           stInfoList,
	                           refServiceCheckedInput,
	                           serviceBussy )){
	  if(serviceBussy)
	    bussy=true;
	}

	return true;
      }
      catch(CORBA::TRANSIENT &ex){
	LOGWARN("(sendDataToKvService) Exception CORBA::TRANSIENT!" << endl
		<< "The reason can be a network problem!");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGERROR("Can't send data to kvService! Is kvService running!");
      }
      catch(...){
	LOGERROR("Can't send data to kvService. Reason unknown!\n");
	return false;
      }

      if(usedNS){
	LOGERROR("Can't send data to kvService, is kvService running!\n");
	return false;
      }
      forceNS=true;

    }
  }
  catch(LookUpException &ex){
    LOGERROR("Can't contact CORBA nameservice. Is the CORBA nameservice running."
	     << endl << "(" <<ex.what() << ")" << endl);
    return false;
  }
  catch(...){
    LOGERROR("Cant send data to kvService, hmmm, very strange, a unkown exception!\n");
    return false;
  }
   
  //Shall newer happend!
  return false;

}



/*
 *lookUpManager will either return the refMgr or look up kvManagerInput'
 *in the CORBA nameservice.
 */ 
CKvalObs::CService::DataReadyInputExt_ptr
ManagerApp::
lookUpKvService(bool forceNS, bool &usedNS)
{
  CORBA::Object_var obj;
  CKvalObs::CService::DataReadyInputExt_ptr ptr;

  usedNS=false;

  while(true){
    if(forceNS){
      usedNS=true;
      
      obj=getRefInNS("kvServiceDataReady");
      
      if(CORBA::is_nil(obj))
	throw LookUpException("EXCEPTION: Can't obtain a reference for 'kvServiceDataReady'\n           from the CORBA nameserver!");
      
      ptr=CKvalObs::CService::DataReadyInputExt::_narrow(obj);
      
      if(CORBA::is_nil(ptr))
	throw LookUpException("EXCEPTION: Can't narrow reference for 'kvServiceDataReady'!");
      
      refKvServiceDataReady=ptr;
      
      return ptr;
    }

    if(CORBA::is_nil(refKvServiceDataReady))
      forceNS=true;
    else
      return refKvServiceDataReady;
  }
}


CKvalObs::CQaBase::QaBaseInput_ptr
ManagerApp::
lookUpQabase(bool forceNS, bool &usedNS)
{
  CORBA::Object_var obj;
  CKvalObs::CQaBase::QaBaseInput_ptr ptr;

  usedNS=false;

  while(true){
    if(forceNS){
      usedNS=true;
      
      obj=getRefInNS("kvQabaseInput");
      
      if(CORBA::is_nil(obj))
	throw LookUpException("EXCEPTION: Can't obtain a reference for 'kvQabaseInput'\n           from the CORBA nameserver!");
      
      ptr=CKvalObs::CQaBase::QaBaseInput::_narrow(obj);
      
      if(CORBA::is_nil(ptr))
	throw LookUpException("EXCEPTION: Can't narrow reference for 'kvQabaseInput'!");
      
      refQa=ptr;
      
      return ptr;
    }

    if(CORBA::is_nil(refQa))
      forceNS=true;
    else
      return refQa;
  }
}





kvalobs::kvStation 
ManagerApp::
lookUpStation(const miutil::miString &kvQuerie)
{
  return kvalobs::kvStation();
}


dnmi::db::Connection*
ManagerApp::
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
ManagerApp::
releaseDbConnection(dnmi::db::Connection *con)
{
  dbMgr.releaseConnection(con);
}


bool 
ManagerApp::
shutdown() 
{ 
  
  if(shutdown_ || sigTerm){

    boost::mutex::scoped_lock l(mutex);
    
    if(!orbIsDown){
      LOGDEBUG("shutdown CORBA!\n");
      orbIsDown=true;
      getOrb()->shutdown(false);
    }
  }
  
  return shutdown_ || sigTerm;
}


void 
ManagerApp::
ok(bool f)
{ 
  boost::mutex::scoped_lock l(mutex);
  ok_=f;
}

bool 
ManagerApp::
ok()const 
{
  
  boost::mutex::scoped_lock l(const_cast<ManagerApp*>(this)->mutex); 
  return ok_;
}

std::string 
ManagerApp::
status()const
{ 
  boost::mutex::scoped_lock l(const_cast<ManagerApp*>(this)->mutex); 
  return status_;
}

void 
ManagerApp::
status(const std::string &m)
{
  boost::mutex::scoped_lock l(mutex);
  status_=m;
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
