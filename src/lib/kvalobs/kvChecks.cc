/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvChecks.cc,v 1.14.6.2 2007/09/27 09:02:30 paule Exp $

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
#include <stdlib.h>
#include <kvalobs/kvChecks.h>
#include <dnmithread/mtcout.h>
#include <miutil/timeconvert.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace dnmi;

std::string kvalobs::kvChecks::toSend() const {
  ostringstream ost;
  ost << "(" << stationid_ << "," << quoted(qcx_) << "," << quoted(medium_qcx_)
      << "," << language_ << "," << quoted(checkname_) << ","
      << quoted(checksignature_) << "," << quoted(active_) << ","
      << quoted(fromtime_) << ")";
  return ost.str();
}

bool kvalobs::kvChecks::set(const dnmi::db::DRow &r_) {
  db::DRow &r = const_cast<db::DRow&>(r_);
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];

      if (*it == "stationid") {
        stationid_ = atoi(buf.c_str());
      } else if (*it == "qcx") {
        qcx_ = buf;
      } else if (*it == "medium_qcx") {
        medium_qcx_ = buf;
      } else if (*it == "language") {
        language_ = atoi(buf.c_str());
      } else if (*it == "checkname") {
        checkname_ = buf;
      } else if (*it == "checksignature") {
        checksignature_ = buf;
      } else if (*it == "active") {
        active_ = buf;
      } else if (*it == "fromtime") {
        fromtime_ = boost::posix_time::time_from_string_nothrow(buf);
      }
    } catch (...) {
      CERR("kvChecks: unexpected exception ..... \n");
    }
  }
  sortBy_ = boost::lexical_cast<std::string>(stationid_);

  return true;
}

bool kvalobs::kvChecks::set(int stationid, const std::string &qcx,
                            const std::string &medium_qcx, int language,
                            const std::string &checkname,
                            const std::string &checksignature,
                            const std::string &active,
                            const boost::posix_time::ptime & fromtime) {
  stationid_ = stationid;
  qcx_ = qcx;
  medium_qcx_ = medium_qcx;
  language_ = language;
  checkname_ = checkname;
  checksignature_ = checksignature;
  active_ = active;
  fromtime_ = fromtime;

  sortBy_ = boost::lexical_cast<std::string>(stationid_);

  return true;
}

std::string kvalobs::kvChecks::uniqueKey() const {
  ostringstream ost;

  ost << " WHERE stationid=" << stationid_ << " AND " << "       qcx="
      << quoted(qcx_) << " AND " << "       language=" << language_ << " AND "
      << "       fromtime=" << quoted(to_kvalobs_string(fromtime_));

  return ost.str();
}
