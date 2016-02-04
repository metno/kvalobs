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

#ifndef SRC_SERVICE_LIBS_KVCPP_SQL_CONNECTIONPOOL_H_
#define SRC_SERVICE_LIBS_KVCPP_SQL_CONNECTIONPOOL_H_

#include <functional>
#include <memory>
#include <list>
#include <mutex>

namespace dnmi {
namespace db {
class Connection;
}
}

namespace kvservice {
namespace sql {

/**
 * Thread-safe dispenser for database connections
 */
class ConnectionPool {
 public:
  ConnectionPool(std::function<dnmi::db::Connection*()> createConnection,
                 std::function<void(dnmi::db::Connection*)> releaseConnection);
  ~ConnectionPool();

  typedef std::shared_ptr<dnmi::db::Connection> ConnectionPtr;

  ConnectionPtr get();

 private:
  std::function<dnmi::db::Connection*()> createConnection_;
  std::function<void(dnmi::db::Connection*)> releaseConnection_;

  std::list<dnmi::db::Connection *> freeConnections_;
  std::mutex mutex_;
};

} /* namespace sql */
} /* namespace kvservice */

#endif /* SRC_SERVICE_LIBS_KVCPP_SQL_CONNECTIONPOOL_H_ */
