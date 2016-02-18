/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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

#include <milog/milog.h>
#include "lib/kvdb/ConnectionPool.h"


using std::chrono::steady_clock;
using std::chrono::seconds;
using std::make_tuple;

namespace dnmi {
namespace db {

ConnectionPool::ConnectionPool(std::function<Connection*()> createConnection, std::function<void(Connection*)> releaseConnection,
                               unsigned int poolsize, unsigned int idleTimeBeforeReleaseInSeconds, UseCount useCount)
    : createConnection_(createConnection),
      releaseConnection_(releaseConnection),
      poolsize_(poolsize),
      useCount_(useCount),
      idleTimeout_(seconds(idleTimeBeforeReleaseInSeconds)) {
}

ConnectionPool::~ConnectionPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto c : freeConnections_)
    releaseConnection_(std::get<2>(c));
}

ConnectionPool::ConnectionPtr ConnectionPool::get() {
  std::lock_guard<std::mutex> lock(mutex_);
  Connection * ret = nullptr;
  UseCount conUseCount = 0;
  if (!freeConnections_.empty()) {
    ret = std::get<2>(freeConnections_.front());
    conUseCount = std::get<1>(freeConnections_.front());
    freeConnections_.pop_front();
  } else {
    ret = createConnection_();
  }
  return ConnectionPtr(ret, [&, conUseCount](Connection * connection) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (freeConnections_.size() < poolsize_ && conUseCount < useCount_) {
      freeConnections_.push_back(make_tuple(steady_clock::now()+idleTimeout_, conUseCount+1, connection));
    } else {
      if (freeConnections_.size() >= poolsize_) {
        LOGWARN("ConnectionPool: Thrashing: Creating more connections to the database than we have available slots (" << poolsize_ << ") in the pool.");
      }
      releaseConnection_(connection);
    }
  });
}

void ConnectionPool::releaseIdleConnections() {
  std::lock_guard<std::mutex> lock(mutex_);
  auto now = steady_clock::now();
  for (auto it = freeConnections_.begin(); it != freeConnections_.end(); /*nothing*/) {
    if (now < std::get<0>(*it)) {
      releaseConnection_(std::get<2>(*it));
      it = freeConnections_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace db
}  // namespace dnmi
