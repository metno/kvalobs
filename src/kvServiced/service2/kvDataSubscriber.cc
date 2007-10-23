/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSubscriber.cc,v 1.1.2.5 2007/09/27 09:02:40 paule Exp $                                                       

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
#include <iostream>
#include "ServiceApp.h"
#include "kvDataSubscriber.h"

using namespace std;
using namespace kvalobs;

KvDataSubscriber::
~KvDataSubscriber()
{ 
  cerr <<"DELETE: KvDataSubscriberInfo: subscriberid: " 
       << subscriberid() << endl;
}


KvDataSubscriber::DataHelper* 
KvDataSubscriber::
obsDataList(DataToSubscribersPtr dp)
{
  CORBA::Long datai;
  char        *sTmp;
  char        buf[64];
  bool        hasData;

  DataHelper  *data;
 
  if(dp->dataList.empty() && dp->textDataList.empty())
    return 0;

  try{
    data=new DataHelper(dp->stationid, dp->typeid_, dp->obstime);
  }
  catch(...){
    LOGERROR("NOMEM: KvDataSubscriber::DataHelper!");
    return 0;
  }
  
  CKvalObs::CService::ObsDataList &d=data->data;
  d.length(1);

  hasData=false;
  d[0].stationid=dp->stationid;

  if(dp->dataList.empty()){
    d[0].dataList.length(0);
  }else{
    d[0].dataList.length(dp->dataList.size());
    datai=0;

    for(list<kvalobs::kvData>::const_iterator it=dp->dataList.begin();
	it!=dp->dataList.end(); 
	it++) {
      hasData=true;
      d[0].dataList[datai].stationID=it->stationID(); 
      d[0].dataList[datai].obstime  =it->obstime().isoTime().c_str();
      d[0].dataList[datai].original =it->original();
      d[0].dataList[datai].paramID  =it->paramID();
      d[0].dataList[datai].tbtime   =it->tbtime().isoTime().c_str();
      d[0].dataList[datai].typeID_  =it->typeID();
      
      sprintf(buf, "%d", it->sensor()); 
      sTmp=CORBA::string_dup(buf);
      
      if(sTmp){
	d[0].dataList[datai].sensor=sTmp;
      }else{
	LOGERROR("DataFunc (CTOR): NOMEM for <kvData::sensor>!");
      }
      
      d[0].dataList[datai].level=it->level();
      d[0].dataList[datai].corrected=it->corrected();
      d[0].dataList[datai].controlinfo=it->controlinfo().flagstring().c_str();
      d[0].dataList[datai].useinfo=it->useinfo().flagstring().c_str();
      d[0].dataList[datai].cfailed=it->cfailed().c_str();
      
      datai++;
    }
    
    if(datai!=dp->dataList.size()){
      LOGERROR("Inconsistent size, dataList!");
      d[0].dataList.length(datai);
    }
  }
  
  if(dp->textDataList.empty()){
    d[0].textDataList.length(0);
  }else{
    datai=0;
    d[0].textDataList.length(dp->textDataList.size());

    for(list<kvalobs::kvTextData>::const_iterator it=dp->textDataList.begin();
	it!=dp->textDataList.end();
	it++) {
      hasData=true;
      
      d[0].textDataList[datai].stationID=it->stationID(); 
      d[0].textDataList[datai].obstime=it->obstime().isoTime().c_str();
      d[0].textDataList[datai].original=it->original().c_str();
      d[0].textDataList[datai].paramID=it->paramID();
      d[0].textDataList[datai].tbtime=it->tbtime().isoTime().c_str();
      d[0].textDataList[datai].typeID_=it->typeID();
      
      datai++;
    }
    
    if(datai!=dp->textDataList.size()){
      LOGERROR("Inconsistent size, textDataList!");
      d[0].textDataList.length(datai);
    }
  }
  
  return data;
}


KvDataSubscriber::DataHelper*
KvDataSubscriber::
getDataFromQue(int timeout)
{
  dnmi::thread::CommandBase *cmd;

  cmd=inque_.get(timeout);
  
  if(!cmd){
    return 0;
  }
  
  DataCommand *dataCmd=dynamic_cast<DataCommand*>(cmd);

  if(!dataCmd){
    cerr << "Invalid Command!!!" << endl;
    delete cmd;
    return 0;
  }

  if(!thisStation(dataCmd->data()->stationid) ||
     !checkStatusAndQc(dataCmd->data())){
    delete dataCmd;
    return 0;
  }
      
  DataHelper *data=obsDataList(dataCmd->data());
  delete dataCmd; //We dont need it anymore
  
  if(!data){
    LOGWARN("UNEXPECTED: No Data!");
  }
   
  return data;
}


void 
KvDataSubscriber::
put(DataCommand *cmd)
{ 
  try{
    inque_.postAndBrodcast(cmd);
  }catch(...){
  }
}




int 
KvDataSubscriber::
run()
{
  milog::LogContext cntxt(subscriberid());
  DataHelper *data=0;
  time_t     retry;
  time_t     maxRetry=0;
  time_t     tick;

  time(&tick);

  while(!terminate_ && !app_.shutdown() && maxRetry<tick){
    if(!data){
      data=getDataFromQue(2);
      
      if(!data)
	continue;
    }else{
      //We have a left over dataset that is not sendt to the subscriber
      sleep(2);
      
      time(&tick);
      
      if(tick<retry)
	continue;
    }
    
    try{
      CKvalObs::CService::kvDataSubscriber_var ref=subscriber();
      ref->callback(data->data);
      LOGINFO("SUCCESS: CALL Stationid: " << data->stationid << " typeid: " << 
	      data->typeid_ << " obstime: " << data->obstime);
      
      maxRetry=0;
      delete data;
      data=0;
    }
    catch(CORBA::TRANSIENT &ex){
      setretry();
      LOGERROR("EXCEPTION: (timeout?) Can't send <Data> event to subscriber!" 
	       << endl << "Subscriberid: " << subscriberid() << ">!");
    }
    catch(...){
      setretry();
      LOGERROR("EXCEPTION: Can't send <Data> event to subscriber!" <<
	       endl << "Subscriberid: " << subscriberid() << ">!");
    }

    time(&tick);
  }
  
  inque_.suspend();
  inque_.clear();
  
  if(terminate_){
    LOGINFO("Terminate: Unsubscribed!");
    return 1;
  }

  if(maxRetry>0){
    LOGWARN("TERMINATE: The subscriber is NOT responding!");
    return 2;
  }
  
  LOGINFO("Terminate: Normal exit!");

  return 0;
}

    


