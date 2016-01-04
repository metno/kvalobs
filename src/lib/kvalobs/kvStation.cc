/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvStation.cc,v 1.14.6.3 2007/09/27 09:02:31 paule Exp $

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
#include <kvalobs/kvStation.h>
#include <miutil/timeconvert.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

std::string kvalobs::kvStation::toSend() const {
  ostringstream ost;
  ost << "(" << stationid_ << ",";

  if (lat_ == kvDbBase::FLT_NULL)
    ost << "NULL,";
  else
    ost << lat_ << ",";

  if (lon_ == kvDbBase::FLT_NULL)
    ost << "NULL,";
  else
    ost << lon_ << ",";

  if (height_ == kvDbBase::FLT_NULL)
    ost << "NULL,";
  else
    ost << height_ << ",";

  if (maxspeed_ == kvDbBase::FLT_NULL)
    ost << "NULL,";
  else
    ost << maxspeed_ << ",";

  if (name_ == kvDbBase::TEXT_NULL)
    ost << "NULL,";
  else
    ost << quoted(name_) << ",";

  if (wmonr_ == kvDbBase::INT_NULL)
    ost << "NULL,";
  else
    ost << wmonr_ << ",";

  if (nationalnr_ == kvDbBase::INT_NULL)
    ost << "NULL,";
  else
    ost << nationalnr_ << ",";

  if (ICAOid_ == kvDbBase::TEXT_NULL)
    ost << "NULL,";
  else
    ost << quoted(ICAOid_) << ",";

  if (call_sign_ == kvDbBase::TEXT_NULL)
    ost << "NULL,";
  else
    ost << quoted(call_sign_) << ",";

  if (stationstr_ == kvDbBase::TEXT_NULL)
    ost << "NULL,";
  else
    ost << quoted(stationstr_) << ",";

  if (environmentid_ == kvDbBase::INT_NULL)
    ost << "NULL,";
  else
    ost << quoted(environmentid_) << ",";

  ost << quoted(static_) << "," << quoted(fromtime_) << ")";

  return ost.str();
}

bool kvalobs::kvStation::set(int stationid__, float lat__, float lon__,
                             float height__, float maxspeed__,
                             const std::string& name__, int wmonr__,
                             int nationalnr__, const std::string& ICAOid__,
                             const std::string& call_sign__,
                             const std::string& stationstr__,
                             int environmentid__, bool static__,
                             const boost::posix_time::ptime& fromtime__) {

  stationid_ = stationid__;
  lat_ = lat__;
  lon_ = lon__;
  height_ = height__;
  maxspeed_ = maxspeed__;
  name_ = name__;
  wmonr_ = wmonr__;
  nationalnr_ = nationalnr__;
  ICAOid_ = ICAOid__;
  call_sign_ = call_sign__;
  stationstr_ = stationstr__;
  environmentid_ = environmentid__;
  static_ = static__;
  fromtime_ = fromtime__;
  sortBy_ = boost::lexical_cast<std::string>(stationid_);
  return true;
}

bool kvalobs::kvStation::set(const dnmi::db::DRow& r_) {
  dnmi::db::DRow &r = const_cast<dnmi::db::DRow&>(r_);
  std::string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];
      boost::trim(buf);
      if (*it == "stationid")
        stationid_ = atoi(buf.c_str());
      else if (*it == "lat")
        lat_ = atof(buf.c_str());
      else if (*it == "lon")
        lon_ = atof(buf.c_str());
      else if (*it == "height")
        height_ = atoi(buf.c_str());
      else if (*it == "maxspeed")
        maxspeed_ = atoi(buf.c_str());
      else if (*it == "name")
        name_ = buf;
      else if (*it == "wmonr")
        wmonr_ = atoi(buf.c_str());
      else if (*it == "nationalnr")
        nationalnr_ = atoi(buf.c_str());
      else if (*it == "icaoid")
        ICAOid_ = (buf.size() == 4 ? buf : "");
      else if (*it == "call_sign")
        call_sign_ = buf;
      else if (*it == "stationstr")
        stationstr_ = buf;
      else if (*it == "environmentid")
        environmentid_ = atoi(buf.c_str());
      else if (*it == "static")
        static_ = (buf == "t");
      else if (*it == "fromtime")
        fromtime_ = boost::posix_time::time_from_string_nothrow(buf);
    } catch (...) {
      CERR("kvStation: exception ..... \n");
    }
  }
  sortBy_ = boost::lexical_cast<std::string>(stationid_);
  return true;
}

std::string kvalobs::kvStation::uniqueKey() const {
  ostringstream ost;

  ost << " WHERE  stationid=" << stationid_ << " AND " << "        fromtime="
      << quoted(fromtime_);

  return ost.str();
}

