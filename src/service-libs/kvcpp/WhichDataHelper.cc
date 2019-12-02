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
#include "WhichDataHelper.h"
#include <miutil/timeconvert.h>

kvservice::WhichDataHelper::WhichDataHelper(CKvalObs::CService::StatusId status)
    : statusId_(status) {
}
kvservice::WhichDataHelper::WhichDataHelper(
    const CKvalObs::CService::WhichDataList &wd)
    : whichData_(wd),
      statusId_(CKvalObs::CService::All) {
}

kvservice::WhichDataHelper::WhichDataHelper(const WhichDataHelper &wd)
    : whichData_(wd.whichData_),
      statusId_(wd.statusId_) {
}

kvservice::WhichDataHelper::~WhichDataHelper() {
}

kvservice::WhichDataHelper&
kvservice::WhichDataHelper::operator=(const WhichDataHelper &wd) {
  if (this != &wd) {
    whichData_ = wd.whichData_;
    statusId_ = wd.statusId_;
  }

  return *this;
}

bool kvservice::WhichDataHelper::addStation(
    long stationid, const boost::posix_time::ptime &from,
    const boost::posix_time::ptime &to) {
  CORBA::Long i = whichData_.length();

  if (from.is_not_a_date_time())
    return false;

  if( i != 0 && (stationid == 0 || whichData_[0].stationid==0)) {
    boost::posix_time::ptime f=from;
    boost::posix_time::ptime t=to;

    for( i=0; i<whichData_.length(); ++i) {
      //Try to merge all obstime such that it span the full range.
      //If toObsTime is undefined it means all data from fromObsTime.
      const char *sFrom=whichData_[i].fromObsTime;
      auto tmpFrom = boost::posix_time::time_from_string_nothrow(sFrom);
      auto tmpTo = boost::posix_time::ptime();

      if (whichData_[i].toObsTime[0] != '\0') {
        const char *sTo=whichData_[i].toObsTime;
        tmpTo = boost::posix_time::time_from_string_nothrow(sTo);
      }

      if( ! tmpFrom.is_not_a_date_time() && tmpFrom < f )
        f=tmpFrom;

      if( ! (tmpTo.is_not_a_date_time() || t.is_not_a_date_time()) && tmpTo > t )
        t=tmpTo;
      else
        t = boost::posix_time::ptime();
    }

    whichData_[0].stationid = 0;
    whichData_[0].status = statusId_;
    whichData_[0].fromObsTime = to_kvalobs_string(f).c_str();

    if (t.is_not_a_date_time())
      whichData_[0].toObsTime = (const char*) "";
    else
       whichData_[0].toObsTime = to_kvalobs_string(to).c_str();

    whichData_.length(1);
    return true;
  }

  whichData_.length(i + 1);
  whichData_[i].stationid = stationid;
  whichData_[i].status = statusId_;
  whichData_[i].fromObsTime = to_kvalobs_string(from).c_str();

  if (to.is_not_a_date_time())
    whichData_[i].toObsTime = (const char*) "";
  else
    whichData_[i].toObsTime = to_kvalobs_string(to).c_str();

  return true;
}
