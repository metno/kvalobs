/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataNotifySubscriber.cc,v 1.1.2.5 2007/09/27 09:02:40 paule Exp $                                                       

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
#include <iostream>
#include <milog/milog.h>
#include <kvalobs/kvData.h>
#include "ServiceApp.h"
#include "kvDataNotifySubscriber.h"

using namespace std;
using namespace kvalobs;

KvDataNotifySubscriber::
~KvDataNotifySubscriber()
{ 
  cerr <<"DELETE: KvDataSubscriberInfo: subscriberid: " 
       << subscriberid() << endl;
}



void 
KvDataNotifySubscriber::
put(DataCommand *cmd)
{ 
  try{
    inque_.postAndBrodcast(cmd);
  }catch(...){
  }
}

void
KvDataNotifySubscriber::
buildWhatList(CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl,
	      DataToSubscribersPtr d)
{
  unsigned char        qcLevel=0x00;
  unsigned char        flag;
  char                 b[100];
  CORBA::Long          i=0;
  CORBA::Long          wli=0;
  list<kvData>  &dataList=d->dataList;
  //  kvalobs::CIkvParamInfoList it;

  wl.length(wli+1);
  wl[wli].stationID=d->stationid;
  wl[wli].typeID_=d->typeid_;
  wl[wli].obsTime=d->obstime.isoTime().c_str();


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

}


KvDataNotifySubscriber::WhatHelper*
KvDataNotifySubscriber::
getDataFromQue(int timeout)
{
  dnmi::thread::CommandBase *cmd;
  
  cmd=inque_.get(timeout);

  if(!cmd)
    return 0;

  DataCommand *dataCmd=dynamic_cast<DataCommand*>(cmd);

  if(!dataCmd){
    LOGWARN("Invalid Command!!!");
    delete cmd;
    return 0;
  }

  if(!thisStation(dataCmd->data()->stationid) ||
     !checkStatusAndQc(dataCmd->data())){
    delete dataCmd;
    return 0;
  }

  WhatHelper *what;
  
  try{
    what=new WhatHelper(dataCmd->data()->stationid,
			dataCmd->data()->typeid_,
			dataCmd->data()->obstime);
  }
  catch(...){
    delete dataCmd;
    LOGERROR("NOMEM: KvDataNotify::getDataFromQue()!");
    return 0;
  }


  buildWhatList(what->wl, dataCmd->data());
  delete dataCmd; //We dont need it anymore
  
  return what;
}


int 
KvDataNotifySubscriber::
run()
{
  milog::LogContext cntxt(subscriberid());
  WhatHelper *what;

  while(!terminate_ && !app_.shutdown()){
    
    if(!what){
      what=getDataFromQue(2);
      
      if(!what)
	continue;
    }else{
      //We have a left over datanotify that is not sendt to the subscriber
      sleep(2);
      
      time(&tick);
      
      if(tick<retry)
	continue;
    }

    try{
      CKvalObs::CService::kvDataNotifySubscriber_var ref=subscriber();
      ref->callback(what->wl);
      LOGINFO("SUCCESS: CALL Stationid: " << stationid << " typeid: " << 
	      typeid_ << " obstime: " << obstime);
      
      delete what;
      what=0;
      maxRetry=0;
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

    


