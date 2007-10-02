/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvCorbaApp.cc,v 1.14.2.4 2007/09/27 09:02:44 paule Exp $                                                       

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
#include <milog/milog.h>
#include <kvskel/kvService.hh>
#include <miconfparser/miconfparser.h>
#include "KvCorbaApp.h"

#include <typeinfo>

using namespace CKvalObs::CService;
using namespace std;
using namespace miutil;
using namespace kvalobs;
using namespace miutil::conf;

kvservice::priv::KvCorbaApp::
KvCorbaApp(int argn, 
	   char **argv, 
	   miutil::conf::ConfSection *conf,
	   const char *options[][2])
  :CorbaHelper::CorbaApp(argn, argv, options), refService(kvService::_nil())
{
  ValElementList valElem;
  string         val;

  valElem=conf->getValue("corba.nameserver");

  if(valElem.empty()){
    LOGFATAL("No nameserver <corba.nameserver> in the configurationfile!");
    exit(1);
  }

  nameserver=valElem[0].valAsString();

  if(nameserver.empty()){
    LOGFATAL("The key <corba.nameserver> in the configurationfile has no value!");
    exit(1);
  }

  setNameservice(nameserver);

  LOGINFO("Using corba nameserver at <" << nameserver << ">!");


  valElem=conf->getValue("corba.path");

  if(valElem.empty()){
    LOGFATAL("No path <corba.path> in the configurationfile!");
    exit(1);
  }

  nameserverpath=valElem[0].valAsString();

  if(nameserverpath.empty()){
    LOGFATAL("The key <corba.path> in the configurationfile has no value!");
    exit(1);
  }

  if(*nameserverpath.rbegin()!='/')
    nameserverpath.append("/");

  if(*nameserverpath.begin()!='/')
    nameserverpath.insert(0, "/");

  LOGINFO("Using kvalobs in path <" 
	  << nameserver << "::" << nameserverpath << ">!");

  
}

kvservice::priv::KvCorbaApp::
~KvCorbaApp()
{
}
	
CKvalObs::CService::kvService_ptr 
kvservice::priv::KvCorbaApp::
lookUpManager(bool forceNS, bool &usedNS)
{
    CORBA::Object_var                 obj;
    CKvalObs::CService::kvService_ptr ptr; 
    std::string path(nameserverpath+"kvService");
    usedNS=false;

    omni_mutex_lock lock(mutex);
    
    while(true){
	if(forceNS){
	    usedNS=true;
	    
	    obj=getObjFromNS(path);
	    
	    if(CORBA::is_nil(obj))
		throw LookUpException("EXCEPTION: Can't obtain a reference for 'kvService'\n           from the CORBA nameserver!");
	    
	    ptr=CKvalObs::CService::kvService::_narrow(obj);
	    
	    if(CORBA::is_nil(ptr))
		throw LookUpException("EXCEPTION: Can't narrow reference for 'kvService'!");
	    
	    refService=CKvalObs::CService::kvService::_duplicate(ptr);
	    
	    return ptr;
	}
	
	if(CORBA::is_nil(refService))
	    forceNS=true;
	else
	  return CKvalObs::CService::kvService::_duplicate(refService);
    }

}


bool                     
kvservice::priv::KvCorbaApp::
addDataNotifySubscriber(DataNotifySubscriber *ptr, 
			const std::string &subid)
{
  omni_mutex_lock lock(mutex);

  IDataNotifyList it=dataNotifySubs.find(subid);

  if(it!=dataNotifySubs.end()){
    LOGERROR("DUPLICATE subid <DataNotifySubscriber>!");
    return false;
  }

  dataNotifySubs[subid]=ptr;
  
  return true;
}
      
bool 
kvservice::priv::KvCorbaApp::
addDataSubscriber(DataSubscriber *ptr, 
		  const std::string    &subid)
{
  omni_mutex_lock lock(mutex);

  IDataList it=dataSubs.find(subid);

  if(it!=dataSubs.end()){
    LOGERROR("DUPLICATE subid <DataSubscriber>!");
    return false;
  }

  dataSubs[subid]=ptr;
  
  return true;

}

bool 
kvservice::priv::KvCorbaApp::
addHintSubscriber(HintSubscriber *ptr,
		    const std::string &subid)
{
  omni_mutex_lock lock(mutex);
  IHintList it=hintSubs.find(subid);

  if(it!=hintSubs.end()){
    LOGERROR("DUPLICATE subid <HintSubscriber>!");
    return false;
  }

  hintSubs[subid]=ptr;
  
  return true;
}



std::string
kvservice::priv::KvCorbaApp::
subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
		    dnmi::thread::CommandQue &que)
{
  DataNotifySubscriber       *sub;  
  kvDataNotifySubscriber_var refSub;  
  kvService_ptr     service; 
  CORBA::String_var ret;
  bool              forceNS=false;
  bool              usedNS;
  int               sensor;
    
  try{
    sub= new DataNotifySubscriber(que);
  }
  catch(...){
    LOGERROR("NOMEM: cant allocate space for <DataNotifySubscriber>!");
    return string();
  }
    
  try{
    PortableServer::ObjectId *id;
    id=getPoa()->activate_object(sub);
    sub->setId(id);
  }
  catch(...){
    LOGERROR("Cant activate <DataNotifySubscriber>!");
    delete sub;
    return string();
  }

  try{
    refSub=sub->_this();
  }
  catch(...){
    LOGERROR("Cant obtain a referance to <DataNotifySubscriber>!");
    getPoa()->deactivate_object(sub->getId());
    sub->_remove_ref();
    return string();
  }
    
  try{
    for(int i=0; i<2; i++){
      service=lookUpManager(forceNS, usedNS);
      
      try{
	ret=service->subscribeDataNotify(*info.getDataSubscribeInfo(), 
					 refSub);
	if(strlen(static_cast<char*>(ret))==0){
	  LOGERROR("WARNING: subscribeDataNotify returned a failindicator!\n");

	  getPoa()->deactivate_object(sub->getId());
	  sub->_remove_ref();
	  return string();
	}
	
	addDataNotifySubscriber(sub, string(static_cast<char *>(ret)));
	return std::string(static_cast<char *>(ret));
      }
      catch(CORBA::TRANSIENT &ex){
	LOGINFO("WARNING: Exception CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGINFO("WARNING: Exception CORBA::COMM_FAILURE!\n");
      }
      catch(...){
	LOGINFO("WARNING: Exception unknown!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
      
      if(usedNS){
	LOGINFO("WARNING: cant subscribe to <DataNotify>!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
	    
      forceNS=true;
    }
  }
  catch(LookUpException &ex){
	LOGINFO("WARNING: KvQtCorbaApp::subscribeDataNotify: " << ex.what() << endl);
  }
  catch(...){
    LOGINFO("WARNING: KvQtCorbaApp::subscribeDataNotify: hmmm, very strange, a unkown exception!\n");
  }

  getPoa()->deactivate_object(sub->getId());
  sub->_remove_ref();
  return string();
	
}


std::string 
kvservice::priv::KvCorbaApp::
subscribeData(const KvDataSubscribeInfoHelper &info,
	      dnmi::thread::CommandQue &que)
{
  DataSubscriber       *sub;  
  kvDataSubscriber_var refSub;  
  kvService_ptr     service; 
  CORBA::String_var ret;
  bool              forceNS=false;
  bool              usedNS;
  int               sensor;
  
  try{
    sub= new DataSubscriber(que);
  }
  catch(...){
    LOGERROR("NOMEM: cant allocate space for <DataSubscriber>!");
    return string();
  }
    
  try{
    PortableServer::ObjectId  *id;

    if(CORBA::is_nil(getPoa())){
      LOGFATAL("getPoa()==NULL");
      return string();
    }

    id=getPoa()->activate_object(sub);
    sub->setId(id);
  }
  catch(...){
    LOGERROR("Cant activate <DataNotifySubscriber>!");
    delete sub;
    return string();
  }

  try{
    refSub=sub->_this();
  }
  catch(...){
    LOGERROR("Cant obtain a referance to <DataNotifySubscriber>!");
    getPoa()->deactivate_object(sub->getId());
    sub->_remove_ref();
    return string();
  }
       
  try{
    for(int i=0; i<2; i++){
      service=lookUpManager(forceNS, usedNS);
      
      try{
	ret=service->subscribeData(*info.getDataSubscribeInfo(), 
				   refSub);
	if(strlen(static_cast<char*>(ret))==0){
	  CERR("WARNING: subscribeData returned with a failindicator!\n");
	  getPoa()->deactivate_object(sub->getId());
	  sub->_remove_ref();
	  return string();
	}
		
	addDataSubscriber(sub, string(static_cast<char *>(ret)));
	
	return std::string(static_cast<char *>(ret));
      }
      catch(CORBA::TRANSIENT &ex){
	LOGINFO("EXCEPTION: CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGINFO("EXCEPTION: CORBA::COMM_FAILURE!\n");
      }
      catch(...){
	LOGINFO("EXCEPTION: Exception unknown!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	  
	return string();
      }
      
      if(usedNS){
	LOGINFO("WARNING: cant subscribe <data>!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
	    
      forceNS=true;
    }
  }
  catch(LookUpException &ex){
    LOGINFO("LOOKUPEXCEPTION: " << ex.what() << endl);
  }
  catch(...){
    LOGINFO("UNKNOWNEXCEPTION: hmmm, very strange, an unkown exception!\n");
    
  }
  
  getPoa()->deactivate_object(sub->getId());
  sub->_remove_ref();
  
  return string();
}

std::string 
kvservice::priv::
KvCorbaApp::subscribeKvHint(dnmi::thread::CommandQue &que)
{
  HintSubscriber       *sub;  
  kvHintSubscriber_var refSub;  
  kvService_ptr     service; 
  CORBA::String_var ret;
  bool              forceNS=false;
  bool              usedNS;
  int               sensor;

  try{
    sub= new HintSubscriber(que);
  }
  catch(...){
    LOGERROR("NOMEM: cant allocate space for <HintSubscriber>!");
    return string();
  }
    
  try{
    PortableServer::ObjectId *id;
    id=getPoa()->activate_object(sub);
    sub->setId(id);
  }
  catch(...){
    LOGERROR("Cant activate <HineSubscriber>!");
    delete sub;
    return string();
  }

  try{
    refSub=sub->_this();
  }
  catch(...){
    LOGERROR("Cant obtain a referance to <HintSubscriber>!");
    getPoa()->deactivate_object(sub->getId());
    sub->_remove_ref();
    return string();
  }
  
    
  try{
    for(int i=0; i<2; i++){
      service=lookUpManager(forceNS, usedNS);
      
      try{
	ret=service->subscribeKvHint(refSub);

	if(strlen(static_cast<char*>(ret))==0){
	  LOGINFO("WARNING: subscribeKvHint returned with a failindicator!\n");
	  getPoa()->deactivate_object(sub->getId());
	  sub->_remove_ref();
	  return string();
	}

	addHintSubscriber(sub, string(static_cast<char *>(ret)));
	
	return std::string(static_cast<char*>(ret));
      }
      catch(CORBA::TRANSIENT &ex){
	LOGINFO("EXCEPTION: CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGINFO("EXCEPTION: CORBA::COMM_FAILURE!\n");
      }
      catch(...){
	LOGINFO("EXCEPTION: unknown!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
      
      if(usedNS){
	LOGINFO("WARNING: cant subscribe <kvHint>!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
      
      forceNS=true;
    }
  }
  catch(LookUpException &ex){
    LOGINFO("LOKUPEXCEPTION: " << ex.what() << endl);
  }
  catch(...){
    LOGINFO("EXCEPTION: unknown exception!\n");
  } 

  getPoa()->deactivate_object(sub->getId());
  sub->_remove_ref();
  return string();
}



void 
kvservice::priv::
KvCorbaApp::unsubscribe(const std::string &id)
{
  {
    IDataNotifyList it=dataNotifySubs.find(id);
    
    if(it!=dataNotifySubs.end()){
      unsubscribe_(id);
      getPoa()->deactivate_object(it->second->getId());
      it->second->_remove_ref();

      omni_mutex_lock lock(mutex);
      dataNotifySubs.erase(it);
      return;
    }
  }
  {
    IDataList it=dataSubs.find(id); 
 
    if(it!=dataSubs.end()){
      unsubscribe_(id);
      getPoa()->deactivate_object(it->second->getId());
      it->second->_remove_ref();

      omni_mutex_lock lock(mutex);
      dataSubs.erase(it);
      return;
    }
  }

  {
    IHintList it=hintSubs.find(id); 
    
    if(it!=hintSubs.end()){
      unsubscribe_(id);
      getPoa()->deactivate_object(it->second->getId());
      it->second->_remove_ref();
      
      omni_mutex_lock lock(mutex);
      hintSubs.erase(it);
      return;
    }
  }
}
  
	

void 
kvservice::priv::KvCorbaApp::
unsubscribeAll()
{
  {
    IDataNotifyList it=dataNotifySubs.begin();
    
    for(;it!=dataNotifySubs.end(); it++){
      unsubscribe_(it->first);
      getPoa()->deactivate_object(it->second->getId());
      it->second->_remove_ref();

      omni_mutex_lock lock(mutex);
      dataNotifySubs.erase(it);
      return;
    }
  }
  {
    IDataList it=dataSubs.begin(); 
 
    for(;it!=dataSubs.end(); it++){
      unsubscribe_(it->first);
      getPoa()->deactivate_object(it->second->getId());
      it->second->_remove_ref();

      omni_mutex_lock lock(mutex);
      dataSubs.erase(it);
      return;
    }
  }

  {
    IHintList it=hintSubs.begin(); 
    
    for(;it!=hintSubs.end(); it++){
      unsubscribe_(it->first);
      getPoa()->deactivate_object(it->second->getId());
      it->second->_remove_ref();

      omni_mutex_lock lock(mutex);
      hintSubs.erase(it);
      return;
    }
  }
}



void
kvservice::priv::KvCorbaApp::
unsubscribe_(const std::string &id)
{
    kvService_ptr service; 
    SubscribeId ret;
    bool        forceNS=false;
    bool        usedNS;
    
    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		service->unsubscribe(id.c_str());
		return;
	    }
	    catch(CORBA::TRANSIENT &ex){
		LOGINFO("WARNING:(KvQtCorbaApp::unsubscribe) Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		LOGINFO("WARNING:(KvQtCorbaApp::unsubscribe) Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		LOGINFO("WARNING:(KvQtCorbaApp::unsubscribe) Exception unknown!\n");
		return;
	    }
	    
	    if(usedNS){
		LOGINFO("WARNING: cant unsubscribe from kvManagerInput!\n");
		return;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	LOGINFO("WARNING: KvQtCorbaApp::subscribeDataNotify: " << ex.what() << endl);
    }
    catch(...){
	LOGINFO("WARNING: KvQtCorbaApp::subscribeDataNotify: hmmm, very strange, a unkown exception!\n");
    }
}

bool 
kvservice::priv::KvCorbaApp::
getRejectdecode( const RejectDecodeInfo &decodeInfo, 
		 RejectedIterator_ptr &it )
{
  kvService_ptr    service; 
  bool             forceNS=false;
  bool             usedNS;

  try{
    for(int i=0; i<2; i++){
      service=lookUpManager(forceNS, usedNS);

      try{
	if ( ! service->getRejectdecode( decodeInfo, it ) ) {
	  LOGINFO("WARNING: cant get rejected list from kvalobs!\n");
	  return false;
	}	
	return true;
      }
      catch(CORBA::TRANSIENT &ex){
	LOGINFO("WARNING:getRejectDecode:  Exception CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGINFO("WARNING:getRejectDecode: Exception CORBA::COMM_FAILURE!\n");
      }
      catch( CORBA::Exception &e ){
	LOGINFO( "WARNING:getRejectDecode: Unhandled CORBA exception: " 
		  << typeid( e ).name() );
	return false;
      }
      catch(...) {
	LOGINFO("WARNING:getRejectDecode: Exception unknown!\n");
	return false;
      }
      
      if(usedNS){
	LOGINFO("WARNING:getRejectDecode: cant connect to kvalobs!\n");
	return false;
      }
      
      forceNS=true;
    }
  }
  catch(LookUpException &ex){
    LOGINFO("WARNING:getRejectDecode: can't lookup CORBA nameserver\n" 
	    "Reason: " << ex.what() << endl);
	return false;
  }
  catch(...){
    LOGINFO("WARNING:getRejectDecode: can't lookup CORBA nameserver\n" <<
	    "Reason:  unkown!\n");
    return false;
  }
  return false;  
}

bool
kvservice::priv::KvCorbaApp::
getParams(CKvalObs::CService::ParamList *& params )
{
    CKvalObs::CService::ParamList *params_;
    kvService_ptr    service; 
    bool             forceNS=false;
    bool             usedNS;


    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		if(!service->getParams(params_)){
		    LOGINFO("WARNING: cant get param data from kvalobs!\n");
		    return false;
		}
		params=params_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		LOGINFO("WARNING:getParams:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		LOGINFO("WARNING:getParams: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		LOGINFO("WARNING:getParams: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		LOGINFO("WARNING:getParams: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	LOGINFO("WARNING:getParams: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      LOGINFO("WARNING:getParams: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}


bool
kvservice::priv::KvCorbaApp:: 
getTypes(CKvalObs::CService::TypeList *& types)
{
    CKvalObs::CService::TypeList *types_;
    kvService_ptr    service; 
    bool             forceNS=false;
    bool             usedNS;


    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		if(!service->getTypes(types_)){
		    LOGINFO("WARNING: cant get <type> data from kvalobs!\n");
		    return false;
		}
		types=types_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		LOGINFO("WARNING:getTypes:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		LOGINFO("WARNING:getTypes: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		LOGINFO("WARNING:getTypes: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		LOGINFO("WARNING:getTypes: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	LOGINFO("WARNING:getTypes: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      LOGINFO("WARNING:getTypes: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}



bool
kvservice::priv::KvCorbaApp:: 
getOperator(CKvalObs::CService::OperatorList *& operators)
{
    CKvalObs::CService::OperatorList *operators_;
    kvService_ptr    service; 
    bool             forceNS=false;
    bool             usedNS;


    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		if(!service->getOperator(operators_)){
		    LOGINFO("WARNING: cant get <operator> data from kvalobs!\n");
		    return false;
		}
		operators=operators_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		LOGINFO("WARNING:getOperator:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		LOGINFO("WARNING:getOperator: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		LOGINFO("WARNING:getOperator: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		LOGINFO("WARNING:getOperator: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	LOGINFO("WARNING:getOperator: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      LOGINFO("WARNING:getOperator: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}


bool
kvservice::priv::KvCorbaApp:: 
getObsPgm(CKvalObs::CService::Obs_pgmList *& obsPgm,
	  const std::list<long> &stationList,
	  bool aUnion)
{
  CKvalObs::CService::Obs_pgmList *Obs_pgmList_;
  CKvalObs::CService::StationIDList idList;
  kvService_ptr    service; 
  bool             forceNS=false;
  bool             usedNS;
  
  idList.length(stationList.size());
  
  std::list<long>::const_iterator it=stationList.begin();
  
  for(CORBA::Long ii=0; it!=stationList.end(); it++){
    idList[ii]=*it;
  }

  try{
    for(int i=0; i<2; i++){
      service=lookUpManager(forceNS, usedNS);
      
      try{
	if(!service->getObsPgm(Obs_pgmList_, idList, aUnion)){
	  LOGINFO("WARNING: cant get <ObsPgm> data from kvalobs!\n");
	  return false;
	}
	obsPgm=Obs_pgmList_;

	return true;
      }
      catch(CORBA::TRANSIENT &ex){
	LOGINFO("WARNING:getObsPgm:  Exception CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGINFO("WARNING:getObsPgm: Exception CORBA::COMM_FAILURE!\n");
      }
      catch(...){
	LOGINFO("WARNING:getObsPgm: Exception unknown!\n");
	return false;
      }
      
      if(usedNS){
	LOGINFO("WARNING:getObsPgm: cant connect to kvalobs!\n");
	return false;
      }
      
      forceNS=true;
    }
  }
  catch(LookUpException &ex){
    LOGINFO("WARNING:getObsPgm: can't lookup CORBA nameserver\n" 
	    "Reason: " << ex.what() << endl);
    return false;
  }
  catch(...){
    LOGINFO("WARNING:getObsPgm: can't lookup CORBA nameserver\n" <<
	    "Reason:  unkown!\n");
    return false;
  }
  
  return false;
}

bool
kvservice::priv::KvCorbaApp:: 
getStationParam( CKvalObs::CService::Station_paramList *& stParam, 
		 int stationid, int paramid , int day )
{
  CKvalObs::CService::Station_paramList *stParam_;
  kvService_ptr    service; 
  bool             forceNS=false;
  bool             usedNS;
  
  
  try{
    for(int i=0; i<2; i++){
      service=lookUpManager(forceNS, usedNS);
      
      try{
	if( ! service->getStationParam( stParam_, stationid, paramid, day )){
	  CERR("WARNING: cant get <operator> data from kvalobs!\n");
	  return false;
	}
	stParam=stParam_;
	
	return true;
      }
      catch(CORBA::TRANSIENT &ex){
	CERR("WARNING:getOperator:  Exception CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	CERR("WARNING:getOperator: Exception CORBA::COMM_FAILURE!\n");
      }
      catch(...){
	CERR("WARNING:getOperator: Exception unknown!\n");
	return false;
      }
      
      if(usedNS){
	CERR("WARNING:getOperator: cant connect to kvalobs!\n");
	return false;
      }
      
      forceNS=true;
    }
  }
  catch(LookUpException &ex){
    CERR("WARNING:getOperator: can't lookup CORBA nameserver\n" 
	 "Reason: " << ex.what() << endl);
    return false;
  }
  catch(...){
    CERR("WARNING:getOperator: can't lookup CORBA nameserver\n" <<
	 "Reason:  unkown!\n");
    return false;
  }
  
  return false;
}



bool 
kvservice::priv::KvCorbaApp::
getStations(CKvalObs::CService::StationList *& stations)
{
    CKvalObs::CService::StationList *stations_;
    kvService_ptr    service; 
    bool             forceNS=false;
    bool             usedNS;


    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		if(!service->getStations(stations_)){
		    LOGINFO("WARNING: cant get station data from kvalobs!\n");
		    return false;
		}
		stations=stations_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		LOGINFO("WARNING:getStations:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		LOGINFO("WARNING:getStations: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		LOGINFO("WARNING:getStations: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		LOGINFO("WARNING:getStations: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	LOGINFO("WARNING:getStations: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      LOGINFO("WARNING:getStations: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}


bool 
kvservice::priv::KvCorbaApp::
getReferenceStations(CKvalObs::CService::Reference_stationList *&ref,
		     long stationid, int paramsetid)
{
  CKvalObs::CService::Reference_stationList *ref_;
  kvService_ptr                             service; 
  bool                                      forceNS=false;
  bool                                      usedNS;


    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		if(!service->getReferenceStation(stationid, paramsetid, ref_)){
		    LOGINFO("WARNING:getReferenceStation: cant get reference_station from kvalobs!\n");
		    return false;
		}
		ref=ref_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		LOGINFO("WARNING:getReferenceStation:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		LOGINFO("WARNING:getReferenceStation: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		LOGINFO("WARNING:getReferenceStation: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		LOGINFO("WARNING:getReferenceStation: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	LOGINFO("WARNING:getReferenceStation: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      LOGINFO("WARNING:getReferenceStation: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}

CKvalObs::CService::DataIterator_ptr
kvservice::priv::KvCorbaApp::
getDataIter(const WhichDataHelper &wd)
{
    DataIterator_ptr it;
    kvService_ptr    service; 
    bool             forceNS=false;
    bool             usedNS;


    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		if(!service->getData(*wd.whichData(), it)){
		    CERR("WARNING:getDataIter: cant get data from kvalobs!\n");
		    return DataIterator::_nil();
		}else
		  return it;
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:getDataIter:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:getDataIter: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:getDataIter: Exception unknown!\n");
		return DataIterator::_nil();
	    }
	    
	    if(usedNS){
		CERR("WARNING:getDataIter: cant connect to kvalobs!\n");
		return DataIterator::_nil();
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	LOGINFO("WARNING:getDataIter: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return DataIterator::_nil();
    }
    catch(...){
      LOGINFO("WARNING:getDataIter: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return DataIterator::_nil();
    }

    return it;
    
}

CKvalObs::CService::ModelDataIterator_ptr 
kvservice::priv::KvCorbaApp::
getModelDataIter(const WhichDataHelper &wd)
{
    ModelDataIterator_ptr it;
    kvService_ptr    service; 
    bool             forceNS=false;
    bool             usedNS;


    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		if(!service->getModelData(*wd.whichData(), it)){
		    LOGINFO("WARNING: cant get data from kvalobs!\n");
		    return ModelDataIterator::_nil();
		}else
		  return it;
	    }
	    catch(CORBA::TRANSIENT &ex){
		LOGINFO("WARNING:getModelDataIter:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		LOGINFO("WARNING:getModelDataItera: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		LOGINFO("WARNING:getModelDataIter: Exception unknown!\n");
		return ModelDataIterator::_nil();
	    }
	    
	    if(usedNS){
		LOGINFO("WARNING:getModelDataIter: cant connect to kvalobs!\n");
		return ModelDataIterator::_nil();
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	LOGINFO("WARNING:getModelDataIter: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return ModelDataIterator::_nil();
    }
    catch(...){
      LOGINFO("WARNING:getModelDataIter: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return ModelDataIterator::_nil();
    }

    return it;

}

bool 
kvservice::priv::KvCorbaApp::
getKvData(KvGetDataReceiver &dataReceiver,
	  const WhichDataHelper &wd,
	  termfunc terminate)
{
    DataIterator_var dataIt=getDataIter(wd);     
    ObsDataList_var  data;
    bool             err=false;
    KvObsDataList    dataList;
    bool             cont=true;

    if(CORBA::is_nil(dataIt))
        return false;
  
    try{
        while(cont && !terminate() && dataIt->next(data)){
            for(CORBA::ULong i=0; i<data->length(); i++){
               dataList.push_back( KvObsData( data[i] ) );
            }
            
            try{
                cont=dataReceiver.next(dataList);
            }
            catch(...){
                LOGDEBUG1("Exception: KvCorbaApp::getKvData on call to receiver!");
            }
            
            dataList.clear();
        }
    
    }
    catch(...){
        LOGDEBUG1("Exception: In KvCorbaApp::getKvData!");
        err=true;
    }
  
    try{
        dataIt->destroy();
    }
    catch(...){
        LOGINFO("EXCEPTION: Cant destroy DataIterator!!!!");
    }
  
    return !err;
}      



bool 
kvservice::priv::KvCorbaApp::getKvData(KvObsDataList &dataList,
				       const WhichDataHelper &wd,
				       termfunc terminate)
{
    DataIterator_var dataIt=getDataIter(wd);     
    ObsDataList_var  data;
    KvDataList       tmpDataList;
    int              sensor;
    bool             err=false;
    
    
    if(CORBA::is_nil(dataIt))
	return false;

    dataList.clear();
    
    try{
	while(dataIt->next(data) && !terminate()){
	    for(CORBA::ULong i=0; i<data->length(); i++){
	      /*
	      tmpDataList.clear();
	      for(CORBA::ULong k=0; k<data[i].dataList.length(); k++){
		if(strlen(data[i].dataList[k].sensor)>0)
		  sensor=*data[i].dataList[k].sensor;
		else
		  sensor=0;
		
		kvalobs::kvData myData(
			data[i].dataList[k].stationID,
			miutil::miTime(data[i].dataList[k].obstime),
			data[i].dataList[k].original,
			data[i].dataList[k].paramID,
			miutil::miTime(data[i].dataList[k].tbtime),
			data[i].dataList[k].typeID_,
			sensor,
			data[i].dataList[k].level,
			data[i].dataList[k].corrected,
			kvalobs::kvDataFlag(string(data[i].dataList[k].controlinfo)),
			kvalobs::kvDataFlag(string(data[i].dataList[k].useinfo)),
			string(data[i].dataList[k].cfailed));
		    
		tmpDataList.push_back(myData);
	      }
	      dataList.push_back(tmpDataList);
	      */
	      dataList.push_back( KvObsData( data[i] ) ); 
	    }
	}
	
    }
    catch(...){
      err=true;
    }
    
    try{
      dataIt->destroy();
    }
    catch(...){
      LOGINFO("EXCEPTION: Cant destroy DataIterator!!!!");
    }
    
    return !err;
}

bool 
kvservice::priv::KvCorbaApp::
getKvRejectdecode( const RejectDecodeInfo &decodeInfo, 
		   RejectedIterator_ptr &it )
{
  return getRejectdecode( decodeInfo, it );
}



bool 
kvservice::priv::KvCorbaApp::getKvParams(
    std::list<kvalobs::kvParam> &paramList
    )
{
    CKvalObs::CService::ParamList *params;

    if(!getParams(params))
	return false;

    paramList.clear();

    for(CORBA::Long i=0; i<params->length(); i++){
	paramList.push_back(kvParam((*params)[i].paramID,
				    miString((*params)[i].name),
				    miString((*params)[i].description),
				    miString((*params)[i].unit),
				    (*params)[i].level_scale,
				    miString((*params)[i].comment)));
	
    }
    
    return true;
}



bool 
kvservice::priv::KvCorbaApp::
getKvStations(std::list<kvalobs::kvStation> &stationList)
{
    CKvalObs::CService::StationList *stations;

    
    if(!getStations(stations)){
	return false;
    }

    stationList.clear();

    for(CORBA::Long i=0; i<stations->length(); i++){
	kvStation station((*stations)[i].stationID,
			  (*stations)[i].lat,
			  (*stations)[i].lon,
			  (*stations)[i].height,
			  (*stations)[i].maxspeed,
			  miString((*stations)[i].name),
			  (*stations)[i].wmonr,
			  (*stations)[i].nationalnr,
			  miString((*stations)[i].ICAOid),
			  miString((*stations)[i].call_sign),
			  miString((*stations)[i].stationstr),
			  (*stations)[i].environmentid,
			  (*stations)[i].static_,
			  miTime(miString((*stations)[i].fromtime)));

	stationList.push_back(station);
    }

    return true;
    
}



bool 
kvservice::priv::KvCorbaApp::
getKvModelData(std::list<kvalobs::kvModelData> &dataList,
	       const WhichDataHelper &wd)
{
    ModelDataIterator_var dataIt=getModelDataIter(wd);     
    ModelDataList_var  data;
    int              sensor;
    bool             err=false;
    
    
    if(CORBA::is_nil(dataIt))
	return false;

    dataList.clear();
    
    try{
	while(dataIt->next(data)){
	    for(CORBA::ULong i=0; i<data->length(); i++){
		for(CORBA::ULong k=0; k<data[i].dataList.length(); k++){
		  kvalobs::kvModelData myData(
			data[i].dataList[k].stationID,
			miutil::miTime(data[i].dataList[k].obstime),
			data[i].dataList[k].paramID,
			data[i].dataList[k].level,
			data[i].dataList[k].modelID,
			data[i].dataList[k].original);
				    
		  dataList.push_back(myData);
		  
		}
	    }
	}
	
    }
    catch(...){
	err=true;
    }
    
    try{
	dataIt->destroy();
    }
    catch(...){
    }
    
    return !err;

} 


bool 
kvservice::priv::KvCorbaApp::
getKvReferenceStations(int stationid, 
		       int paramsetid, 
		       std::list<kvalobs::kvReferenceStation> &refList)
{
  CKvalObs::CService::Reference_stationList *refStations;

    if(!getReferenceStations(refStations, stationid, paramsetid))
      return false;

    refList.clear();

    for(CORBA::Long i=0; i<refStations->length(); i++){
	refList.push_back(
		 kvReferenceStation((*refStations)[i].stationID,
				    (*refStations)[i].paramsetID,
				    miString((*refStations)[i].reference)
				    )
		 );
	
    }
    
    return true;
} 


bool 
kvservice::priv::KvCorbaApp::
getKvTypes(std::list<kvalobs::kvTypes> &typeList)
{
    CKvalObs::CService::TypeList *types;
    
    if(!getTypes(types)){
	return false;
    }

    typeList.clear();

    for(CORBA::Long i=0; i<types->length(); i++){
	kvTypes type((*types)[i].typeID_,
		     miString((*types)[i].format),
		     (*types)[i].earlyobs,
		     (*types)[i].lateobs,
		     miString((*types)[i].read),
		     miString((*types)[i].obspgm),
		     miString((*types)[i].comment));

	typeList.push_back(type);
    }

    return true;
}

bool 
kvservice::priv::KvCorbaApp::
getKvOperator(std::list<kvalobs::kvOperator> &operatorList)
{
   CKvalObs::CService::OperatorList *operators;
    
    if(!getOperator(operators)){
	return false;
    }

    operatorList.clear();

    for(CORBA::Long i=0; i<operators->length(); i++){
      kvOperator oper(miString(
			 static_cast<const char*>((*operators)[i].username)
			 ),
		      (*operators)[i].userid
		      );

	operatorList.push_back(oper);
    }

    return true;
}

bool 
kvservice::priv::KvCorbaApp::
getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgmList,
	    const std::list<long> &stationList,
	    bool aUnion)
{
  CKvalObs::CService::Obs_pgmList *obspgm;

    
    if(!getObsPgm(obspgm, stationList, aUnion)){
	return false;
    }

    obsPgmList.clear();

    for(CORBA::Long i=0; i<obspgm->length(); i++){
	kvObsPgm obsp((*obspgm)[i].stationID,
		      (*obspgm)[i].paramID,
		      (*obspgm)[i].level,
		      (*obspgm)[i].nr_sensor,
		      (*obspgm)[i].typeID_,
		      (*obspgm)[i].collector,
		      (*obspgm)[i].kl00,
		      (*obspgm)[i].kl01,
		      (*obspgm)[i].kl02,
		      (*obspgm)[i].kl03,
		      (*obspgm)[i].kl04,
		      (*obspgm)[i].kl05,
		      (*obspgm)[i].kl06,
		      (*obspgm)[i].kl07,
		      (*obspgm)[i].kl08,
		      (*obspgm)[i].kl09,
		      (*obspgm)[i].kl10,
		      (*obspgm)[i].kl11,
		      (*obspgm)[i].kl12,
		      (*obspgm)[i].kl13,
		      (*obspgm)[i].kl14,
		      (*obspgm)[i].kl15,
		      (*obspgm)[i].kl16,
		      (*obspgm)[i].kl17,
		      (*obspgm)[i].kl18,
		      (*obspgm)[i].kl19,
		      (*obspgm)[i].kl20,
		      (*obspgm)[i].kl21,
		      (*obspgm)[i].kl22,
		      (*obspgm)[i].kl23,
		      (*obspgm)[i].mon,
		      (*obspgm)[i].tue,
		      (*obspgm)[i].wed,
		      (*obspgm)[i].thu,
		      (*obspgm)[i].fri,
		      (*obspgm)[i].sat,
		      (*obspgm)[i].sun,
		      miutil::miTime((*obspgm)[i].fromtime)		      
		      );

	obsPgmList.push_back(obsp);
    }

    return true;

}

bool 
kvservice::priv::KvCorbaApp::
getKvStationParam( std::list<kvalobs::kvStationParam> &stParam,
		   int stationid, int paramid , int day )
{
  CKvalObs::CService::Station_paramList *stp;

  if ( !getStationParam( stp, stationid, paramid, day ) )
    return false;

  for ( CORBA::Long i = 0; i < stp->length(); i++ ) {
    stParam.push_back( kvalobs::kvStationParam( (*stp)[i].stationid,
						(*stp)[i].paramid,
						(*stp)[i].level,
						((char*)(*stp)[i].sensor)[0] - '0', 
						(*stp)[i].fromday,
						(*stp)[i].today,
						(*stp)[i].hour,
						miString( (*stp)[i].qcx ),
						miString( (*stp)[i].metadata ),
						miString( (*stp)[i].desc_metadata ),
						miTime( (*stp)[i].fromtime )
						)
		       );
  }
  return true;
}
