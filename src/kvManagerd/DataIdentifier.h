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

#ifndef SRC_KVMANAGERD_DATAIDENTIFIER_H_
#define SRC_KVMANAGERD_DATAIDENTIFIER_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <iosfwd>
#include <string>

namespace dnmi {
namespace db {
class DRow;
}
}

/**
 * A unique identifier for an observation, consisting of stationid, typeid and obstime
 */
class DataIdentifier {
 public:
  typedef boost::posix_time::ptime Time;

  /**
   * Read stationid, typeid, obstime from any row from any query
   */
  explicit DataIdentifier(dnmi::db::DRow & row);

  DataIdentifier(int station, int type, const Time &obstime);

  int station() const {
    return station_;
  }

  int type() const {
    return type_;
  }

  const Time & obstime() const {
    return obstime_;
  }

  std::string sqlWhere(const std::string & identifier = std::string()) const;

 private:
  int station_;
  int type_;
  Time obstime_;
};

std::ostream & operator <<(std::ostream & s, const DataIdentifier & di);

#endif /* SRC_KVMANAGERD_DATAIDENTIFIER_H_ */
