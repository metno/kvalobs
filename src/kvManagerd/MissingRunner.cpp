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
#include "milog/milog.h"
#include "DataIdentifier.h"
#include "KvalobsDatabaseAccess.h"

bool MissingRunner::stopped_ = false;
std::condition_variable MissingRunner::condition_;

MissingRunner::MissingRunner(const std::string & connectString)
    : connectString_(connectString) {
}

MissingRunner::~MissingRunner() {
}

namespace {
std::chrono::system_clock::time_point nextRunTime() {
  auto t = std::chrono::system_clock::now();
  t += std::chrono::minutes(30);
  t = std::chrono::time_point_cast < std::chrono::hours > (t);
  t += std::chrono::minutes(30);
  return t;
}
}

void MissingRunner::operator()() {
  milog::LogContext context("MissingRunner");

  std::mutex allMissing;
  std::unique_lock < std::mutex > lock(allMissing);

  while (!stopped()) {
    try {
      run();
    } catch (std::exception & e) {
      LOGERROR(e.what());
    }
    if (!stopped())
      condition_.wait_until(lock, nextRunTime());
  }
}

using boost::posix_time::ptime;
using boost::posix_time::hours;
using boost::posix_time::minutes;
using boost::posix_time::second_clock;

void MissingRunner::run() {
  KvalobsDatabaseAccess dbAccess(connectString_);

  ptime obstime = dbAccess.lastFindAllMissingRuntime() + hours(1);
  while (obstime < boost::posix_time::microsec_clock::universal_time()
      && !stopped()) {
    LOGINFO("Locating missing observations at " << obstime);
    addAllMissingData(dbAccess, obstime);
    dbAccess.setLastFindAllMissingRuntime(obstime);
    obstime += hours(1);
  }
}

void MissingRunner::addAllMissingData(
    KvalobsDatabaseAccess & dbAccess,
    const boost::posix_time::ptime & obstime) {
  auto missing = dbAccess.findAllMissing(obstime);

  for (const DataIdentifier & di : missing) {
    auto transaction = dbAccess.transaction(true);
    dbAccess.addMissingData(di);
    transaction->commit();
  }
}

void MissingRunner::stop() {
  stopped_ = true;
  condition_.notify_all();
}
