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

#ifndef SRC_KVMANAGERD_MISSINGRUNNER_H_
#define SRC_KVMANAGERD_MISSINGRUNNER_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <condition_variable>
#include <chrono>
#include <string>

class KvalobsDatabaseAccess;

class MissingRunner {
 public:
  explicit MissingRunner(const std::string & connectString);
  ~MissingRunner();

  void operator()();

  static bool stopped() {
    return stopped_;
  }

  static void stop();

 private:
  typedef std::chrono::system_clock::time_point Time;
  void run();
  void addAllMissingData(KvalobsDatabaseAccess & dbAccess,
                         const boost::posix_time::ptime & obstime);

  std::string connectString_;
  static bool stopped_;
  static std::condition_variable condition_;
};

#endif  // SRC_KVMANAGERD_MISSINGRUNNER_H_
