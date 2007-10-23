/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSubscriberInfo.h,v 1.1.6.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __kvDataSubscriberInfo_h__
#define __kvDataSubscriberInfo_h__

#include <kvskel/kvServiceCommon.hh>
#include <kvskel/kvService.hh>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>

namespace kvalobs{

  class KvDataSubscriberInfo{
    //Definitions from kvService.hh (kvService.idl)
    //enum StatusId { All, OnlyFailed, OnlyOk };
    //enum QcId { QC1, QC2d, QC2m, HQC };
    CKvalObs::CService::StatusId   status_;
    CKvalObs::CService::QcIdList   qc_;

  public:
    KvDataSubscriberInfo()
      :status_(CKvalObs::CService::All)
      {
      }

    KvDataSubscriberInfo(const CKvalObs::CService::StatusId       status,
			 const CKvalObs::CService::QcIdList       qc)
      :status_(status), qc_(qc)
      {
      }
    
    KvDataSubscriberInfo(const KvDataSubscriberInfo &info)
      :status_(info.status_), qc_(info.qc_)
      {
      }

    ~KvDataSubscriberInfo(){}
    

    KvDataSubscriberInfo& operator=(const KvDataSubscriberInfo &rhs){
                                   if(this!=&rhs){
				     status_=rhs.status_;
				     qc_=rhs.qc_;
				   }
				   return *this;
                                 }
    
    CKvalObs::CService::StatusId status()const { return status_;}
    CKvalObs::CService::QcIdList     qc()const{ return qc_;}
    bool                          qcAll()const{ return qc_.length()==0;}
    bool                          hasQc(CKvalObs::CService::QcId qc)const; 

    friend std::ostream& operator<<(std::ostream& os, 
				    const KvDataSubscriberInfo &c);
  };

  std::ostream& operator<<(std::ostream& os, const KvDataSubscriberInfo &c);
};


#endif
