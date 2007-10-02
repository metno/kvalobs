/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CheckedDataCommand.cc,v 1.14.6.1 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <kvDbGate.h>
#include <CheckedDataCommand.h>
#include <milog.h>

namespace{
  const unsigned char QC1_mask=0x01;
  const unsigned char QC2d_mask=0x02;
  const unsigned char QC2m_mask=0x04;
  const unsigned char HQC_mask=0x08;
};


using namespace std;

CheckedDataCommand::
CheckedDataCommand(const CKvalObs::StationInfoList &stInfo, 
		   ManagerApp &app_)
  :kvalobs::StationInfoCommand(stInfo), app(app_), dbCon(0)
{
}
  

void 
CheckedDataCommand::callDataNotifySubscribers(
    kvalobs::kvStationInfoList &stationInfoList
    )
{
  long                        stationID;
  kvalobs::IkvStationInfoList it=stationInfoList.begin();
  //  CERR("CheckedDataCommand::callDataNotifySubscribers  Data from kvQaBase!\n");  
  
  if(!app.subscribers.hasDataNotifySubscribers())
      return;

  for(;it!=stationInfoList.end(); it++){
    DataNotifyFunc dataNotify(*it);
    stationID=it->stationID();
    LOGDEBUG("CALL DataNotifySubscribers: stationID(" << stationID <<")\n" << *it);

    app.subscribers.forAllDataNotifySubscribers(dataNotify, stationID);
  }

}

void 
CheckedDataCommand::callDataSubscribers(
    kvalobs::kvStationInfoList &stationInfoList
    )
{
    long stationID;
    
    std::list<kvalobs::kvData>  dataList;
    kvalobs::IkvStationInfoList it=stationInfoList.begin();

    if(!app.subscribers.hasDataSubscribers())
	return;

    if(!dbCon){
      LOGERROR("CheckedDataCommand: dbCon==0!");
      return;
    }

    kvalobs::kvDbGate gate(dbCon);

    for(;it!=stationInfoList.end() && !app.shutdown(); it++){
      if(gate.select(dataList,
		     kvQueries::selectDataFromType(it->stationID(),
						   it->typeID(),
						   it->obstime())))
	{
	  DataFunc dataf(dataList);
	  
	  if(dataList.empty())
	    continue;
	  
	  stationID=it->stationID();
	  LOGDEBUG("CALL DataSubscribers: stationID: "<< it->stationID() 
		   << " obstime: " << it->obstime() 
		   << " typeID: " << it->typeID());
	  app.subscribers.forAllDataSubscribers(dataf, stationID);
	}
    }
}

 
bool       
CheckedDataCommand::executeImpl()
{
    callDataNotifySubscribers(getStationInfo());
    callDataSubscribers(getStationInfo());

    return true;
} 

void       
CheckedDataCommand::debugInfo(std::iostream &info)
{
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
  }
  catch(...){
    LOGERROR("EXCEPTION: Can't send <DataNotify> event to subscriber!" <<
	     endl << "Subscriberid: " << ptr->subscriberid() << ">!");
  }
}

bool
DataNotifyFunc::buildWhatList(
      CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl
      )
{
  unsigned char qcLevel=0x00;
  unsigned char flag;
  char b[100];
  CORBA::Long i=0;
  CORBA::Long wli=0;
  kvalobs::CIkvParamInfoList it;

  wl.length(wli+1);
  wl[wli].stationID=stationInfo.stationID();
  wl[wli].typeID=stationInfo.typeID();
  wl[wli].obsTime=stationInfo.obstime().isoTime().c_str();

  it=stationInfo.getParams().begin();

  for(;it!=stationInfo.getParams().end(); it++){
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
  kvalobs::CIkvParamInfoList it;
  unsigned char flag;

  if(!ptr->subscriberInfo().qcAll()){
    //Check if the subscriber is interested in the qcLevels that
    //is set for this observation. We return true if we find a
    //qcLevel that match for any parameter in the observation.
    bool qcLevel=false;

    it=stationInfo.getParams().begin();

    for(;it!=stationInfo.getParams().end() && !qcLevel; it++){
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
    
    it=stationInfo.getParams().begin();

    for(;it!=stationInfo.getParams().end() && !hasFailed ; it++){
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
    std::list<kvalobs::kvData>::const_iterator it;
    unsigned char flag;
    
    if(!ptr->subscriberInfo().qcAll()){
	//Check if the subscriber is interested in the qcLevels that
	//is set for this observation. We return true if we find a
	//qcLevel that match for any parameter in the observation.
	bool qcLevel=false;

	it=dataList.begin();

	for(;it!=dataList.end() && !qcLevel; it++){
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
	
	it=dataList.begin();

	for(;it!=dataList.end() && !hasFailed ; it++){
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
DataFunc::fillDataList(CKvalObs::CService::ObsDataList &dl)
{

    std::list<kvalobs::kvData>::iterator it=dataList.begin();
    miutil::miTime         thisTime;
    CORBA::Long            obsi=dl.length();
    CORBA::Long            datai=0;
    char                   *sTmp;

    if(it==dataList.end())
	return true;
   
    dl.length(obsi+1);
    thisTime=it->obstime();

    while(it!=dataList.end()){
	if(it->obstime()!=thisTime){
	    //New record
	    obsi++;
	    datai=0;
	    dl.length(obsi+1);
	}
     
	dl[obsi].dataList.length(datai+1);
	dl[obsi].dataList[datai].stationID=it->stationID(); 
	dl[obsi].dataList[datai].obstime=it->obstime().isoTime().c_str();
	dl[obsi].dataList[datai].original=it->original();
	dl[obsi].dataList[datai].paramID=it->paramID();
	dl[obsi].dataList[datai].tbtime=it->tbtime().isoTime().c_str();
	dl[obsi].dataList[datai].typeID=it->typeID();
	
	sTmp=CORBA::string_alloc(2);
	
	if(sTmp){
	    sTmp[0]=it->sensor();
	    sTmp[1]='\0';
	    dl[obsi].dataList[datai].sensor=sTmp;
	}
	
	dl[obsi].dataList[datai].level=it->level();
	dl[obsi].dataList[datai].corrected=it->corrected();
	dl[obsi].dataList[datai].controlinfo=it->controlinfo().flagstring().c_str();
	dl[obsi].dataList[datai].useinfo=it->useinfo().flagstring().c_str();
	dl[obsi].dataList[datai].cfailed=it->cfailed().c_str();
	
	datai++;
	it++; //Move to the next data in dataList.
    }
    
    return true;
}

void 
DataFunc::func(KvDataSubscriberPtr ptr)
{
 using namespace CKvalObs::CService;

 ObsDataList data;

 if(!checkStatusAndQc(ptr)){
   return;
  }
  
  if(!fillDataList(data)){
      LOGERROR("DataFunc::func: OUT OF MEMMORY!\n");
      return;
  }
	  
  try{
    CKvalObs::CService::kvDataSubscriber_var ref=ptr->subscriber();
    
    ref->callback(data);
  }
  catch(...){
    LOGERROR("EXCEPTION: Can't send <Data> event to subscriber!" <<
	     endl << "Subscriberid: " << ptr->subscriberid() << ">!");
  }
    
}
