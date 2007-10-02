/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: WhichDataHelper.h,v 1.1.6.2 2007/09/27 09:02:44 paule Exp $                                                       

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
#ifndef __kvservice_whichdatahelper__
#define __kvservice_whichdatahelper__

#include <kvskel/kvService.hh>
#include <puTools/miTime>

namespace kvservice{

  class WhichDataHelper{

    // enum StatusId { All, OnlyFailed, OnlyOk };

    CKvalObs::CService::WhichDataList whichData_;
    CKvalObs::CService::StatusId  statusId_;

  public:
    WhichDataHelper(CKvalObs::CService::StatusId status=CKvalObs::CService::All);
    WhichDataHelper(const CKvalObs::CService::WhichDataList &wd); 
      
    WhichDataHelper(const WhichDataHelper &wd);
    
    
    ~WhichDataHelper();
    
    WhichDataHelper& operator=(const WhichDataHelper &wd);
    
    bool addStation(long stationid, 
		    const miutil::miTime &from,
		    const miutil::miTime &to=miutil::miTime());
    
    const CKvalObs::CService::WhichDataList* whichData()const{ return &whichData_;}
  };
}

#endif
