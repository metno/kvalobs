/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvStationParam.cc,v 1.8.6.2 2007/09/27 09:02:31 paule Exp $

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
#include <kvalobs/kvStationParam.h>
#include <miutil/timeconvert.h>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

std::string kvalobs::kvStationParam::toSend() const {
  ostringstream ost;
  ost << "(" << stationid_ << "," << paramid_ << "," << level_ << "," << sensor_
      << "," << fromday_ << "," << today_ << "," << hour_ << "," << quoted(qcx_)
      << "," << quoted(metadata_) << "," << quoted(descMetadata_) << ","
      << quoted(fromtime_) << ")";
  return ost.str();
}

bool kvalobs::kvStationParam::set(int stationid__, int paramid__, int level__,
                                  int sensor__, int fromday__, int today__,
                                  int hour__, const std::string& qcx__,
                                  const std::string& metadata__,
                                  const std::string& descMetadata__,
                                  const boost::posix_time::ptime& fromtime__) {
  stationid_ = stationid__;
  paramid_ = paramid__;
  level_ = level__;
  sensor_ = sensor__;
  fromday_ = fromday__;
  today_ = today__;
  hour_ = hour__;
  qcx_ = qcx__;
  metadata_ = metadata__;
  descMetadata_ = descMetadata__;
  fromtime_ = fromtime__;
  sortBy_ = boost::lexical_cast<std::string>(stationid_)
      + boost::lexical_cast<std::string>(paramid_);
  return true;
}

bool kvalobs::kvStationParam::set(const dnmi::db::DRow& r_) {
  dnmi::db::DRow &r = const_cast<dnmi::db::DRow&>(r_);
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];
      if (*it == "stationid") {
        stationid_ = atoi(buf.c_str());
      } else if (*it == "paramid") {
        paramid_ = atoi(buf.c_str());
      } else if (*it == "level") {
        level_ = atoi(buf.c_str());
      } else if (*it == "sensor") {
        sensor_ = atoi(buf.c_str());
      } else if (*it == "fromday") {
        fromday_ = atoi(buf.c_str());
      } else if (*it == "today") {
        today_ = atoi(buf.c_str());
      } else if (*it == "hour") {
        hour_ = atoi(buf.c_str());
      } else if (*it == "qcx") {
        qcx_ = buf;
      } else if (*it == "metadata") {
        metadata_ = buf;
      } else if (*it == "desc_metadata") {
        descMetadata_ = buf;
      } else if (*it == "fromtime") {
        fromtime_ = boost::posix_time::time_from_string_nothrow(buf);
      }
    } catch (...) {
      CERR("kvStationParam: exception ..... \n");
    }
  }

  sortBy_ = boost::lexical_cast<std::string>(stationid_)
      + boost::lexical_cast<std::string>(paramid_);
  return true;
}

std::string kvalobs::kvStationParam::uniqueKey() const {
  ostringstream ost;

  ost << " WHERE stationid=" << stationid_ << " AND " << "       paramid="
      << paramid_ << " AND " << "       level=" << level_ << " AND "
      << "       sensor=" << quoted(sensor_) << " AND " << "       fromday="
      << fromday_ << " AND " << "       today=" << today_ << " AND "
      << "       hour=" << hour_ << " AND " << "       qcx=" << quoted(qcx_)
      << " AND " << "       fromtime=" << quoted(fromtime_);

  return ost.str();

}

