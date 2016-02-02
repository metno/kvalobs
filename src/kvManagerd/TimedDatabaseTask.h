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

#ifndef SRC_KVMANAGERD_TIMEDDATABASETASK_H_
#define SRC_KVMANAGERD_TIMEDDATABASETASK_H_

#include <chrono>
#include <string>

class TimedDatabaseTask {
 public:
  TimedDatabaseTask(const std::string & connectString, const std::string & logContext);
  virtual ~TimedDatabaseTask();

  void operator()();

  static bool stopped() {
    return stopped_;
  }

  static void stop();

 protected:
  typedef std::chrono::system_clock::time_point Time;
  virtual Time nextRunTime() const = 0;
  virtual void run() = 0;

  const std::string connectString() const {
    return connectString_;
  }

 private:
  std::string connectString_;
  std::string logContext_;
  static bool stopped_;
};

#endif /* SRC_KVMANAGERD_TIMEDDATABASETASK_H_ */
