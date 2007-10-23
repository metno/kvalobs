/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSubscriberInfo.cc,v 1.1.2.6 2007/09/27 09:02:40 paule Exp $                                                       

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
#include "ServiceApp.h"
#include "SubscriberCommandBase.h"
#include "kvDataSubscriberInfo.h"

using namespace std;
using namespace CKvalObs::CService;

const unsigned char KvDataSubscriberInfo::QC1_mask=0x01;
const unsigned char KvDataSubscriberInfo::QC2d_mask=0x02;
const unsigned char KvDataSubscriberInfo::QC2m_mask=0x04;
const unsigned char KvDataSubscriberInfo::HQC_mask=0x08;

KvDataSubscriberInfo::
KvDataSubscriberInfo(const CKvalObs::CService::DataSubscribeInfo &info,
		     ServiceApp                                  &app)
  :terminate_(false), 
   app_(app),
   retry(0),
   maxRetry(0),
   tick(0)

{
  if(info.qc.length()>0){
    qc_=vector<QcId>(info.qc.length());

    for(vector<QcId>::size_type i=0; i<info.qc.length(); i++)
      qc_[i]=info.qc[i];
  }

  if(info.ids.length()>0){
    stations_=vector<int>(info.ids.length());
    
    for(vector<int>::size_type i=0; i<info.ids.length(); i++)
      stations_[i]=info.ids[i];

  }
}



bool 
KvDataSubscriberInfo::
checkStatusAndQc(DataToSubscribersPtr d)
{
  unsigned char                              flag;
  const std::list<kvalobs::kvData>           &data=d->dataList;
  std::list<kvalobs::kvData>::const_iterator it;

  if(!qcAll()){
    //Check if the subscriber is interested in the qcLevels that
    //is set for this observation. We return true if we find a
    //qcLevel that match for any parameter in the observation.
    bool qcLevel=false;
    
    for(it=data.begin(); it!=data.end() && !qcLevel; it++){
      flag=it->controlinfo().cflag(0);

      for(CITQc qcit=qcbegin(); qcit!=qcend();  qcit++){
	if(fqcLevel(*qcit, flag)){
	  qcLevel=true;
	  break;
	}
      }
    }
      
    if(!qcLevel)
      return false;
  }

  
  
  if(status()!=CKvalObs::CService::All){
    bool hasFailed=false;
    
    for(it=data.begin(); it!=data.end() && !hasFailed; it++){
      flag=it->useinfo().cflag(0);
	
	//Check if bit 1 in useinfo is set. This bit is set to 1 if
	//the value is useless.
	//CODE: Check if this select the correct bit.
	if(flag & 0x01) 
	  hasFailed=true;
    }
     
    if(hasFailed &&  status()==CKvalObs::CService::OnlyFailed)
      return true;
    else if(!hasFailed && status()==CKvalObs::CService::OnlyOk)
      return true;
    else
      return false;
  }
  
  return true;
}

bool 
KvDataSubscriberInfo::
fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag)
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
KvDataSubscriberInfo::
hasQc(CKvalObs::CService::QcId qc)const
{
  if(qcAll())
    return true;

  for(vector<QcId>::size_type i=0; i<qc_.size(); i++)
    if(qc_[i]==qc)
      return true;

  return false;
}



bool                    
KvDataSubscriberInfo::
thisStation(int stationid)const
{
  if(stations_.empty())
    return true;

  for(vector<int>::size_type i=0; i<stations_.size(); i++)
    if(stations_[i]==stationid)
      return true;

  return false;
}


void 
KvDataSubscriberInfo::
setretry()
{
  if(app_.shutdown()){
    maxRetry=0;
    return;
  }

  if(maxRetry==0){
    time(&maxRetry);
    maxRetry+=MAX_RETRY;
  }

  time(&retry);
  retry+=RETRY;
}
  


dnmi::thread::CommandBase*
KvDataSubscriberInfo::
getDataFromQue(int timeout)
{
  dnmi::thread::CommandBase *cmd;

  cmd=inque_.get(timeout);
  
  if(!cmd){
    return 0;
  }
  
  return cmd;
}

void 
KvDataSubscriberInfo::
put(dnmi::thread::CommandBase *cmd)
{
  try{
    inque_.postAndBrodcast(cmd);
  }catch(...){
  }
}

bool 
KvDataSubscriberInfo::
shutdown()
{
  return app_.shutdown();
}


std::ostream& 
operator<<(std::ostream& os, const KvDataSubscriberInfo &c)
{
  os << "KvDataSubscriberInfo: statusId: ";
  
  switch(c.status()){
  case CKvalObs::CService::All:        os << "All";        break;
  case CKvalObs::CService::OnlyFailed: os << "OnlyFailed"; break;
  case CKvalObs::CService::OnlyOk:     os << "OnlyOk";     break;
  }

  os << "  QcId: ";

  if(!c.qcAll()){
    for(vector<QcId>::size_type i=0; i<c.qc().size(); i++){
      switch(c.qc_[i]){
      case CKvalObs::CService::QC1:  os << " QC1";  break;
      case CKvalObs::CService::QC2d: os << " QC2d"; break;
      case CKvalObs::CService::QC2m: os << " QC2m"; break;
      case CKvalObs::CService::HQC:  os << " HQC";  break;
      }
    }
  }else
    os << "QCAll";

  return os;
}
