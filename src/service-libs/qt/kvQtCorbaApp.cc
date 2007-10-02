/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQtCorbaApp.cc,v 1.23.2.3 2007/09/27 09:02:47 paule Exp $                                                       

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
#include <dnmithread/mtcout.h>
#include <kvskel/kvService.hh>
#include "kvQtCorbaApp.h"

#include <iostream>

using namespace CKvalObs::CService;
using namespace std;
using namespace miutil;
using namespace kvalobs;

kvservice::priv::
KvQtCorbaApp::KvQtCorbaApp(int argn, 
					    char **argv, 
					    const char *options[0][2])
  :KvApp(argn, argv, options),
   dataInput(CKvalObs::CDataSource::Data::_nil())
{
}

kvservice::priv::
KvQtCorbaApp::~KvQtCorbaApp()
{
  CERR("DTOR: ~KvQtCorbaApp: CORBA subsystem is shutdown.");
}
	
CKvalObs::CService::kvService_ptr 
kvservice::priv::
KvQtCorbaApp::lookUpManager(bool forceNS, bool &usedNS)
{
    CORBA::Object_var obj;
    CKvalObs::CService::kvService_ptr ptr; 
    
    usedNS=false;
    
    while(true){
	if(forceNS){
	    usedNS=true;
	    
	    obj=getRefInNS("kvService");
	    
	    if(CORBA::is_nil(obj))
		throw LookUpException("EXCEPTION: Can't obtain a reference for 'kvService'\n           from the CORBA nameserver!");
	    
	    ptr=CKvalObs::CService::kvService::_narrow(obj);
	    
	    if(CORBA::is_nil(ptr))
		throw LookUpException("EXCEPTION: Can't narrow reference for 'kvService'!");
	    
	    refManager=CKvalObs::CService::kvService::_duplicate(ptr);
	    
	    return ptr;
	}
	
	if(CORBA::is_nil(refManager))
	    forceNS=true;
	else
		    return CKvalObs::CService::kvService::_duplicate(refManager);
    }
}


CKvalObs::CService::kvDataNotifySubscriber_ptr
kvservice::priv::KvQtCorbaApp::
getDataNotifySubscriber()
{ 
    return 
	CKvalObs::CService::kvDataNotifySubscriber::_duplicate( 
	    refDataNotifySubs
	    );
}


std::string
kvservice::priv::KvQtCorbaApp::
subscribeDataNotify(const KvDataSubscribeInfoHelper &info )
{
    kvService_ptr     service; 
    CORBA::String_var ret;
    bool              forceNS=false;
    bool              usedNS;
    int               sensor;
    

    if(CORBA::is_nil(refDataNotifySubs)){
	CERR("WARNING:(KvQtCorbaApp::subscribeDataNotify): No DataNotifySubsriber (callback)!\n");
	return string();
    }
    
    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		ret=service->subscribeDataNotify(*info.getDataSubscribeInfo(), 
						 refDataNotifySubs);
		if(ret<0){
		    CERR("WARNING:(KvQtCorbaApp::subscribeDataNotify): subscribeDataNotify returned with a failindicator!\n");
		    return string();
		}
		
		subscriberid.push_back(string(ret));
		
		return std::string(ret);
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:(KvQtCorbaApp::subscribeDataNotify) Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:(KvQtCorbaApp::subscribeDataNotify) Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:(KvQtCorbaApp::subscribeDataNotify) Exception unknown!\n");
		return string();
	    }
	    
	    if(usedNS){
		CERR("WARNING: cant subscribe 'dataNotify' from kvManagerInput!\n");
		return string();
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING: KvQtCorbaApp::subscribeDataNotify: " << ex.what() << endl);
	return string();
    }
    catch(...){
	CERR("WARNING: KvQtCorbaApp::subscribeDataNotify: hmmm, very strange, a unkown exception!\n");
	return string();
    }
}


std::string 
kvservice::priv::KvQtCorbaApp::
subscribeData(const KvDataSubscribeInfoHelper &info)
{
  kvService_ptr     service; 
  CORBA::String_var ret;
  bool              forceNS=false;
  bool              usedNS;
  int               sensor;
  

    if(CORBA::is_nil(refDataSubs)){
	CERR("WARNING:(KvQtCorbaApp::subscribeData): No DataSubsriber (callback)!\n");
	return string();
    }
    
    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		ret=service->subscribeData(*info.getDataSubscribeInfo(), 
						 refDataSubs);
		if(strlen(static_cast<char*>(ret))==0){
		    CERR("WARNING:(KvQtCorbaApp::subscribeData): subscribeData returned with a failindicator!\n");
		    return string();
		}
		
		subscriberid.push_back(string(ret));
		
		return std::string(static_cast<char *>(ret));
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:(KvQtCorbaApp::subscribeData) Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:(KvQtCorbaApp::subscribeData) Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:(KvQtCorbaApp::subscribeData) Exception unknown!\n");
		return string();
	    }
	    
	    if(usedNS){
		CERR("WARNING: cant subscribe 'data' from kvManagerInput!\n");
		return string();
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING: KvQtCorbaApp::subscribeData: " << ex.what() << endl);
	return string();
    }
    catch(...){
	CERR("WARNING: KvQtCorbaApp::subscribeData: hmmm, very strange, a unkown exception!\n");
	return string();
    }
}

std::string 
kvservice::priv::
KvQtCorbaApp::subscribeKvHint()
{
 	
  kvService_ptr     service; 
  CORBA::String_var ret;
  bool              forceNS=false;
  bool              usedNS;
  int               sensor;
  

    if(CORBA::is_nil(refKvHintSubs)){
	CERR("WARNING:(KvQtCorbaApp::subscribeKvHint): No KvHintSubcsriber (callback)!\n");
	return string();
    }
    
    try{
	for(int i=0; i<2; i++){
	    service=lookUpManager(forceNS, usedNS);
	    
	    try{
		ret=service->subscribeKvHint(refKvHintSubs);
		if(ret<0){
		    CERR("WARNING:(KvQtCorbaApp::subscribeKvHint): subscribeKvHint returned with a failindicator!\n");
		    return string();
		}
		
		subscriberid.push_back(string(ret));
		
		return std::string(ret);
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:(KvQtCorbaApp::subscribeKvHint) Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:(KvQtCorbaApp::subscribeKvHint) Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:(KvQtCorbaApp::subscribeKvHint) Exception unknown!\n");
		return string();
	    }
	    
	    if(usedNS){
		CERR("WARNING: cant subscribe 'kvHint' from kvManagerd!\n");
		return string();
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING: KvQtCorbaApp::subscribeKvHint: " << ex.what() << endl);
	return string();
    }
    catch(...){
	CERR("WARNING: KvQtCorbaApp::subscribeKvHint: hmmm, very strange, a unkown exception!\n");
	return string();
    }  
}



void 
kvservice::priv::
KvQtCorbaApp::unsubscribe(const std::string &id)
{
    std::list<std::string>::iterator it=subscriberid.begin();
    
    for(;it!=subscriberid.end(); it++){
	if(id==*it){
	    unsubscribe_(id);
	    subscriberid.erase(it);
	    return;
	}
    }
}

void 
kvservice::priv::
KvQtCorbaApp::unsubscribeAll()
{
    std::list<std::string>::iterator it=subscriberid.begin();
    CERR("unsubscribe ........\n");

    while(it!=subscriberid.end()){
      CERR("    subscriberid: " << *it << endl);
      unsubscribe_(*it);
      subscriberid.erase(it);
      it=subscriberid.begin();
    }
}



void
kvservice::priv::KvQtCorbaApp::
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
		CERR("WARNING:(KvQtCorbaApp::unsubscribe) Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:(KvQtCorbaApp::unsubscribe) Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:(KvQtCorbaApp::unsubscribe) Exception unknown!\n");
		return;
	    }
	    
	    if(usedNS){
		CERR("WARNING: cant unsubscribe from kvManagerInput!\n");
		return;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING: KvQtCorbaApp::subscribeDataNotify: " << ex.what() << endl);
    }
    catch(...){
	CERR("WARNING: KvQtCorbaApp::subscribeDataNotify: hmmm, very strange, a unkown exception!\n");
    }
}

bool
kvservice::priv::KvQtCorbaApp::
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
		    CERR("WARNING: cant get param data from kvalobs!\n");
		    return false;
		}
		params=params_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:getParams:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:getParams: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:getParams: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		CERR("WARNING:getParams: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING:getParams: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      CERR("WARNING:getParams: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}


bool
kvservice::priv::KvQtCorbaApp:: 
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
		    CERR("WARNING: cant get <type> data from kvalobs!\n");
		    return false;
		}
		types=types_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:getTypes:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:getTypes: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:getTypes: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		CERR("WARNING:getTypes: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING:getTypes: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      CERR("WARNING:getTypes: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}


bool
kvservice::priv::KvQtCorbaApp:: 
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
		    CERR("WARNING: cant get <operator> data from kvalobs!\n");
		    return false;
		}
		operators=operators_;

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
kvservice::priv::KvQtCorbaApp:: 
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
kvservice::priv::KvQtCorbaApp::
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
		    CERR("WARNING: cant get station data from kvalobs!\n");
		    return false;
		}
		stations=stations_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:getStations:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:getStations: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:getStations: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		CERR("WARNING:getStations: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING:getStations: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      CERR("WARNING:getStations: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}


bool 
kvservice::priv::KvQtCorbaApp::
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
		    CERR("WARNING:getReferenceStation: cant get reference_station from kvalobs!\n");
		    return false;
		}
		ref=ref_;

		return true;
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:getReferenceStation:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:getReferenceStation: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:getReferenceStation: Exception unknown!\n");
		return false;
	    }
	    
	    if(usedNS){
		CERR("WARNING:getReferenceStation: cant connect to kvalobs!\n");
		return false;
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING:getReferenceStation: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return false;
    }
    catch(...){
      CERR("WARNING:getReferenceStation: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return false;
    }

    return false;
}

CKvalObs::CService::DataIterator_ptr
kvservice::priv::KvQtCorbaApp::
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
	CERR("WARNING:getDataIter: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return DataIterator::_nil();
    }
    catch(...){
      CERR("WARNING:getDataIter: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return DataIterator::_nil();
    }

    return it;
    
}

CKvalObs::CService::ModelDataIterator_ptr 
kvservice::priv::KvQtCorbaApp::
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
		    CERR("WARNING: cant get data from kvalobs!\n");
		    return ModelDataIterator::_nil();
		}else
		  return it;
	    }
	    catch(CORBA::TRANSIENT &ex){
		CERR("WARNING:getModelDataIter:  Exception CORBA::TRANSIENT!\n");
	    }
	    catch(CORBA::COMM_FAILURE &ex){
		CERR("WARNING:getModelDataItera: Exception CORBA::COMM_FAILURE!\n");
	    }
	    catch(...){
		CERR("WARNING:getModelDataIter: Exception unknown!\n");
		return ModelDataIterator::_nil();
	    }
	    
	    if(usedNS){
		CERR("WARNING:getModelDataIter: cant connect to kvalobs!\n");
		return ModelDataIterator::_nil();
	    }
	    
	    forceNS=true;
	}
    }
    catch(LookUpException &ex){
	CERR("WARNING:getModelDataIter: can't lookup CORBA nameserver\n" 
	     "Reason: " << ex.what() << endl);
	return ModelDataIterator::_nil();
    }
    catch(...){
      CERR("WARNING:getModelDataIter: can't lookup CORBA nameserver\n" <<
	   "Reason:  unkown!\n");
      return ModelDataIterator::_nil();
    }

    return it;

}

bool
kvservice::priv::KvQtCorbaApp:: 
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
	  CERR("WARNING: cant get <ObsPgm> data from kvalobs!\n");
	  return false;
	}
	obsPgm=Obs_pgmList_;

	return true;
      }
      catch(CORBA::TRANSIENT &ex){
	CERR("WARNING:getObsPgm:  Exception CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	CERR("WARNING:getObsPgm: Exception CORBA::COMM_FAILURE!\n");
      }
      catch(...){
	CERR("WARNING:getObsPgm: Exception unknown!\n");
	return false;
      }
      
      if(usedNS){
	CERR("WARNING:getObsPgm: cant connect to kvalobs!\n");
	return false;
      }
      
      forceNS=true;
    }
  }
  catch(LookUpException &ex){
    CERR("WARNING:getObsPgm: can't lookup CORBA nameserver\n" 
	    "Reason: " << ex.what() << endl);
    return false;
  }
  catch(...){
    CERR("WARNING:getObsPgm: can't lookup CORBA nameserver\n" <<
	    "Reason:  unkown!\n");
    return false;
  }
  
  return false;
}


bool 
kvservice::priv::KvQtCorbaApp::getKvData(KvObsDataList &dataList,
					 const WhichDataHelper &wd)
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
	while(dataIt->next(data)){
	    for(CORBA::ULong i=0; i<data->length(); i++){
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
      CERR("EXCEPTION: Cant destroy DataIterator!!!!");
    }
    
    return !err;
}


bool 
kvservice::priv::KvQtCorbaApp::getKvParams(
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
kvservice::priv::KvQtCorbaApp::
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
kvservice::priv::KvQtCorbaApp::
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
kvservice::priv::KvQtCorbaApp::
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
kvservice::priv::KvQtCorbaApp::
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
kvservice::priv::KvQtCorbaApp::
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
kvservice::priv::KvQtCorbaApp::
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
kvservice::priv::KvQtCorbaApp::
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


bool
kvservice::priv::KvQtCorbaApp::
connectToKvInput(bool reConnect) 
{
  // TODO: Thread safety!

  if ( reConnect || CORBA::is_nil(dataInput) ) {
    dataInput = CKvalObs::CDataSource::Data::
      _narrow(this->getRefInNS("kvinput"));

    if ( CORBA::is_nil(dataInput) )
      return false;
  }
  return true;
}

const CKvalObs::CDataSource::Result_var
kvservice::priv::KvQtCorbaApp::
sendDataToKv(const char *data, const char *obsType)
{
  static omni_mutex me;
  
  me.lock(); // REMEMBER TO UNLOCK BEFORE LEAVING FUNCTION!

  if ( ! connectToKvInput() ) {
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = "Unable to look up Data Input Daemon.";
    me.unlock(); return r;
  }

  try{
    CKvalObs::CDataSource::Result_var ret = dataInput->newData(data, obsType);
    me.unlock(); return ret;
  }
  catch(CORBA::TRANSIENT &ex){
    CERR("WARNING:sendDataToKv:  Exception CORBA::TRANSIENT!\n");
  }
  catch(CORBA::COMM_FAILURE &ex){
    CERR("WARNING:sendDataToKv: Exception CORBA::COMM_FAILURE!\n");
  }
  catch(...){
    CERR("WARNING:sendDataToKv: Exception unknown!\n");
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = "Unable to connect to Data Input Daemon.";
 
    me.unlock(); return r;
  }

  if ( ! connectToKvInput(true) ) {
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = "Unable to look up Data Input Daemon.";
    me.unlock(); return r;
  }

  try{
    CKvalObs::CDataSource::Result_var ret = dataInput->newData(data, 
							       obsType);
    me.unlock(); return ret;
  }
  catch(...){
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = "Unable to look up Data Input Daemon.";
    me.unlock(); return r;
  }
}

