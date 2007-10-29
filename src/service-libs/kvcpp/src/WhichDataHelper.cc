/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: WhichDataHelper.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include "../include/WhichDataHelper.h"

kvservice::WhichDataHelper::WhichDataHelper(
    CKvalObs::CService::StatusId status	  
    ):
    statusId_(status)
{
}
kvservice::WhichDataHelper::WhichDataHelper(
    const CKvalObs::CService::WhichDataList &wd
    )
    :whichData_(wd),statusId_(CKvalObs::CService::All)
{
}       

kvservice::WhichDataHelper::WhichDataHelper(const WhichDataHelper &wd)
    :whichData_(wd.whichData_), statusId_(wd.statusId_)
{
}

kvservice::WhichDataHelper::~WhichDataHelper()
{
}

kvservice::WhichDataHelper& 
kvservice::WhichDataHelper::operator=(const WhichDataHelper &wd)
{
    if(this!=&wd){
	whichData_=wd.whichData_;
	statusId_=wd.statusId_;
    }
    
    return *this;
}
      
 
bool 
kvservice::WhichDataHelper::addStation(long stationid, 
				       const miutil::miTime &from,
				       const miutil::miTime &to)
{
    CORBA::Long i=whichData_.length();
    
    if(from.undef())
	return false;

    whichData_.length(i+1);

    whichData_[i].stationid=stationid;
    whichData_[i].status=statusId_;
    whichData_[i].fromObsTime=from.isoTime().c_str();
    
    if(to.undef())
	whichData_[i].toObsTime=(const char*)"";
    else
	whichData_[i].toObsTime=to.isoTime().c_str();

    return true;
}
