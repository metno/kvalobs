/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSubscribeInfoHelper.h,v 1.1.6.2 2007/09/27 09:02:44 paule Exp $                                                       

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
#ifndef __kvDataSubscribeInfoHelper_h__
#define __kvDataSubscribeInfoHelper_h__

#include <kvskel/kvServiceCommon.hh>
#include <kvskel/kvService.hh>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>

namespace kvservice{

/**
 * This is a helper class that can be uses to setup the sunscriber 
 * information. Use it when you want to subscribe to data from kvalobs.
 * 
 * If you dont add stationsID to the instance, all stations are returned. 
 * If you only want a subset of station add the stationsID's you are intrested
 * in with help of the function  'addStationId(long stationID)'
 */

class KvDataSubscribeInfoHelper{
  CKvalObs::CService::DataSubscribeInfo info_;
  
 public:
  /**
   * Constructor:
   *   It takes as param which data we want to subscribe to based on
   *   the useInfo flag.  Valid values is:
   * 
   *       enum StatusId { All, OnlyFailed, OnlyOk }
   *  
   * \param id status.
   *
   */
  KvDataSubscribeInfoHelper(CKvalObs::CService::StatusId 
			            id=CKvalObs::CService::All);
  
  KvDataSubscribeInfoHelper(const KvDataSubscribeInfoHelper &info)
    :info_(info.info_)
    {
    }
  
  ~KvDataSubscribeInfoHelper();
  
  
  KvDataSubscribeInfoHelper& operator=(const KvDataSubscribeInfoHelper &rhs){
    if(this!=&rhs){
      info_=rhs.info_;
    }
    return *this;
  }
  
  CKvalObs::CService::StatusId status()const { return info_.status;}

  void                         status(CKvalObs::CService::StatusId id)
                                      { info_.status=id;}

  CKvalObs::CService::QcIdList     qc()const { return info_.qc;}
  
  bool addStationId(long stationid) ;

  /**
   * After which QC level do you want data from. Valid values
   *
   *   QcId { QC1, QC2d, QC2m, HQC },
   *
   * The default value is to callback for everey QClevel.
   */
  bool addQc(CKvalObs::CService::QcId qcId);
  
 const CKvalObs::CService::DataSubscribeInfo* getDataSubscribeInfo()const 
                  { return &info_;}
  
  friend std::ostream& operator<<(std::ostream& os, 
				  const KvDataSubscribeInfoHelper &c);
};
}
#endif
