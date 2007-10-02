/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQtApp.cc,v 1.16.2.4 2007/09/27 09:02:47 paule Exp $                                                       

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
#include <stdlib.h>
#include <string>
//#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <dnmithread/mtcout.h>
#include "kvQtApp.h"
#include "kvNewDataThread.h"
#include "kvCorbaThread.h"
#include "kvWhatListQue.h"
#include "kvQtEvents.h"

using namespace CKvalObs::CService;
using namespace std;



//KvQtCorbaThread must be created first, and deleted last. Because
//the other threads are depending on CORBA.

kvservice::KvQtApp *kvservice::KvQtApp::kvQApp =0;


kvservice::KvQtApp::
KvQtApp(int &argn, char **argv, 
	bool guiapp_, 
	const char *options[0][2]):
  QApplication(argn, argv, guiapp), guiapp(guiapp_),
  corbaThread_(0), corbaIsShutdown(false)
{
  if(!kvQApp)
    kvQApp=this;
  
  try{
    //whatListQue    = new priv::KvWhatListQue(); 
    corbaThread_   = new priv::KvQtCorbaThread(argn, 
					       argv, 
					       options);
    //	newDataThread_ = new priv::KvNewDataThread(*this, *whatListQue);
    //newDataThread  = new boost::thread(*newDataThread_);
  }
  catch(...){
    CERR("FATAL: failed to initialize KVALOBS service interface!!");
    exit(1);
  }
  
  //QObject::connect( qApp, SIGNAL(lastWindowClosed()), this, SLOT(onQuit()));
  
  while(!corbaThread_->isInitialized())
    sleep(1);
  
}


kvservice::KvQtApp::
~KvQtApp()
{
  //if(!guiapp)
    quit();
  
  CERR("DTOR: ~kvQtAPP is about to return\n"); 
}

std::string 
kvservice::KvQtApp::
corbaName() const
{
  return corbaThread_->getCorbaApp().kvserver();
}

    
void 
kvservice::KvQtApp::
quit()
{
  static omni_mutex mutex;
  priv::KvQtCorbaApp *capp;
    
  omni_mutex_lock lock(mutex);
  
  if(corbaIsShutdown)
    return;
 
  corbaIsShutdown=true;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  //Unsubscribe from kvalobs
  capp->unsubscribeAll();
  
  CorbaHelper::CorbaApp::getCorbaApp()->getOrb()->shutdown(true);
  corbaThread_->join(0);
  
  
  CERR("AFTER: join\n");
}

void 
kvservice::KvQtApp::
onQuit()
{
  quit();
}
  
void 
kvservice::KvQtApp::
customEvent(QCustomEvent *event)
{
  CERR("KvQtApp::customEvent: called! \n");
  
  if(event->type()==KV_DATANOTIFY_EVENT){
    CERR("KvQtApp::customEvent: KV_DATANOTIFY_EVENT! \n");
    emit kvDataNotify(static_cast<priv::KvDataNotifyEvent*>(event)->what());
  }else if(event->type()==KV_DATA_EVENT){
    CERR("KvQtApp::customEvent: KV_DATA_EVENT! \n");
    emit kvData(static_cast<priv::KvDataEvent*>(event)->data());
  }else if(event->type()==KV_HINT_EVENT){
    CERR("KvQtApp::customEvent: KV_HINT_EVENT! \n");
    priv::KvHintEvent *hint=static_cast<priv::KvHintEvent*>(event);
    
    emit kvHint(hint->upEvent());
  }
}

  
  
std::string
kvservice::KvQtApp::
subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
		    const QObject                   *receiver, 
		    const char                      *member)
{
  priv::KvQtCorbaApp *capp;
  std::string         id;
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  CERR("KvQtApp::subscribeDataNotify: called ... !\n");
  
  id=capp->subscribeDataNotify(info);
  
  if(!id.empty()){
    if(!QObject::connect( this, 
			  SIGNAL(kvDataNotify(kvservice::KvWhatListPtr)), 
			  receiver, 
			  member)){
      capp->unsubscribe(id);
      return string();
    }
    
    return id;
  }
  
  return string();
}

std::string 
kvservice::KvQtApp::
subscribeData(const KvDataSubscribeInfoHelper &info,
	      const QObject *receiver,
	      const char *member)
{
  priv::KvQtCorbaApp *capp;
  std::string         id;
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  CERR("KvQtApp::subscribeData: called ... !\n");
  
  id=capp->subscribeData(info);
  
  if(!id.empty()){
    if(!QObject::connect( this, 
			  SIGNAL(kvData(kvservice::KvObsDataListPtr)), 
			  receiver, 
			  member)){
      capp->unsubscribe(id);
      return string();
    }
    
    return id;
  }
  
  return string();
}

/**
 * \return subscriberid on success and a empty string on failure.
 */  
std::string 
kvservice::KvQtApp::
subscribeKvHint(const QObject *receiver, 
		const char *member)
{
  priv::KvQtCorbaApp *capp;
  std::string         id;
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  CERR("KvQtApp::subscribeDataNotify: called ... !\n");
  
  id=capp->subscribeKvHint();
  
  if(!id.empty()){
    if(!QObject::connect( this, 
			  SIGNAL(kvHint(bool)), 
			  receiver, 
			  member)){
      capp->unsubscribe(id);
      return string();
    }
    
    return id;
  }
  
  return string();
}



void 
kvservice::KvQtApp::
unsubscribe(const std::string &id)
{
  priv::KvQtCorbaApp *capp;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  
  capp->unsubscribe(id);
}

bool 
kvservice::KvQtApp::
getKvData(KvObsDataList &dataList,
	  const WhichDataHelper &wd)
{
  priv::KvQtCorbaApp *capp;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvData(dataList, wd);
}

bool 
kvservice::KvQtApp::
getKvParams(std::list<kvalobs::kvParam> &paramList)
{
  priv::KvQtCorbaApp *capp;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvParams(paramList);
}

bool 
kvservice::KvQtApp::
getKvStations(std::list<kvalobs::kvStation> &stationList)
{
  priv::KvQtCorbaApp *capp;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvStations(stationList);
}

bool 
kvservice::KvQtApp::
getKvModelData(std::list<kvalobs::kvModelData> &dataList,
	       const WhichDataHelper &wd)
{
  priv::KvQtCorbaApp *capp;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvModelData(dataList, wd);
}

bool
kvservice::KvQtApp::
getKvReferenceStations(int stationid, 
		       int paramsetid, 
		       std::list<kvalobs::kvReferenceStation> &refList)
{
  priv::KvQtCorbaApp *capp;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvReferenceStations(stationid, paramsetid, refList);
}

bool 
kvservice::KvQtApp::
getKvTypes(std::list<kvalobs::kvTypes> &typeList)
{
  priv::KvQtCorbaApp *capp;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvTypes(typeList);
}

bool
kvservice::KvQtApp::
getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
	    const std::list<long> &stationList,
	    bool aUnion)
{
  priv::KvQtCorbaApp *capp;

  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvObsPgm(obsPgm, stationList, aUnion);
}

bool
kvservice::KvQtApp::
getKvStationParam( std::list<kvalobs::kvStationParam> &stParam,
		   int stationid, int paramid, int day )
{
  priv::KvQtCorbaApp *capp;

  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvStationParam(stParam, stationid, paramid, day );
}
    
bool 
kvservice::KvQtApp::
getKvOperator(std::list<kvalobs::kvOperator> &operatorList)
{
  priv::KvQtCorbaApp *capp;
  
  capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
  return capp->getKvOperator(operatorList);

}

const CKvalObs::CDataSource::Result_var
kvservice::KvQtApp::
sendDataToKv(const char *data, const char *obsType)
{
 priv::KvQtCorbaApp *capp;
  
 capp=static_cast<priv::KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());
 return capp->sendDataToKv(data, obsType);
}
