/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceSubscriber.cc,v 1.12.2.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <cstring>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvWorkelement.h>
#include <milog/milog.h>
#include "ServiceSubscriber.h"

namespace{
  const unsigned char QC1_mask=0x01;
  const unsigned char QC2d_mask=0x02;
  const unsigned char QC2m_mask=0x04;
  const unsigned char HQC_mask=0x08;
};


using namespace std;
using namespace kvalobs;
using namespace miutil;

ServiceSubscriber::
ServiceSubscriber(ServiceApp &app_,
		  dnmi::thread::CommandQue &que_)
  :app(app_), inputque(que_), dbCon(0)
{
}
  
ServiceSubscriber::
ServiceSubscriber(const ServiceSubscriber &s)
  :app(s.app), inputque(s.inputque), dbCon(s.dbCon)
{
}

ServiceSubscriber::
~ServiceSubscriber()
{
}


void 
ServiceSubscriber::
updateWorkelementServiceStart(const kvalobs::kvStationInfo &st,
			      dnmi::db::Connection *con)
{
  kvDbGate gate(con);
  ostringstream ost;
  
  ost << "UPDATE workque SET service_start='" 
      << miTime::nowTime() 
      << "' WHERE stationid=" << st.stationID() 
      << "  AND obstime='" << st.obstime().isoTime() 
      << "' AND typeid=" << st.typeID();
  

  if(!gate.exec(ost.str())){
    LOGERROR("DBERROR: Cant update workque!" << endl <<
	     "Reason: " << gate.getErrorStr());
  }

}

void 
ServiceSubscriber::
updateWorkelementServiceStop(const kvalobs::kvStationInfo &st,
			       dnmi::db::Connection *con)
{
  kvDbGate gate(con);
  ostringstream ost;
  list<kvWorkelement> workList;

  ost << "UPDATE workque SET service_stop='" 
      << miTime::nowTime() 
      << "' WHERE stationid=" << st.stationID() 
      << "  AND obstime='" << st.obstime().isoTime() 
      << "' AND typeid=" << st.typeID();
  

  if(!gate.exec(ost.str())){
    LOGERROR("DBERROR: Cant update workque!" << endl <<
	     "Reason: " << gate.getErrorStr());
    return;
  }
}


void 
ServiceSubscriber::
callDataNotifySubscribers(const kvalobs::kvStationInfo &si )
{
  if(!app.subscribers.hasDataNotifySubscribers())
      return;

  if(!dbCon){
      LOGERROR("DataNotify: No database connection! (dbCon==0)");
      return;
  }
  
  kvalobs::kvDbGate gate(dbCon);

  list<kvData> dataList;
    
  if(!gate.select(dataList,
		  kvQueries::selectDataFromType(si.stationID(),
						si.typeID(),
						si.obstime()))){
    LOGERROR("DataNotify: Cant read from the database: " << endl <<
	     "  stationID: " << si.stationID() << endl <<
	     "     typeID: " << si.typeID() << endl <<
	     "    obstime: " << si.obstime() );
    return;
  }
  
  DataNotifyFunc dataNotify(si, dataList);
  LOGDEBUG("CALL DataNotifySubscribers: stationID(" << si.stationID() <<
	   ")\n" << si);
  
  app.subscribers.forAllDataNotifySubscribers(dataNotify, si.stationID());
  removeDeadConnections();
}

void 
ServiceSubscriber::
callDataSubscribers(const kvalobs::kvStationInfo &si)
{
  long stationID;
  std::list<kvalobs::kvData>       dataList;
  std::list<kvalobs::kvTextData>   textDataList;
  DataToSendList                   dataToSend;                   
  
  if(!app.subscribers.hasDataSubscribers())
    return;
  
  if(!dbCon){
    LOGERROR("callDataSubscribers: dbCon==0!");
    return;
  }

  kvalobs::kvDbGate gate(dbCon);

  if(!gate.select(dataList,
		  kvQueries::selectDataFromType(si.stationID(),
						si.typeID(),
						si.obstime()))){
    dataList.clear();
  }

  if(!gate.select(textDataList,
		  kvQueries::selectDataFromType(si.stationID(),
						si.typeID(),
						si.obstime()))){
    textDataList.clear();
  }
  
  if(dataList.empty() && textDataList.empty())
    return;

  dataToSend.push_back(DataToSend(dataList, textDataList, si.stationID()));
      
  DataFunc dataf(dataToSend);
  
  stationID=si.stationID();
  LOGDEBUG("CALL DataSubscribers: stationID: "<< si.stationID() 
	   << " obstime: " << si.obstime() 
	   << " typeID: " << si.typeID());
  app.subscribers.forAllDataSubscribers(dataf, stationID);
  removeDeadConnections();
}

void       
ServiceSubscriber::
removeDeadConnections()
{
  app.subscribers.removeDeadSubscribers(60);
}
 
void       
ServiceSubscriber::
operator()()
{
  const                     int CON_IDLE_TIME=60;
  const                     int WAIT_ON_QUE_TIMEOUT=1;
  int                       conIdleTime=0;
  DataReadyCommand          *stInfoCmd=0;
  dnmi::thread::CommandBase *cmd=0;

  milog::LogContext logContext("ServiceSubscriber");

  while(!app.shutdown()){
    cmd=inputque.get(WAIT_ON_QUE_TIMEOUT);
    
    if(!cmd){
      conIdleTime+=WAIT_ON_QUE_TIMEOUT;
      
      if(conIdleTime>CON_IDLE_TIME){
	if(dbCon){
	  LOGDEBUG("Closing the database connection!");
	  app.releaseDbConnection(dbCon);
	  dbCon=0;
	}
	
	conIdleTime=0;
      }
      
      continue;
    }
    
    if(!dbCon){
      //Will try to create a new connection to the database, we will
      //not continue before a connection is created or the application
      //is shutdown.
      
      do{
	dbCon=app.getNewDbConnection();
	
	if(dbCon){
	  LOGDEBUG("Created a new connection to the database!");
	  break;
	}
	
	LOGINFO("Can't create a connection to the database, retry in 5 seconds ..");
	sleep(5);
      }while(!app.shutdown());
      
      if(!dbCon){
	//We have failed to create a new database connection and we have got a 
	//shutdown condition. We use continue to evaluate the outer while loop 
	//that in turn will end this thread.
	continue; 
      }
    }
    
    try{
      stInfoCmd=dynamic_cast<DataReadyCommand*>(cmd);
      
      if(!stInfoCmd){
	delete cmd;
	LOGERROR("Unexpected command!");
	continue;
      }
    }
    catch(...){
      LOGERROR("Exception: unexpected command!");
      continue;
    }
    
    
    LOGDEBUG("DataReady received!");
    
    
    conIdleTime=0;
    kvalobs::IkvStationInfoList it=stInfoCmd->getStationInfo().begin();

    for(;it!=stInfoCmd->getStationInfo().end(); it++){
      updateWorkelementServiceStart(*it, dbCon);    
      callDataNotifySubscribers(*it);
      callDataSubscribers(*it);
      updateWorkelementServiceStop(*it, dbCon);    
    }
    
    app.sendToManager(stInfoCmd->getStationInfo(), 
		      stInfoCmd->getCallback());
    
    delete stInfoCmd;
  }
  
  LOGINFO("ServiceSubscriber terminated!");
} 




void 
DataNotifyFunc::func(KvDataNotifySubscriberPtr ptr)
{
  using namespace CKvalObs::CService;
  kvDataNotifySubscriber::WhatList wl;

  if(!checkStatusAndQc(ptr)){
    return;
  }


  if(!buildWhatList(wl)){
    LOGERROR("DataNotifyFunc::func: buildWhatList failed!\n");
    return;
  }

  try{
    CKvalObs::CService::kvDataNotifySubscriber_var ref=ptr->subscriber();
    
    ref->callback(wl);
    ptr->connection(true);
  }
  catch(CORBA::TRANSIENT &ex){
       ptr->connection(false, true);
       LOGERROR("EXCEPTION: (timeout?) Can't send <DataNotify> event to subscriber!" <<
		endl << "Subscriberid: " << ptr->subscriberid() << ">!");
  }
  catch(...){
    ptr->connection(false);
    LOGERROR("EXCEPTION: Can't send <DataNotify> event to subscriber!" <<
	     endl << "Subscriberid: " << ptr->subscriberid() << ">!");
  }
}

bool
DataNotifyFunc::buildWhatList(
      CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl
      )
{
  unsigned char        qcLevel=0x00;
  unsigned char        flag;
  char                 b[100];
  CORBA::Long          i=0;
  CORBA::Long          wli=0;
  //  kvalobs::CIkvParamInfoList it;

  wl.length(wli+1);
  wl[wli].stationID=stationInfo.stationID();
  wl[wli].typeID_=stationInfo.typeID();
  wl[wli].obsTime=stationInfo.obstime().isoTime().c_str();


  for(list<kvData>::const_iterator it=dataList.begin();
      it!=dataList.end(); 
      it++){
    flag=it->controlinfo().cflag(0);
    qcLevel |=flag;
  }
  
  i=0;

  if(qcLevel & QC1_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=CKvalObs::CService::QC1;
    i++;
  }

  if(qcLevel & QC2d_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=CKvalObs::CService::QC2d;
    i++;
  }

  if(qcLevel & QC2m_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=CKvalObs::CService::QC2m;
    i++;
  }

  if(qcLevel & HQC_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=CKvalObs::CService::HQC;
    i++;
  }

  return true;
}

bool
DataNotifyFunc::fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag)
{
  switch(qcId){
  case CKvalObs::CService::QC1:  
    if(flag & QC1_mask)
      return true;
    break;

  case CKvalObs::CService::QC2d: 
    if(flag & QC2d_mask)
      return true; 
    break;

  case CKvalObs::CService::QC2m:
    if(flag & QC2m_mask)
      return true; 
    break;

  case CKvalObs::CService::HQC:  
    if(flag & HQC_mask)
      return true;
    break;
  }

  return false;
}


bool
DataNotifyFunc::checkStatusAndQc(KvDataNotifySubscriberPtr ptr)
{
  unsigned char flag;

  if(!ptr->subscriberInfo().qcAll()){
    //Check if the subscriber is interested in the qcLevels that
    //is set for this observation. We return true if we find a
    //qcLevel that match for any parameter in the observation.
    bool qcLevel=false;

    for(list<kvData>::const_iterator it=dataList.begin();
	it!=dataList.end() && !qcLevel; it++){
      flag=it->controlinfo().cflag(0);

      for(int i=0; i<ptr->subscriberInfo().qc().length(); i++){
	if(fqcLevel(ptr->subscriberInfo().qc()[i], flag)){
	  qcLevel=true;
	  break;
	}
      }
    }
      
    if(!qcLevel)
      return false;
  }


  if(ptr->subscriberInfo().status()!=CKvalObs::CService::All){
    bool hasFailed=false;
    
    for(list<kvData>::const_iterator it=dataList.begin();
	it!=dataList.end() && !hasFailed ; it++){
      flag=it->useinfo().cflag(0);
      
      //Check if bit 1 in useinfo is set. This bit is set to 1 if
      //the value is useless.
      //CODE: Check if this select the correct bit.
      if(flag & 0x01) 
	hasFailed=true;
    }

    if(hasFailed && 
       ptr->subscriberInfo().status()==CKvalObs::CService::OnlyFailed)
      return true;
    else if(!hasFailed && 
	    ptr->subscriberInfo().status()==CKvalObs::CService::OnlyOk)
      return true;
    else
      return false;
  }
  
  return true;

}


bool 
DataFunc::fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag)
{
    switch(qcId){
    case CKvalObs::CService::QC1:  
	if(flag & QC1_mask)
	    return true;
	break;
	
    case CKvalObs::CService::QC2d: 
	if(flag & QC2d_mask)
	    return true; 
	break;
	
    case CKvalObs::CService::QC2m:
	if(flag & QC2m_mask)
	    return true; 
	break;
	
    case CKvalObs::CService::HQC:  
	if(flag & HQC_mask)
	    return true;
	break;
    }
    
    return false;
}

bool 
DataFunc::checkStatusAndQc(KvDataSubscriberPtr ptr)
{
    CORBA::Long   it;
    CORBA::Long   i;
    unsigned char flag;
    
    if(!ptr->subscriberInfo().qcAll()){
	//Check if the subscriber is interested in the qcLevels that
	//is set for this observation. We return true if we find a
	//qcLevel that match for any parameter in the observation.
	bool qcLevel=false;

	for(i=0; i<data.length(); i++){
	  for(it=0;it<data[i].dataList.length() && !qcLevel; it++){
	    flag=kvControlInfo(string(data[i].dataList[it].controlinfo)).cflag(0);
    
	    for(int i=0; i<ptr->subscriberInfo().qc().length(); i++){
	      if(fqcLevel(ptr->subscriberInfo().qc()[i], flag)){
		qcLevel=true;
		break;
	      }
	    }
	  }
	 
	  if(!qcLevel)
	    return false;
	}
    }
    

    if(ptr->subscriberInfo().status()!=CKvalObs::CService::All){
      bool hasFailed=false;
      
      for(i=0; i<data.length(); i++){
	for(it=0;it<data[i].dataList.length() && !hasFailed ; it++){
	  flag=kvUseInfo(string(data[i].dataList[it].useinfo)).cflag(0);
	  
	  //Check if bit 1 in useinfo is set. This bit is set to 1 if
	  //the value is useless.
	  //CODE: Check if this select the correct bit.
	  if(flag & 0x01) 
	    hasFailed=true;
	}
      }
      
      if(hasFailed && 
	 ptr->subscriberInfo().status()==CKvalObs::CService::OnlyFailed)
	return true;
      else if(!hasFailed && 
	      ptr->subscriberInfo().status()==CKvalObs::CService::OnlyOk)
	return true;
      else
	return false;
    }
    
    return true;
    
}


DataFunc::DataFunc(const DataToSendList &dataList)
{
  CORBA::Long            obsi=0;
  CORBA::Long            datai;
  char                   *sTmp;
  char                   buf[20];
  bool                   hasData;

  data.length(dataList.size());

  for(CIDataToSendList itd=dataList.begin();
      itd!=dataList.end();
      itd++){
    hasData=false;

    if(itd->dataList.size()==0 && itd->textDataList.size()==0)
      continue;

    data[obsi].stationid=itd->stationid;

    if(itd->dataList.size()>0){
      datai=0;
      data[obsi].dataList.length(itd->dataList.size());
      hasData=true;

      for(list<kvalobs::kvData>::const_iterator it=itd->dataList.begin();
	  it!=itd->dataList.end(); 
	  it++) {

	data[obsi].dataList[datai].stationID=it->stationID(); 
	data[obsi].dataList[datai].obstime=it->obstime().isoTime().c_str();
	data[obsi].dataList[datai].original=it->original();
	data[obsi].dataList[datai].paramID=it->paramID();
	data[obsi].dataList[datai].tbtime=it->tbtime().isoTime().c_str();
	data[obsi].dataList[datai].typeID_=it->typeID();
	
	sprintf(buf, "%d", it->sensor()); 
	sTmp=CORBA::string_dup(buf);
	
	if(sTmp){
	  data[obsi].dataList[datai].sensor=sTmp;
	}else{
	  LOGERROR("DataFunc (CTOR): NOMEM for <kvData::sensor>!");
	}
	
	data[obsi].dataList[datai].level=it->level();
	data[obsi].dataList[datai].corrected=it->corrected();
	data[obsi].dataList[datai].controlinfo=it->controlinfo().flagstring().c_str();
	data[obsi].dataList[datai].useinfo=it->useinfo().flagstring().c_str();
	data[obsi].dataList[datai].cfailed=it->cfailed().c_str();
	
	datai++;
      }

      if(datai!=itd->dataList.size()){
	LOGERROR("Datafunc (CTOR): Inconsistent size, dataList!");
	data[obsi].dataList.length(datai);
      }
	
    }else{
      data[obsi].dataList.length(0);
    }

    if(itd->textDataList.size()>0){
      datai=0;
      data[obsi].textDataList.length(itd->textDataList.size());
      hasData=true;

      for(list<kvalobs::kvTextData>::const_iterator it=itd->textDataList.begin();
	  it!=itd->textDataList.end();
	  it++) {

	data[obsi].textDataList[datai].stationID=it->stationID(); 
	data[obsi].textDataList[datai].obstime=it->obstime().isoTime().c_str();
	data[obsi].textDataList[datai].original=it->original().c_str();
	data[obsi].textDataList[datai].paramID=it->paramID();
	data[obsi].textDataList[datai].tbtime=it->tbtime().isoTime().c_str();
	data[obsi].textDataList[datai].typeID_=it->typeID();
	
	datai++;
      }

      if(datai!=itd->textDataList.size()){
	LOGERROR("Datafunc (CTOR): Inconsistent size, textDataList!");
	data[obsi].textDataList.length(datai);
      }

    }else{
      data[obsi].textDataList.length(0);
    }


    if(hasData)
      obsi++;
  }

  data.length(obsi);
}

void 
DataFunc::func(KvDataSubscriberPtr ptr)
{
 using namespace CKvalObs::CService;

 if(!checkStatusAndQc(ptr)){
   return;
 }
  
 try{
    kvDataSubscriber_var ref=ptr->subscriber();
    ref->callback(data);
    ptr->connection(true);
  }
  catch(CORBA::TRANSIENT &ex){
    ptr->connection(false, true);
    LOGERROR("EXCEPTION: (timeout?) Can't send <Data> event to subscriber!" <<
	     endl << "Subscriberid: " << ptr->subscriberid() << ">!");
  }
  catch(...){
     ptr->connection(false);
    LOGERROR("EXCEPTION: Can't send <Data> event to subscriber!" <<
	     endl << "Subscriberid: " << ptr->subscriberid() << ">!");
  }
}