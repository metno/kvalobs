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

#include "MissingRunner.h"
#include <boost/date_time.hpp>
#include <chrono>
#include <exception>
#include <mutex>
#include <string>
#include <thread>
#include "miutil/timeconvert.h"
#include "milog/milog.h"
#include "kvdb/kvdb.h"
#include "DataIdentifier.h"
#include "KvalobsDatabaseAccess.h"


MissingRunner::MissingRunner(const std::string & connectString, bool checkForMissingObs_)
    : TimedDatabaseTask(connectString, "MissingRunner"),
      checkForMissingObs(checkForMissingObs_) {
}

MissingRunner::~MissingRunner() {
}

MissingRunner::Time MissingRunner::nextRunTime() const {
  auto t = std::chrono::system_clock::now();
  t += std::chrono::minutes(30);
  t = std::chrono::time_point_cast < std::chrono::hours > (t);
  t += std::chrono::minutes(30);
  return t;
}

using boost::posix_time::ptime;
using boost::posix_time::hours;
using boost::posix_time::minutes;
using boost::posix_time::second_clock;
using boost::posix_time::to_kvalobs_string;

void MissingRunner::run() {
  if (!checkForMissingObs) {
    LOGINFO("Locating missing observations 'disabled'. (" << second_clock::universal_time() << ")");
    return;
  }
  LOGINFO("Locating missing observations");

  KvalobsDatabaseAccess dbAccess(connectString());

  ptime obstime = dbAccess.lastFindAllMissingRuntime() + hours(1);
  while (obstime < boost::posix_time::microsec_clock::universal_time() && !stopped()) {
    LOGINFO("Locating missing observations at " << obstime);
    try {
      addAllMissingData(dbAccess, obstime);
      dbAccess.setLastFindAllMissingRuntime(obstime);
      obstime += hours(1);
    } catch (dnmi::db::SQLSerializeError & e) {
      LOGWARN(e.what() << "  - retrying");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

void MissingRunner::addAllMissingData(KvalobsDatabaseAccess & dbAccess, const boost::posix_time::ptime & obstime) {
  auto missing = dbAccess.findAllMissing(obstime);
  LOGINFO("Number of missing stations: " << missing.size() << " obstime: " << to_kvalobs_string(obstime));
  for (const DataIdentifier & di : missing) {
    bool retry=false;
    do{
      try {
        auto transaction = dbAccess.transaction(true);
        dbAccess.addMissingData(di);
        transaction->commit();
        retry=false;
      }
      catch(const dnmi::db::SQLSerializeError & e) {
        LOGWARN(e.what() << "  - retrying");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        retry=true;
      }
      catch(const std::exception &e) {
        LOGERROR("DB error: " << e.what());
      }
    } while (retry);
  }
}
