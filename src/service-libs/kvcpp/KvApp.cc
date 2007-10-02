/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvApp.cc,v 1.9.2.2 2007/09/27 09:02:44 paule Exp $                                                       

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h> 
#include <string.h>
#include <stdlib.h>
#include <string>
//#include <boost/thread/thread.hpp>
//#include <boost/shared_ptr.hpp>
#include <milog/milog.h>
#include <fstream>
#include "KvApp.h"
#include "KvCorbaApp.h"
#include "kvCorbaThread.h"
#include <miutil/trimstr.h>

using namespace CKvalObs::CService;
using namespace std;
using namespace kvservice;
using namespace milog;

namespace{
  FLogStream   *fs=0;
  StdErrStream *trace=0;

  LogLevel getLogLevel(const char *str);
  bool     setLoglevel(const string &ll, const string &tl);
  volatile sig_atomic_t sigTerm=0;
  void sig_term(int);
  void setSigHandlers();
  bool terminateFunc();

  omni_mutex mutex;
  
}


//KvCorbaThread must be created first, and deleted last. Because
//the other threads are depending on CORBA.

kvservice::KvApp *kvservice::KvApp::kvApp =0;


kvservice::KvApp::
KvApp(int &argn, char **argv,
      miutil::conf::ConfSection *conf,
      const char *options[][2])
  : corbaThread_(0), shutdown_(false) 
{

  if(!conf){
    LOGFATAL("KvApp: No configuration information is given!!!"<<endl);
    exit(1);
  }

  if(!kvApp)
    kvApp=this;

  setSigHandlers();
  
  try{
    corbaThread_   = new priv::KvCorbaThread(argn, 
					     argv,
					     conf,
					     options);
  }
  catch(...){
    LOGFATAL("FATAL: failed to initialize KVALOBS service interface!!"<<endl);
    exit(1);
  }
  
  while(!corbaThread_->isInitialized())
    sleep(1);
  
  dataInput = CKvalObs::CDataSource::Data::_nil();
}


kvservice::KvApp::
~KvApp()
{
    doShutdown();
}

bool 
kvservice::KvApp::
shutdown()const
{
  omni_mutex_lock lock(mutex);

  return shutdown_ || sigTerm;
}

void
kvservice::KvApp:: 
doShutdown()
{
  omni_mutex_lock lock(mutex);

  priv::KvCorbaApp *capp;
  
  shutdown_=true;
  sigTerm=1;

  if(!corbaThread_){
    LOGDEBUG1("The CORBA subsystem is allready shutdown!" << endl);
    return;
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  //Unsubscribe from kvalobs
  capp->unsubscribeAll();
  
  CorbaHelper::CorbaApp::getCorbaApp()->getOrb()->shutdown(true);
  corbaThread_->join(0);
  //delete corbaThread_; //This cause a segmentation fault
  corbaThread_=0;
  LOGDEBUG1("AFTER: join" << endl);
}

void
kvservice::KvApp::
run()
{
  while(!shutdown())
    sleep(1);
}
   

miutil::conf::ConfSection* 
kvservice::KvApp::
readConf(const std::string &fname)
{
  miutil::conf::ConfParser  parser;
  miutil::conf::ConfSection *conf;
  ifstream    fis;
  
  fis.open(fname.c_str());

  if(!fis){
    LOGERROR("Cant open the configuration file <" << fname << ">!" << endl);
  }else{
      LOGINFO("Reading configuration from file <" << fname << ">!" << endl);
      conf=parser.parse(fis);
      
      if(!conf){
	LOGERROR("Error while reading configuration file: <" << fname 
		 << ">!" << endl << parser.getError() << endl);
      }else{
	LOGINFO("Configuration file loaded!\n");
	return conf;
      }
    }

    return 0;
}


PortableServer::POA_ptr        
kvservice::KvApp::
getPoa()const
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!");
    return PortableServer::POA::_nil();
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());

  return capp->getPoa();
}

CORBA::ORB_ptr    
kvservice::KvApp::             
getOrb()const
{
  priv::KvCorbaApp *capp;
  
  omni_mutex_lock lock(mutex);
  
  if(!corbaThread_){
    LOGDEBUG1("The CORBA subsystem is shutdown!!!"<<endl);
    return CORBA::ORB::_nil();
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  return capp->getOrb();
}

PortableServer::POAManager_ptr 
kvservice::KvApp::
getPoaMgr()const
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);
    
  if(!corbaThread_){
    LOGDEBUG1("The CORBA subsystem is shutdown!!!"<<endl);
    return PortableServer::POAManager::_nil();
  }
   
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  return capp->getPoaMgr();
}

bool   
kvservice::KvApp::
putObjInNS(CORBA::Object_ptr objref, 
	   const std::string &name)
{
  priv::KvCorbaApp *capp;
  std::string name_(name);
  
  miutil::trimstr(name_);

  if(name_.empty()){
    LOGWARN("kvservice::KvApp::putObjFromNS: name empty!"<<endl);
    return CORBA::Object::_nil();
  }

  if(name_[0]!='/'){
    std::string myKvServer=kvpathInCorbaNameserver();
    miutil::trimstr(myKvServer);

    if(myKvServer.empty()){
      LOGWARN("kvservice::KvApp::putObjFromNS: No <kvserver> is given!"<<endl);
    }else if(myKvServer[myKvServer.length()-1]!='/'){
      myKvServer+="/";
    }
   
    name_.insert(0, myKvServer);
  }

  
  omni_mutex_lock lock(mutex);
  
  if(!corbaThread_){
    LOGDEBUG("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  return capp->putObjInNS(objref, name_);
}

CORBA::Object_ptr 
kvservice::KvApp::
getObjFromNS(const std::string &name)
{
  priv::KvCorbaApp *capp;
  std::string name_(name);
  
  miutil::trimstr(name_);

  if(name_.empty()){
    LOGWARN("kvservice::KvApp::getObjFromNS: name empty!"<<endl);
    return CORBA::Object::_nil();
  }

  if(name_[0]!='/'){
    std::string myKvServer=kvpathInCorbaNameserver();
    miutil::trimstr(myKvServer);

    if(myKvServer.empty()){
      LOGWARN("kvservice::KvApp::getObjFromNS: No <kvserver> is given!"<<endl);
    }else if(myKvServer[myKvServer.length()-1]!='/'){
      myKvServer+="/";
    }
   
    name_.insert(0, myKvServer);
  }

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return CORBA::Object::_nil();
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());

  
  return capp->getObjFromNS(name_);
}

std::string 
kvservice::KvApp::
corbaRef(CORBA::Object_ptr ptr)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);
  
  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return "";
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  return capp->corbaRef(ptr);
   
}

CORBA::Object_ptr 
kvservice::KvApp::
corbaRef(const std::string &ref)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return CORBA::Object::_nil();
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  return capp->corbaRef(ref);
  
}


std::string  
kvservice::KvApp::
corbanameserver()const
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return "";
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  return capp->corbanameserver();

} 

std::string  
kvservice::KvApp::
kvpathInCorbaNameserver()const
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return "";
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  return capp->kvpathInCorbaNameserver();

}

    
std::string
kvservice::KvApp::
subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
		    dnmi::thread::CommandQue &que)
{
  priv::KvCorbaApp *capp;
  std::string         id;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return string();
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  LOGDEBUG1("KvApp::subscribeDataNotify: called ... !\n");
  
  id=capp->subscribeDataNotify(info, que);
  
  if(!id.empty()){
    LOGINFO("Subscribed to <DataNotify> with id: " << id <<endl);
  }else{
    LOGERROR("Cant subscribe to <DataNotify>!"<<endl);
  }
  
  return id;
}

std::string 
kvservice::KvApp::
subscribeData(const KvDataSubscribeInfoHelper &info,
	      dnmi::thread::CommandQue &que)
{
  priv::KvCorbaApp *capp;
  std::string         id;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return string();
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  LOGDEBUG6("KvApp::subscribeData: called ... !\n");
  
  id=capp->subscribeData(info, que);
  
  if(!id.empty()){
    LOGINFO("Subscribed to <Data> with id: " << id <<endl );
  }else{
    LOGERROR("Cant subscribe to <Data>!"<<endl);
  }
  
  return id;

}

/**
 * \return subscriberid on success and a empty string on failure.
 */  
std::string 
kvservice::KvApp::
subscribeKvHint(dnmi::thread::CommandQue &que)
{
  priv::KvCorbaApp *capp;
  std::string         id;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return string();
  }


  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  LOGDEBUG6("KvApp::subscribeDataNotify: called ... !\n"<<endl);
  
  id=capp->subscribeKvHint(que);
  
  if(!id.empty()){
    LOGINFO("Subscribed to <Hint> with id: " << id<<endl);
  }else{
    LOGERROR("Cant subscribe to <Hint>!"<<endl);
  }
  
  return id;
}



void 
kvservice::KvApp::
unsubscribe(const std::string &id)
{
  priv::KvCorbaApp *capp;
  
  omni_mutex_lock lock(mutex);
  
  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return;
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  capp->unsubscribe(id);
}


bool 
kvservice::KvApp::
getKvData(KvGetDataReceiver &dataReceiver,
	  const WhichDataHelper &wd)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);
  
  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvData(dataReceiver, wd, terminateFunc);
}

bool 
kvservice::KvApp::
getKvData(KvObsDataList &dataList,
	  const WhichDataHelper &wd)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);
  
  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvData(dataList, wd, terminateFunc);
}

bool
kvservice::KvApp::
connectToKvInput(bool reConnect) {

  if ( reConnect || CORBA::is_nil(dataInput) ) {

    LOGDEBUG("Looking up CORBA Data object"<<endl);

    dataInput = CKvalObs::CDataSource::Data::
      _narrow(this->getObjFromNS("kvinput"));

    if ( CORBA::is_nil(dataInput) )
      return false;
  }
  return true;
}

const CKvalObs::CDataSource::Result_var
kvservice::KvApp::
sendDataToKv(const char *data, const char *obsType)
{
  static omni_mutex me;

  me.lock(); // REMEMBER TO UNLOCK BEFORE LEAVING FUNCTION!

  if ( ! connectToKvInput() ) {
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = "Unable to look up Data Input Daemon.";

    me.unlock();
    return r;
  }

  try {
    CKvalObs::CDataSource::Result_var ret = dataInput->newData(data, obsType);

    me.unlock();
    return ret;
  }
  catch(CORBA::TRANSIENT &ex){
    LOGWARN("WARNING:sendDataToKv:  Exception CORBA::TRANSIENT!\n"<<endl);
  }
  catch(CORBA::COMM_FAILURE &ex){
    LOGWARN("WARNING:sendDataToKv: Exception CORBA::COMM_FAILURE!\n"<<endl);
  }
  catch(...){
    LOGWARN("WARNING:sendDataToKv: Exception unknown!\n");
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = "Unable to connect to Data Input Daemon.";
 
    me.unlock();
    return r;
  }

  // No success when trying to connect to kvinput. Force a new lookup
  // in nameserver, and try again:

  if ( ! connectToKvInput(true) ) {
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = "Unable to look up Data Input Daemon.";

    me.unlock();
    return r;
  }

  try{
    CKvalObs::CDataSource::Result_var ret = dataInput->newData(data, obsType);

    me.unlock();
    return ret;
  }
  catch(...){
    //The referance we got from the nameserver was not referancing
    //a running kvinput.
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = "Unable to look up Data Input Daemon.";

    me.unlock();
    return r;
  }
}


bool
kvservice::KvApp::
getKvRejectDecode( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, 
		   RejectDecodeIterator &it )
{
  priv::KvCorbaApp *capp = 
    static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  it.cleanup();
  return capp->getKvRejectdecode( decodeInfo, it.getCorbaObjPtr().out() );  
}

bool 
kvservice::KvApp::
getKvParams(std::list<kvalobs::kvParam> &paramList)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvParams(paramList);
}

bool 
kvservice::KvApp::
getKvStations(std::list<kvalobs::kvStation> &stationList)
{
  priv::KvCorbaApp *capp;
  
  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvStations(stationList);
}

bool 
kvservice::KvApp::
getKvModelData(std::list<kvalobs::kvModelData> &dataList,
	       const WhichDataHelper &wd)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvModelData(dataList, wd);
}

bool
kvservice::KvApp::
getKvReferenceStations(int stationid, 
		       int paramsetid, 
		       std::list<kvalobs::kvReferenceStation> &refList)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvReferenceStations(stationid, paramsetid, refList);
}

bool 
kvservice::KvApp::
getKvTypes(std::list<kvalobs::kvTypes> &typeList)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvTypes(typeList);
}


bool 
kvservice::KvApp::
getKvOperator(std::list<kvalobs::kvOperator> &operatorList)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvOperator(operatorList);
}

bool
kvservice::KvApp::
getKvStationParam( std::list<kvalobs::kvStationParam> &stParam,
		   int stationid, int paramid, int day )
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }

  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvStationParam(stParam, stationid, paramid, day );
}

bool
kvservice::KvApp::
getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
	    const std::list<long> &stationList,
	    bool aUnion)
{
  priv::KvCorbaApp *capp;

  omni_mutex_lock lock(mutex);

  if(!corbaThread_){
    LOGERROR("The CORBA subsystem is shutdown!!!"<<endl);
    return false;
  }
  
  capp=static_cast<priv::KvCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvObsPgm(obsPgm, stationList, aUnion);
}

namespace{
  void 
  sig_term(int)
  {
    sigTerm=1;
  }
  
  bool 
  terminateFunc()
  {
    return sigTerm==1;
  }
  
  void
  setSigHandlers()
  {
    sigset_t         oldmask;
    struct sigaction act, oldact;
    
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGTERM, &act, &oldact)<0){
      LOGFATAL("Can't install signal handler for SIGTERM\n");
      exit(1);
    }
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGINT, &act, &oldact)<0){
      LOGFATAL("Can't install signal handler for SIGTERM\n");
      exit(1);
    }
  }
  
  bool
  setLoglevel(const std::string &ll, const std::string &tl)
  {
    LogLevel loglevel;
    
    if(!ll.empty()){
      loglevel=getLogLevel(ll.c_str());
      
      if(loglevel==milog::NOTSET){
	return false;
      }
      
      fs->loglevel(loglevel);
    }
  
    if(!tl.empty()){
      loglevel=getLogLevel(tl.c_str());
      
      if(loglevel==milog::NOTSET){
	return false;
      }
      
      trace->loglevel(loglevel);
    }

    return true;
  }


  LogLevel getLogLevel(const char *str)
  {
    if(strcmp("FATAL", str)==0){
      return milog::FATAL;
    }else if(strcmp("ERROR", str)==0){
      return milog::ERROR;
    }else if(strcmp("WARN", str)==0){
      return milog::WARN;
    }else if(strcmp("DEBUG", str)==0){
      return milog::DEBUG;
    }else if(strcmp("DEBU1", str)==0){
      return milog::DEBUG1;
    }else if(strcmp("DEBUG2", str)==0){
      return milog::DEBUG2;
    }else if(strcmp("DEBUG3", str)==0){
      return milog::DEBUG3;
    }else if(strcmp("DEBUG4", str)==0){
      return milog::DEBUG4;
    }else if(strcmp("DEBUG5", str)==0){
      return milog::DEBUG5;
    }else if(strcmp("DEBUG6", str)==0){
      return milog::DEBUG6;
    }else if(strcmp("INFO", str)==0){
      return milog::INFO;
    }else if(strcmp("0", str)==0){
      return milog::FATAL;
    }else if(strcmp("1", str)==0){
      return milog::ERROR;
    }else if(strcmp("2", str)==0){
      return milog::WARN;
    }else if(strcmp("3", str)==0){
      return milog::INFO;
    }else if(strcmp("4", str)==0){
      return milog::DEBUG;
    }else if(strcmp("5", str)==0){
      return milog::DEBUG1;
    }else if(strcmp("6", str)==0){
      return milog::DEBUG2;
    }else if(strcmp("7", str)==0){
      return milog::DEBUG3;
    }else if(strcmp("8", str)==0){
      return milog::DEBUG4;
    }else if(strcmp("9", str)==0){
      return milog::DEBUG5;
    }else if(strcmp("10", str)==0){
      return milog::DEBUG6;
    }else{
      return milog::NOTSET;
    }
  }
}

