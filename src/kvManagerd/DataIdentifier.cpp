/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2016 met.no

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

#include "DataIdentifier.h"
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <string>
#include "kvdb/kvdb.h"

DataIdentifier::DataIdentifier(dnmi::db::DRow & row) {
  station_ = boost::lexical_cast<int>(row["stationid"]);
  type_ = boost::lexical_cast<int>(row["typeid"]);
  obstime_ = boost::posix_time::time_from_string(row["obstime"]);
}

DataIdentifier::DataIdentifier(int station, int type, const Time &obstime)
    : station_(station),
      type_(type),
      obstime_(obstime) {
}

std::string DataIdentifier::sqlWhere(const std::string & identifier) const {
  std::string prefix;
  if (!identifier.empty())
    prefix = identifier + ".";
  std::ostringstream s;
  s << prefix << "stationid=" << station_ << " and ";
  s << prefix << "obstime='" << obstime_ << "' and ";
  s << prefix << "typeid=" << type_;
  return s.str();
}

std::ostream & operator <<(std::ostream & s, const DataIdentifier & di) {
  return s << di.station() << '/' << di.type() << '/' << di.obstime();
}
