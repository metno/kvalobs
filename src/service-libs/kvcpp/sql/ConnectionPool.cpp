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

#include "ConnectionPool.h"
#include <milog/milog.h>

namespace kvservice {
namespace sql {

ConnectionPool::ConnectionPool(
    std::function<dnmi::db::Connection*()> createConnection,
    std::function<void(dnmi::db::Connection*)> releaseConnection)
    : createConnection_(createConnection),
      releaseConnection_(releaseConnection) {
}

ConnectionPool::~ConnectionPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (dnmi::db::Connection * c : freeConnections_)
    releaseConnection_(c);
}

ConnectionPool::ConnectionPtr ConnectionPool::get() {
  std::lock_guard<std::mutex> lock(mutex_);
  dnmi::db::Connection * ret = nullptr;
  if (!freeConnections_.empty()) {
    ret = freeConnections_.front();
    freeConnections_.pop_front();
  } else {
    ret = createConnection_();
  }
  return std::shared_ptr<dnmi::db::Connection> (ret, [&](dnmi::db::Connection * connection) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (freeConnections_.size() < 4) {
          LOGDEBUG("Putting connection back in pool. New pool size: " << freeConnections_.size() +1);
          freeConnections_.push_back(connection);
        } else {
          LOGDEBUG("Releasing connection. Pool size: " << freeConnections_.size());
          releaseConnection_(connection);
        }
      });
}

} /* namespace sql */
} /* namespace kvservice */
