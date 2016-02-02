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

#include "TimedDatabaseTask.h"
#include <map>
#include <mutex>
#include <string>
#include <condition_variable>
#include "milog/milog.h"

bool TimedDatabaseTask::stopped_ = false;


namespace {
  std::map<TimedDatabaseTask*, std::condition_variable> conditions;
}  // namespace

TimedDatabaseTask::TimedDatabaseTask(const std::string & connectString, const std::string & logContext)
    : connectString_(connectString),
      logContext_(logContext) {
  conditions[this];
}

TimedDatabaseTask::~TimedDatabaseTask() {
  conditions.erase(this);
}

void TimedDatabaseTask::operator()() {
  milog::LogContext context(logContext_);

  // We use mutex and condition variables to enable interruptible sleep_until
  std::condition_variable & condition = conditions[this];
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  while (!stopped()) {
    try {
      run();
    } catch (std::exception & e) {
      LOGERROR(e.what());
    }
    if (!stopped())
      condition.wait_until(lock, nextRunTime());
  }
  LOGDEBUG("Stopped timed task");
}

void TimedDatabaseTask::stop() {
  stopped_ = true;
  for (auto it = conditions.begin(); it != conditions.end(); ++it)
    it->second.notify_all();
}
