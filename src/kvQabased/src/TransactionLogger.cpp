/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvPath.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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

#include "TransactionLogger.h"
#include <kvalobs/kvPath.h>
#include <miutil/timeconvert.h>
#include <milog/milog.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <chrono>
#include <cstdio>
#include <cstring>


namespace qabase {

TransactionLogger::TransactionLogger(const kvalobs::kvStationInfo & si)
    : success_(false),
      si_(si),
      startTime_(std::chrono::system_clock::now()) {
}

TransactionLogger::~TransactionLogger() {
  int fd = open(logFile().c_str(), O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (fd < 0) {
    const int BUF_SIZE = 512;
    char msg[BUF_SIZE];
    strerror_r(errno, msg, BUF_SIZE);
    LOGWARN("Error when creating log file: " << msg);
  } else {
    std::string msg = message();
    write(fd, msg.c_str(), msg.size());
    close(fd);
  }
}

namespace {
std::string format(const std::chrono::system_clock::time_point & time) {
  struct tm stm;
  std::time_t t = std::chrono::system_clock::to_time_t(time);
  gmtime_r(&t, &stm);
  char timeString[256];
  std::strftime(timeString, 256, "%FT%T", &stm);
  return timeString;
}
}

std::string TransactionLogger::message() const {
  auto endTime = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime_);

  std::ostringstream summaryLog;
  summaryLog << format(endTime);
  if (success_)
    summaryLog << "\tSUCCESS\t";
  else
    summaryLog << "\tFAILURE\t";
  summaryLog << '(' << si_.stationID() << '/' << si_.typeID() << '/'
      << to_kvalobs_string(si_.obstime()) << ')';
  summaryLog << "\tduration=" << duration.count() << "ms\n";

  return summaryLog.str();
}

std::string TransactionLogger::logFile() {
  return kvalobs::kvPath(kvalobs::logdir) + "/kvQabased_transaction.log";
}

}  // namespace qabase
