/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSubscribeInfoHelper.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include <ostream>
#include "kvDataSubscribeInfoHelper.h"

using namespace CKvalObs::CService;



namespace kvservice{

KvDataSubscribeInfoHelper::KvDataSubscribeInfoHelper(
	    CKvalObs::CService::StatusId id)
{
  info_.status=id;
}
  
KvDataSubscribeInfoHelper::~KvDataSubscribeInfoHelper()
{
}
  
  
bool 
KvDataSubscribeInfoHelper::addStationId(long stationid)
{
  CORBA::Long index=info_.ids.length();

  try{
    info_.ids.length(index+1);
    info_.ids[index]=stationid;
  }
  catch(...){
    return false;
  }

  return true;
}
  
bool 
KvDataSubscribeInfoHelper::addQc(CKvalObs::CService::QcId qcId)
{
  for(CORBA::ULong i=0; i<info_.qc.length(); i++){
    if(info_.qc[i]==qcId)
      return true;
  }

  try{
    info_.qc.length(info_.qc.length()+1);
    info_.qc[info_.qc.length()-1]=qcId;
  }
  catch(...){
    return false;
  }

  return true;
}
  


std::ostream& 
operator<<(std::ostream& os, 
	   const KvDataSubscribeInfoHelper &c)
{
  os << "KvDataSubscribeInfo: status="; 

  switch(c.info_.status){
  case CKvalObs::CService::All:        os << "All";        break;
  case CKvalObs::CService::OnlyFailed: os << "OnlyFailed"; break;
  case CKvalObs::CService::OnlyOk:     os << "OnlyOk";     break;
  default:
    os << "Unknown";
  }

  os << "  QcId: ";

  if(c.info_.qc.length()>0){
    for(CORBA::ULong i=0; i<c.info_.qc.length(); i++){
      switch(c.info_.qc[i]){
      case CKvalObs::CService::QC1:  os << " QC1";  break;
      case CKvalObs::CService::QC2d: os << " QC2d"; break;
      case CKvalObs::CService::QC2m: os << " QC2m"; break;
      case CKvalObs::CService::HQC:  os << " HQC";  break;
      default:
	os << "Unknown";
      }
    }
  }else
    os << "QCAll";

  os << "  Stations:\n";
  if(c.info_.ids.length()>0){
    for(CORBA::ULong i=0; i<c.info_.ids.length(); i++){
      os << "    id: " << c.info_.ids[i] << std::endl;
    }
  }else
    os << "    All stations!";

  return os;
}

}
