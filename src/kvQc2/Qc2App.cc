/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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
#include "Qc2App.h"
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


Qc2App::Qc2App(int argn, char **argv,
		     const std::string &driver_,
		     const std::string &connect_,
		     const char *opt[][2])
  :KvApp(argn, argv, opt), dbConnect(connect_), dbDriver(driver_),
   shutdown_(false), orbIsDown(false)

{
  
  LOGINFO("Soup");
  
  for (int k=0;k < argn; ++k){
  	std::cout << "arg " << k << ": " << argv[k]  << std::endl;
  }
  
  string driver(kvPath("pkglibdir")+"/db/"+dbDriver);
  //string driver(getKvalobsPath()+"lib/kvdb/"+dbDriver);
  setSigHandlers();

  LOGINFO("Loading driver for database engine <" << driver << ">!\n");

  if(!dbMgr.loadDriver(driver, dbDriverId)){
    LOGERROR("Can't load driver <" << driver << endl
	     << dbMgr.getErr() << endl
    	     << "Check if the driver is in the directory $KVALOBS/lib/kvdb???");

    exit(1);
  }

  LOGINFO("Driver <" << dbDriverId<< "> loaded!\n");

}

Qc2App::~Qc2App()
{
}

bool
Qc2App::isOk()const
{
  //TODO:
  //For now!
  return KvApp::isOk();
}


dnmi::db::Connection*
Qc2App::getNewDbConnection()
{
  Connection *con;

  std::string ost;
  ost=dbConnect;
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
Qc2App::releaseDbConnection(dnmi::db::Connection *con)
{
  dbMgr.releaseConnection(con);
  LOGINFO("Database connection (" << dbDriverId
	  << ") released");
}




bool
Qc2App::
sendDataToKvService(const kvalobs::kvStationInfoList &info_,
                    bool &bussy)
{



  kvalobs::kvStationInfoList info=const_cast<kvalobs::kvStationInfoList&>(info_)
;
  //  kvalobs::CIkvParamInfoList it;
  CKvalObs::StationInfoList  stInfoList;
  CORBA::Boolean             serviceBussy;
  bool                       forceNS=false;
  bool                       usedNS=false;
  CKvalObs::CService::DataReadyInput_ptr service;

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
 if(!service->dataReady(stInfoList,
                               refServiceCheckedInput,
                               serviceBussy)){
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

  //Shall never happen!
  return false;

}


/*
 *lookUpManager will either return the refMgr or look up kvManagerInput'
 *in the CORBA nameservice.
 */
CKvalObs::CService::DataReadyInput_ptr
Qc2App::
lookUpKvService(bool forceNS, bool &usedNS)
{
  CORBA::Object_var obj;
  CKvalObs::CService::DataReadyInput_ptr ptr;

  usedNS=false;

  while(true){
    if(forceNS){
      usedNS=true;

      obj=getRefInNS("kvServiceDataReady");

      if(CORBA::is_nil(obj))
        throw LookUpException("EXCEPTION: Can't obtain a reference for 'kvServiceDataReady'\n           from the CORBA nameserver!");

      ptr=CKvalObs::CService::DataReadyInput::_narrow(obj);

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




bool
Qc2App::shutdown()
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
