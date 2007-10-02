/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvServiceImpl.cc,v 1.1.2.3 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <memory>
#include <kvalobs/kvQueries.h>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvOperator.h>
#include <kvalobs/kvPsSubscribers.h>
#include "kvServiceImpl.h"
#include "kvDataIteratorImpl.h"
#include "kvRejectedIteratorImpl.h"
#include "kvModelDataIteratorImpl.h"

using namespace milog;
using namespace CKvalObs::CService;
using namespace kvalobs;
using namespace std;
using namespace dnmi::db;
using namespace milog;
using namespace miutil;

KvServiceImpl::
KvServiceImpl(ServiceApp &app_):app(app_)
{
}

KvServiceImpl::
~KvServiceImpl()
{
}

void
KvServiceImpl::
addToObsPgmList(CKvalObs::CService::Obs_pgmList &pgmList, 
			       std::list<kvalobs::kvObsPgm>    &obsPgms,
			       bool                            aUnion)
{
  CKvalObs::CService::Obs_pgm pgm;
  CKvalObs::CService::Obs_pgm pgmUnion;
  bool                        first=true;
  long                        stationid=-1;
  std::list<kvalobs::kvObsPgm>::iterator it=obsPgms.begin(); 
  CORBA::Long  index=pgmList.length();
                
  if(obsPgms.empty())
    return;

  if(!aUnion){
    try{
      pgmList.length(pgmList.length()+obsPgms.size());
    }
    catch(...){
      LOGERROR("NOMEM!!!");
      return;
    }
  }

  pgmUnion.kl00=false;
  pgmUnion.kl01=false;
  pgmUnion.kl02=false;
  pgmUnion.kl03=false;
  pgmUnion.kl04=false;
  pgmUnion.kl05=false;
  pgmUnion.kl06=false;
  pgmUnion.kl07=false;
  pgmUnion.kl08=false;
  pgmUnion.kl09=false;
  pgmUnion.kl10=false;
  pgmUnion.kl11=false;
  pgmUnion.kl12=false;
  pgmUnion.kl13=false;
  pgmUnion.kl14=false;
  pgmUnion.kl15=false;
  pgmUnion.kl16=false;
  pgmUnion.kl17=false;
  pgmUnion.kl18=false;
  pgmUnion.kl19=false;
  pgmUnion.kl20=false;
  pgmUnion.kl21=false;
  pgmUnion.kl22=false;
  pgmUnion.kl23=false;
  pgmUnion.mon=false;
  pgmUnion.tue=false;
  pgmUnion.wed=false;
  pgmUnion.thu=false;
  pgmUnion.fri=false;
  pgmUnion.sat=false;
  pgmUnion.sun=false;


  for(; it!=obsPgms.end(); it++){
    pgm.stationID=it->stationID();
    pgm.paramID=it->paramID();
    pgm.level=it->level();
    pgm.nr_sensor=it->nr_sensor();
    pgm.typeID_=it->typeID();
    pgm.collector=it->collector();
    pgm.kl00=it->kl00();
    pgm.kl01=it->kl01();
    pgm.kl02=it->kl02();
    pgm.kl03=it->kl03();
    pgm.kl04=it->kl04();
    pgm.kl05=it->kl05();
    pgm.kl06=it->kl06();
    pgm.kl07=it->kl07();
    pgm.kl08=it->kl08();
    pgm.kl09=it->kl09();
    pgm.kl10=it->kl10();
    pgm.kl11=it->kl11();
    pgm.kl12=it->kl12();
    pgm.kl13=it->kl13();
    pgm.kl14=it->kl14();
    pgm.kl15=it->kl15();
    pgm.kl16=it->kl16();
    pgm.kl17=it->kl17();
    pgm.kl18=it->kl18();
    pgm.kl19=it->kl19();
    pgm.kl20=it->kl20();
    pgm.kl21=it->kl21();
    pgm.kl22=it->kl22();
    pgm.kl23=it->kl23();
    pgm.mon=it->mon();
    pgm.tue=it->tue();
    pgm.wed=it->wed();
    pgm.thu=it->thu();
    pgm.fri=it->fri();
    pgm.sat=it->sat();
    pgm.sun=it->sun();
    pgm.fromtime=it->fromtime().isoTime().c_str();
      

    if(aUnion){
      if(stationid!=pgm.stationID){
	//If this is not the first stationID,
	//we must add the previeuos to the list.

	if(stationid>-1){
	  pgmList.length(index+1);
	  pgmList[index]=pgmUnion;
	  index++;
	}

	stationid=pgm.stationID;
	pgmUnion=pgm;
	pgmUnion.paramID=-1;
		
	continue;
      }
  
      if(pgm.kl00)
	pgmUnion.kl00=true;

      if(pgm.kl01)
	pgmUnion.kl01=true;

      if(pgm.kl02)
	pgmUnion.kl02=true;

      if(pgm.kl03)
	pgmUnion.kl03=true;

      if(pgm.kl04)
	pgmUnion.kl04=true;

      if(pgm.kl05)
	pgmUnion.kl05=true;

      if(pgm.kl06)
	pgmUnion.kl06=true;

      if(pgm.kl07)
	pgmUnion.kl07=true;

      if(pgm.kl08)
	pgmUnion.kl08=true;
      
      if(pgm.kl09)
	pgmUnion.kl09=true;

      if(pgm.kl10)
	pgmUnion.kl10=true;

      if(pgm.kl11)
	pgmUnion.kl11=true;

      if(pgm.kl12)
	pgmUnion.kl12=true;

      if(pgm.kl13)
	pgmUnion.kl13=true;

      if(pgm.kl14)
	pgmUnion.kl14=true;

      if(pgm.kl15)
	pgmUnion.kl15=true;

      if(pgm.kl16)
	pgmUnion.kl16=true;

      if(pgm.kl17)
	pgmUnion.kl17=true;

      if(pgm.kl18)
	pgmUnion.kl18=true;

      if(pgm.kl19)
	pgmUnion.kl19=true;

      if(pgm.kl20)
	pgmUnion.kl20=true;

      if(pgm.kl21)
	pgmUnion.kl21=true;

      if(pgm.kl22)
	pgmUnion.kl22=true;

      if(pgm.kl23)
	pgmUnion.kl23=true;

      if(pgm.mon)
	pgmUnion.mon=true;

      if(pgm.tue)
	pgmUnion.tue=true;

      if(pgm.wed)
	pgmUnion.wed=true;

      if(pgm.thu)
	pgmUnion.thu=true;
      
      if(pgm.fri)
	pgmUnion.fri=true;

      if(pgm.sat)
	pgmUnion.sat=true;
      
      if(pgm.sun)
	pgmUnion.sun=true;

      continue;
    }else{
      //    pgmList.length(index+1);
      pgmList[index]=pgm;
      index++;
    }
  }

  //If aUnion is true we have a left over from the for loop.
  //Add it to the list.

  if(aUnion){
    pgmList.length(index+1);
    pgmList[index]=pgmUnion;
    index++;
  }

}


CKvalObs::CService::WhichDataList*
KvServiceImpl::
createWhichDataListForAllStations(dnmi::db::Connection *dbCon, 
				  const CKvalObs::CService::WhichData &wData)
{
  CKvalObs::CService::WhichDataList *wDataList;
  
  if(!dbCon){
    LOGERROR("INTERNAL: Database connection is 0 (dbCon==0)!");
    return 0;
  }

  kvDbGate gate(dbCon);
  list<kvStation> stations;
  
  if(!gate.select(stations, kvQueries::selectAllStations("stationid"))){
    LOGERROR("Failed to get the station's from the database!" <<
	     gate.getErrorStr());
    return 0;
  }

  LOGDEBUG("Retrieved the stations from the database!");
  
  try{
    wDataList=new CKvalObs::CService::WhichDataList();
  }
  catch(...){
    LOGERROR("Out of memmory!");
    return 0;
  }

  list<kvStation>::iterator it=stations.begin();
  
  wDataList->length(stations.size());

  for(CORBA::Long i=0;
      it!=stations.end(); 
      it++, i++){
    (*wDataList)[i]=wData;
    (*wDataList)[i].stationid=it->stationID();
  }

  return wDataList;
}

char* 
KvServiceImpl::
subscribeDataNotify(const DataSubscribeInfo& info, 
		    kvDataNotifySubscriber_ptr sub)
{
  string                     subid;
  KvDataNotifySubscriber     *dnsub=0;    

  LogContext context("service/subscribeDataNotify");
  
  LOGINFO("New <datanotify> subscriber!");

  try{
    dnsub=new KvDataNotifySubscriber(info, sub, app);
  }
  catch(...){
    return CORBA::string_dup("");
  }
  
  //createDataNotifySubscribe take over the ownerchip of
  //the sub pointer.

  subid=app.subscribers.createDataNotifySubscriber(dnsub);
  
  if(subid.empty()){
    LOGERROR("Failed to register a new <datanotify> subscriber!");    
    return CORBA::string_dup("");
  }
   
  LOGINFO("New <datanotify> subscriber registred with subscriberid ("<<
	  subid<<")");
    
  return CORBA::string_dup(subid.c_str());
}


char*
KvServiceImpl::
subscribeData(const DataSubscribeInfo& info, 
	           kvDataSubscriber_ptr     sub)
{
  string              subid;
  KvDataSubscriber    *dsub=0;    

  LogContext context("service/subscribeData");
  
  LOGINFO("New <data> subscriber!");

  try{
     dsub=new KvDataSubscriber(info, sub, app);
  }
  catch(...){
  return CORBA::string_dup("");
  }
  
  //createDataSubscribe take over the ownerchip of
  //the sub pointer.

  subid=app.subscribers.createDataSubscriber(dsub);
  
  if(subid.empty()){
    LOGERROR("Failed to register a new <data> subscriber!");    
    return CORBA::string_dup("");
  }
   
  LOGINFO("New <data> subscriber registred with subscriberid ("<<
	  subid<<")");
    
  return CORBA::string_dup(subid.c_str());
} 

char* 
KvServiceImpl::
subscribeKvHint(CKvalObs::CService::kvHintSubscriber_ptr sub)
{
    string               subid;

    LogContext context("service/subscribeKvHint");  
    LOGINFO("New <hint> subscriber!");
    
    subid=app.subscribers.createHintSubscriber(sub);

    if(subid.empty()){
      LOGERROR("Failed to register a new <hint> subscriber!");    
      return CORBA::string_dup("");
    }    

    LOGINFO("New <hint> subscriber registred with subscriberid ("<<
	    subid<<")");
    
    return CORBA::string_dup(subid.c_str());
}



void 
KvServiceImpl::
unsubscribe(const char *subid)
{
	LogContext context("service/unsubscribe");
   LOGDEBUG("subscriberid: " << subid);
     
   if(!subid || *subid=='\0'){
   	LOGERROR("unsubscribe called without an susbscriberid!");
   	return;
   } 
     
   std::string id=subid;
   std::string::size_type i;
   
   i=id.find("ps_");
     
   if(i==0){
   	app.subscribers.unregisterPsSubscriber(id);
   	
   	Connection *dbCon=app.getNewDbConnection();
   
   	if(dbCon){
   		app.savePsSubscriberSIOR(dbCon, id, "");
    		app.releaseDbConnection(dbCon);
   	}
   }else{
   	app.subscribers.removeSubscriber(subid);
   }
}
  
CORBA::Boolean 
KvServiceImpl::
getData(const WhichDataList& whichData,
	DataIterator_out     it)
{
  	LogContext context("service/getData");
  
  	LOGDEBUG("called ...\n");

	if(app.isMaxClientReached()){
		LOGWARN("To many clients.....");
		return false;
	}

  	Connection *pCon=app.getNewDbConnection();
  	WhichDataList *pWhichData;
  	DataIteratorImpl *dataIt;
  	PortableServer::ObjectId *itId;

  	if(!pCon)
    	return false;

  	if(whichData.length()==0){
    	app.releaseDbConnection(pCon);
    	it=DataIterator::_nil();
    	LOGWARN("No station to get data from!");
    	return false;
  	}
   
  	if(whichData[0].stationid==0){
    	LOGINFO("It is asked for all stations!"  << endl <<
	    		  "fromtime: " << miTime(whichData[0].fromObsTime) <<
	    		  " (" << whichData[0].fromObsTime << ")" << endl <<
	    		  "totime: "  << miTime(whichData[0].toObsTime) << 
	    		  " (" << whichData[0].toObsTime << ")");
    
    	pWhichData=createWhichDataListForAllStations(pCon, whichData[0]);
  	}else{
    	try{
      	pWhichData=new WhichDataList(whichData);
    	}
    	catch(...){
      	pWhichData=0;
    	}
  	}

  	if(!pWhichData){
    	LOGERROR("OUT OF MEMMORY ...");
    	it=DataIterator::_nil();
    	app.releaseDbConnection(pCon);
    	return false;
  	}
    
  	try{
    	//dataIt becomes the owner of the pointers 'con' and 'pWhichData' 
    	//and will delete them when dataIt is deleted.
    	dataIt=new DataIteratorImpl(pCon, pWhichData, app);
  	}
  	catch(...){
    	LOGERROR("OUT OF MEMMORY (2)");
    	app.releaseDbConnection(pCon);
    	delete pWhichData;
    	it=DataIterator::_nil();
    	return false;
  	}
  
  	try{
    	itId=app.getPoa()->activate_object(dataIt);
    	dataIt->setObjId(itId);
  	}
  	catch(...){
    	LOGERROR("Cant register the DataIteratorImpl in the poa!");
    	delete dataIt; 
    	it=DataIterator::_nil();
    	return false;
  	}
 
  	try{
    	it=dataIt->_this();
  	}
  	catch(...){
    	LOGERROR("Cant obtain a referanse to the DataIterator!\n");
    	delete dataIt; 
    	it=DataIterator::_nil();
    	return false;
  	}

  	if(CORBA::is_nil(it)){
    	LOGERROR("cant instatiate (DataIterator)!!!!");
    	delete dataIt;
    	return false;
  	}

	app.addReaperObj(dataIt);

  	return true;
}

CORBA::Boolean 
KvServiceImpl::
getModelData(const CKvalObs::CService::WhichDataList& whichData,
	     CKvalObs::CService::ModelDataIterator_out it)
{
  	LogContext context("service/getModelData");
  
  	LOGDEBUG("called ...\n");
	
	if(app.isMaxClientReached()){
		LOGWARN("To many clients.....");
		return false;
	}

  	Connection *pCon=app.getNewDbConnection();
  	WhichDataList *pWhichData;
  	ModelDataIteratorImpl *dataIt;
  	PortableServer::ObjectId *itId;

  	if(!pCon)
    	return false;

  	try{
    	pWhichData=new WhichDataList(whichData);
  	}
  	catch(...){
    	LOGERROR("OUT OF MEMMORY ...");
    	it=ModelDataIterator::_nil();
    	app.releaseDbConnection(pCon);
   	return false;
  	}

  	try{
    	//dataIt becomes the owner of the pointers 'con' and 'pWhichData' 
    	//and will delete them when dataIt is deleted.
    	dataIt=new ModelDataIteratorImpl(pCon, pWhichData, app);
  	}
  	catch(...){
    	LOGERROR("OUT OF MEMMORY (2)");
    	app.releaseDbConnection(pCon);
    	delete pWhichData;
    	it=ModelDataIterator::_nil();
    	return false;
  	}
  
  	try{
    	itId=app.getPoa()->activate_object(dataIt);
    	dataIt->setObjId(itId);
  	}
  	catch(...){
    	LOGERROR("Cant register the DataIteratorImpl in the poa!");
    	delete dataIt; 
    	it=ModelDataIterator::_nil();
    	return false;
  	}
 
  	try{
    	it=dataIt->_this();
  	}
  	catch(...){
    	LOGERROR("Cant obtain a referanse to the DataIterator!\n");
    	delete dataIt; 
    	it=ModelDataIterator::_nil();
    	return false;
  	}

  	if(CORBA::is_nil(it)){
    	LOGERROR("cant instatiate (DataIterator)!!!!");
    	delete dataIt;
    	return false;
  	}
  
	app.addReaperObj(dataIt);
  	return true;
}


CORBA::Boolean 
KvServiceImpl::
getRejectdecode(const CKvalObs::CService::RejectDecodeInfo& info,
		CKvalObs::CService::RejectedIterator_out it)
{
  	RejectedIteratorImpl     *dataIt;
  	PortableServer::ObjectId *itId;
  	miTime                   fromTime, toTime;
  	list<string>             decodeList;
  
  	LogContext context("service/getRejectdecode");

	if(app.isMaxClientReached()){
		LOGWARN("To many clients.....");
		return false;
	}

  	it=RejectedIterator::_nil();

  	if(strlen(info.fromTime)==0){
   	fromTime=miTime::nowTime();
    	fromTime=miTime(fromTime.date(), miClock(0,0,0));
  	}else{
    	fromTime=miTime(info.fromTime);
  	}
  
  	if(strlen(info.toTime)==0){
    	toTime=miTime::nowTime();
  	}else{
    	toTime=miTime(info.toTime);
  	}

  	if(fromTime.undef() || toTime.undef()){
    	if(fromTime.undef()){
      	LOGERROR("INVALID fromTime: <" << info.fromTime << ">");
    	}else{
      	LOGERROR("INVALID toTime: <" << info.toTime << ">");
    	}

    	return false;
  	}

  	if(info.decodeList.length()>0){
    	for(CORBA::Long i=0; i<info.decodeList.length(); i++)
      	decodeList.push_back(string(info.decodeList[i]));
  	}

  	Connection *pCon=app.getNewDbConnection();

  	if(!pCon)
    	return false;

  	try{
    	//dataIt becomes the owner of the pointers 'con' and 'pWhichData' 
    	//and will delete them when dataIt is deleted.
    	dataIt=new RejectedIteratorImpl(pCon, fromTime, toTime, decodeList, app);
  	}
  	catch(...){
   	LOGERROR("OUT OF MEMMORY (2)");
    	app.releaseDbConnection(pCon);
    	return false;
  	}
  
  	try{
    	itId=app.getPoa()->activate_object(dataIt);
    	dataIt->setObjId(itId);
  	}
  	catch(...){
    	LOGERROR("Cant register the DataIteratorImpl in the poa!");
    	delete dataIt; 
    	return false;
  	}
 
  	try{
    	it=dataIt->_this();
  	}
  	catch(...){
   	LOGERROR("Cant obtain a referanse to the DataIterator!\n");
    	delete dataIt; 
    	return false;
  	}

  	if(CORBA::is_nil(it)){
    	LOGERROR("cant instatiate (DataIterator)!!!!");
    	delete dataIt;
    	return false;
  	}

  	app.addReaperObj(dataIt);

  	return true;
} 



CORBA::Boolean 
KvServiceImpl::
getReferenceStation(CORBA::Long stationid, 
		    CORBA::Short paramsetid, 
		    CKvalObs::CService::Reference_stationList_out 
		    refStationList)
{
  LogContext context("service/getReferenceStation");
  Connection *pCon=app.getNewDbConnection();
  bool       res;

  try{
    refStationList=new Reference_stationList();
  }
  catch(...){
    if(pCon)
      app.releaseDbConnection(pCon);

    LOGDEBUG("OUT OF MEMMORY!!!");
    return false;
  }

  if(!pCon)
    return false;

  kvDbGate gate(pCon);
  list<kvReferenceStation> stations;
  
  res=gate.select(stations, kvQueries::selectReferenceStation(stationid, 
							      paramsetid));
  app.releaseDbConnection(pCon);
  
   
  if(!res){
    LOGERROR("Failed to get the reference station's from the database!");
    return false;
  }

  LOGDEBUG("Retrieved the stations from the database!");
  
  refStationList->length(stations.size());
  
  list<kvReferenceStation>::iterator it=stations.begin();
  

  for(CORBA::Long i=0;
      it!=stations.end(); 
      it++, i++){
    (*refStationList)[i].stationID=it->stationID();
    (*refStationList)[i].paramsetID=it->paramsetID();
    (*refStationList)[i].reference=CORBA::string_dup(it->reference().c_str());
  }

  return true;
}




CORBA::Boolean 
KvServiceImpl::
getStations(CKvalObs::CService::StationList_out stationList)
{
  LogContext context("service/getStations");
  Connection *pCon=app.getNewDbConnection();
  bool       res;

  try{
    stationList=new StationList();
  }
  catch(...){
    if(pCon)
      app.releaseDbConnection(pCon);

    LOGDEBUG("OUT OF MEMMORY!!!");
    return false;
  }

  if(!pCon)
    return false;

  kvDbGate gate(pCon);
  list<kvStation> stations;
  
  res=gate.select(stations, kvQueries::selectAllStations("stationid"));
  app.releaseDbConnection(pCon);
  
   
  if(!res){
    LOGERROR("Failed to get the station's from the database!");
    return false;
  }

  LOGDEBUG("Retrieved the stations from the database!");
  
  list<kvStation>::iterator it=stations.begin();
  
  stationList->length(stations.size());

  for(CORBA::Long i=0;
      it!=stations.end(); 
      it++, i++){
    (*stationList)[i].stationID=it->stationID();
    (*stationList)[i].lat=it->lat();
    (*stationList)[i].lon=it->lon();
    (*stationList)[i].height=it->height();
    (*stationList)[i].maxspeed=it->maxspeed();
    (*stationList)[i].name=it->name().c_str();
    (*stationList)[i].wmonr=it->wmonr();
    (*stationList)[i].nationalnr=it->nationalnr();
    (*stationList)[i].ICAOid=it->ICAOID().c_str();
    (*stationList)[i].call_sign=it->call_sign().c_str();
    (*stationList)[i].stationstr=it->stationstr().c_str();
    (*stationList)[i].environmentid=it->environmentid();
    (*stationList)[i].static_=it->_static();
    (*stationList)[i].fromtime=it->fromtime().isoTime().c_str();
  }

  return true;
}


CORBA::Boolean 
KvServiceImpl::
getParams(ParamList_out paramList)
{
  list<kvParam>           params;
  list<kvParam>::iterator it;
  bool                    res;
  Connection *pCon=app.getNewDbConnection();
  
  LogContext context("service/getParams");
  
  try{
    paramList=new ParamList();
  }
  catch(...){
    if(pCon)
      app.releaseDbConnection(pCon);

    LOGDEBUG("OUT OF MEMMORY!!!");
    return false;
  }
  
  if(!pCon)
    return false;

  kvDbGate gate(pCon);
  
  res=gate.select(params, kvQueries::selectParam("paramid"));
  app.releaseDbConnection(pCon);

  if(!res){
    LOGERROR("Failed to get the params's from the database!");
    return false;
  }

  LOGDEBUG("Retrieved the params from the database!");
  
  paramList->length(params.size());
  it=params.begin();

  for(CORBA::Long i=0;
      it!=params.end(); 
      it++, i++){
    (*paramList)[i].paramID=it->paramID();
    (*paramList)[i].name=it->name().c_str();
    (*paramList)[i].description=it->description().c_str();
    (*paramList)[i].unit=it->unit().c_str();
    (*paramList)[i].level_scale=it->level_scale();
    (*paramList)[i].comment=it->comment().c_str();
  }

  return true;
}


CORBA::Boolean 
KvServiceImpl::
getObsPgm(CKvalObs::CService::Obs_pgmList_out obs_pgmList, 
	  const CKvalObs::CService::StationIDList& stationIDList, 
	  CORBA::Boolean aUnion)
{
  list<kvObsPgm>           obsPgms;
  list<kvObsPgm>::iterator it;
  bool                     res=false;
  bool                     tmpRes;
  std::ostringstream       ost;
  Connection *pCon=app.getNewDbConnection();
  
  LogContext context("service/getObsPgm");

  LOGDEBUG("called!");

  obs_pgmList=0;
  
  try{
    obs_pgmList=new CKvalObs::CService::Obs_pgmList();
  }
  catch(...){
    if(pCon)
      app.releaseDbConnection(pCon);

    LOGDEBUG("OUT OF MEMMORY!!!");
    
    return false;
  }

  if(!pCon)
    return false;

  kvDbGate gate(pCon);
  

  if(stationIDList.length()==0){
      res=gate.select(obsPgms, " order by stationID");
      app.releaseDbConnection(pCon);

      if(res){
	addToObsPgmList(*obs_pgmList, obsPgms, aUnion);
      }

      LOGDEBUG("Returning <ObsPgm> for all stations!");
      return res;
  }
  
  for(CORBA::Long i=0; i<stationIDList.length(); i++){
    ost.str("");
    ost << "where stationID=" << stationIDList[i] << "order by paramID"; 
    
    
    tmpRes=gate.select(obsPgms, ost.str());
    
    if(tmpRes)
      res=true;
  
    if(tmpRes)
      addToObsPgmList(*obs_pgmList, obsPgms, aUnion);
  }

  app.releaseDbConnection(pCon);
  
  if(!res)
    obs_pgmList->length(0);

  LOGDEBUG("Returning <ObsPgm> for the requested stations!");

  return res;
  
}

CORBA::Boolean 
KvServiceImpl::
getTypes(CKvalObs::CService::TypeList_out typeList)
{
  LogContext context("service/getTypes");
  list<kvTypes>           types;
  list<kvTypes>::iterator it;

  Connection *pCon=app.getNewDbConnection();
  bool       res;

  typeList=0;

  try{
    typeList=new CKvalObs::CService::TypeList();
  }
  catch(...){
    if(pCon)
      app.releaseDbConnection(pCon);

    LOGDEBUG("NOMEM!!!");
    return false;
  }


  if(!pCon)
    return false;

  kvDbGate gate(pCon);
  
  res=gate.select(types, " order by typeid");
  app.releaseDbConnection(pCon);

  if(!res){
    LOGERROR("Failed to get the <types> from the database!");
    return false;
  }

  LOGDEBUG("Retrieved the <types> from the database!");
  
  it=types.begin();
  typeList->length(types.size());

  for(CORBA::Long i=0;
      it!=types.end(); 
      it++, i++){
    (*typeList)[i].typeID_ =it->typeID();
    (*typeList)[i].format  =it->format().c_str();
    (*typeList)[i].earlyobs=it->earlyobs();
    (*typeList)[i].lateobs =it->lateobs();
    (*typeList)[i].read    =it->read().c_str();
    (*typeList)[i].obspgm  =it->obspgm().c_str();
    (*typeList)[i].comment =it->comment().c_str();
  }

  
  return true;
 
}


CORBA::Boolean 
KvServiceImpl::
getOperator(CKvalObs::CService::OperatorList_out operatorList)
{
  LogContext                 context("service/getOperator");
  list<kvOperator>           operators;
  list<kvOperator>::iterator it;

  Connection *pCon=app.getNewDbConnection();
  bool       res;

  operatorList=0;

  try{
    operatorList=new CKvalObs::CService::OperatorList();
  }
  catch(...){
    if(pCon)
      app.releaseDbConnection(pCon);

    LOGDEBUG("NOMEM!!!");
    return false;
  }

  if(!pCon)
    return false;

  kvDbGate gate(pCon);
  
  res=gate.select(operators, " order by userid");
  app.releaseDbConnection(pCon);

  if(!res){
    LOGERROR("Failed to get the <types> from the database!");
    return false;
  }

  LOGDEBUG("Retrieved the <types> from the database!");
  
  it=operators.begin();
  operatorList->length(operators.size());

  for(CORBA::Long i=0;
      it!=operators.end(); 
      it++, i++){
    (*operatorList)[i].userid =it->userID();
    (*operatorList)[i].username  =it->username().c_str();
  }

  return true;
 
}

CORBA::Boolean 
KvServiceImpl::
getStationParam( CKvalObs::CService::Station_paramList_out spList,
		 CORBA::Long stationid, 
		 CORBA::Long paramid, 
		 CORBA::Long day )
{
  LogContext context( "service/getStationParam" );
  Connection *con = app.getNewDbConnection();

  spList = 0;

  try {
    spList = new CKvalObs::CService::Station_paramList();
  }
  catch(...){
    if( con )
      app.releaseDbConnection(con);

    LOGDEBUG("NOMEM!!!");
    return false;
  }

  if(!con)
    return false;

  ostringstream s;

  s << "select * from station_param where stationid=" << stationid;

  if ( paramid >= 0 )
    s << " and paramid=" << paramid;

  if ( day >= 0 )
    s << " and fromday<=" << day
      << " and today>=" << day;

  s << " order by paramid, fromday";

  bool       retRes=true;

  try{
    auto_ptr<Result> res( con->execQuery(s.str()) );

    spList->length( res->size() );
    int pos = -1;
   
    while ( res->hasNext() ) {
      DRow r = res->next();
      pos++;
      CORBA::Long rPos = 0;
      Station_param &sParam = (*spList)[ pos ];
      sParam.stationid =     atol( r[ rPos++ ].c_str() );
      sParam.paramid =       atol( r[ rPos++ ].c_str() );
      sParam.level =         atol( r[ rPos++ ].c_str() );
      sParam.sensor =              r[ rPos++ ].c_str();
      sParam.fromday =       atol( r[ rPos++ ].c_str() );
      sParam.today =         atol( r[ rPos++ ].c_str() );
      sParam.hour =          atol( r[ rPos++ ].c_str() );
      sParam.qcx =                 r[ rPos++ ].c_str();
      sParam.metadata =            r[ rPos++ ].c_str();
      sParam.desc_metadata =       r[ rPos++ ].c_str();
      sParam.fromtime =            r[ rPos++ ].c_str();
    }
  }
  catch(...){
    retRes=false;
  }

  app.releaseDbConnection(con);

  return retRes;
}



KvServiceExtImpl::~KvServiceExtImpl()
{
}
		
char* 
KvServiceExtImpl::
registerDataNotify(const char* id, CKvalObs::CService::kvDataNotifySubscriberExt_ptr sub)
{
		return CORBA::string_dup("");
}

char* 
KvServiceExtImpl::
registerData(const char* id, 
             CKvalObs::CService::kvDataSubscriberExt_ptr sub)
{
	kvDataSubscriberExt_ptr ptr;

  	LogContext context("service/registerData");
  
  	LOGDEBUG("Register persistent <data> subscriber!");

  	try{
    	ptr=kvDataSubscriberExt::_duplicate(sub);
  	}
  	catch(...){
    	CORBA::release(ptr);
    	return CORBA::string_dup("");
  	}
  
  	if(!app.subscribers.registerPsDataSubscriber(ptr, id)){
  		LOGERROR("Failed to register a persistent <data> subscriber!");
  	 	CORBA::release(ptr);
    	return CORBA::string_dup("");
  	}
   
   Connection *dbCon=app.getNewDbConnection();
   
   if(dbCon){
   	app.savePsSubscriberSIOR(dbCon, id, app.corbaRef(ptr));
    	app.releaseDbConnection(dbCon);
   }
   
 	LOGINFO("Registred a persistent <data> subscriber, id <" << id <<">.");
 	return CORBA::string_dup(id);
}

CORBA::Boolean 
KvServiceExtImpl::
getDataExt(const CKvalObs::CService::WhichDataExtList& whichData,
  			  CKvalObs::CService::DataIterator_out it)
{
	return false;
}

