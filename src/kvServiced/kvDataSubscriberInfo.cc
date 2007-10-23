/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSubscriberInfo.cc,v 1.1.6.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#include "kvDataSubscriberInfo.h"

bool
kvalobs::KvDataSubscriberInfo::hasQc(CKvalObs::CService::QcId qc)const
{
  if(qcAll())
    return true;

  for(CORBA::Long i=0; i<qc_.length(); i++)
    if(qc_[i]==qc)
      return true;

  return false;
}



std::ostream& 
kvalobs::operator<<(std::ostream& os, const KvDataSubscriberInfo &c)
{
  os << "KvDataSubscriberInfo: statusId: ";
  
  switch(c.status()){
  case CKvalObs::CService::All:        os << "All";        break;
  case CKvalObs::CService::OnlyFailed: os << "OnlyFailed"; break;
  case CKvalObs::CService::OnlyOk:     os << "OnlyOk";     break;
  }

  os << "  QcId: ";

  if(!c.qcAll()){
    for(int i=0; i<c.qc().length(); i++){
      switch(c.qc()[i]){
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
