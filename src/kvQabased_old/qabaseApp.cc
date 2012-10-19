/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: qabaseApp.cc,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "qabaseApp.h"
#include <milog/milog.h>
#include <string>
#include <kvalobs/kvPath.h>

namespace{
  volatile sig_atomic_t sigTerm=0;
  void     sig_term(int);
  void     setSigHandlers();
};

using namespace std;
using namespace dnmi::db;
using namespace milog;

QaBaseApp::QaBaseApp(int argn, char **argv,
		     const std::string &driver_,
		     const std::string &connect_,
		     const char *opt[][2])
  :KvApp(argn, argv, opt), dbConnect(connect_), dbDriver(driver_),
   refManager(CKvalObs::CManager::CheckedInput::_nil()),
   shutdown_(false), orbIsDown(false)
{
  string driver(kvPath("pkglibdir")+"/db/"+dbDriver);

  setSigHandlers();

  LOGINFO("Loading driver for database engine <" << driver << ">!\n");

  if(!dbMgr.loadDriver(driver, dbDriverId)){
    LOGERROR("Can't load driver <" << driver << endl
	     << dbMgr.getErr() << endl
    	     << "Check if the driver is in the directory $KVALOBS/lib/db???");

    exit(1);
  }

  LOGINFO("Driver <" << dbDriverId<< "> loaded!\n");

}

QaBaseApp::~QaBaseApp()
{
}

bool
QaBaseApp::isOk()const
{
  //TODO:
  //For now!
  return KvApp::isOk();
}

/*
 *lookUpManager will either return the refMgr or look up kvManagerInput'
 *in the CORBA nameservice.
 */
CKvalObs::CManager::CheckedInput_ptr
QaBaseApp::lookUpManager(bool forceNS, bool &usedNS)
{
  CORBA::Object_var obj;
  CKvalObs::CManager::CheckedInput_ptr ptr;

  usedNS=false;

  while(true){
    if(forceNS){
      usedNS=true;

      obj=getRefInNS("kvManagerCheckedInput");

      if(CORBA::is_nil(obj))
	throw LookUpException("EXCEPTION: Can't obtain a reference for 'kvManagerCheckedInput'\n           from the CORBA nameserver!");

      ptr=CKvalObs::CManager::CheckedInput::_narrow(obj);

      if(CORBA::is_nil(ptr))
	throw LookUpException("EXCEPTION: Can't narrow reference for 'kvManagerCheckedInput'!");

      refManager=ptr;

      return ptr;
    }

    if(CORBA::is_nil(refManager))
      forceNS=true;
    else
      return refManager;
  }
}


bool
QaBaseApp::sendToManager(kvalobs::kvStationInfoList &retList,
			 CKvalObs::CManager::CheckedInput_ptr callback)
{
  kvalobs::IkvStationInfoList it;
  CKvalObs::StationInfoList   stInfo;
//  CORBA::Long                 i;
  CORBA::Long                 k;
//  CKvalObs::CManager::CheckedInput_ptr mgr;
//  bool forceNS=false;
//  bool usedNS=false;

  if(retList.empty()){
    LOGDEBUG("SendToManager: No data!");
    return true;
  }

  stInfo.length(retList.size());
  it=retList.begin();

  for(k=0;it!=retList.end(); it++, k++){
    stInfo[k].stationId=it->stationID();
    stInfo[k].obstime=CORBA::string_dup(it->obstime().isoTime().c_str());
    stInfo[k].typeId_=it->typeID();
  }


  //The callback may or may not represent the manager. It can be a callback
  //to a process that comunicate directly with the qabase, ex. the regression
  //test system.

  if(CORBA::is_nil(callback)){
    LOGDEBUG("WARNING: (SendToManager): No callback!");
    return false;
  }

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

  return false;
}



dnmi::db::Connection*
QaBaseApp::getNewDbConnection()
{
  Connection *con;

  con=dbMgr.connect(dbDriverId, dbConnect);

  if(!con){
    LOGERROR("Can't create a database connection  ("
	     << dbDriverId << ")" << endl
	     << "Connect string: <" << dbConnect << ">!");
    return 0;
  }

  LOGINFO("New database connection (" << dbDriverId
       << ") created!");
  return con;
}

void
QaBaseApp::releaseDbConnection(dnmi::db::Connection *con)
{
  dbMgr.releaseConnection(con);
  LOGINFO("Database connection (" << dbDriverId
	  << ") released");
}


bool
QaBaseApp::shutdown()
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


namespace{

void
setSigHandlers()
{
//  sigset_t     oldmask;
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
