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

#ifndef SRC_LIB_KVDB_CONNECTIONPOOL_H_
#define SRC_LIB_KVDB_CONNECTIONPOOL_H_

#include <functional>
#include <memory>
#include <list>
#include <tuple>
#include <mutex>
#include <chrono>

namespace dnmi {
namespace db {
class Connection;

/**
 * Thread-safe connection pool for database connections
 */
class ConnectionPool {
 public:
  typedef unsigned int UseCount;
  typedef dnmi::db::Connection Connection;
  typedef std::shared_ptr<Connection> ConnectionPtr;
  /**
   * Constructor to create a connection pool.
   *
   * @param createConnection a function to be called when a new connection is to be created. The
   *        function must return a new connection.
   * @param releaseConnection a function to be called when a connection is to be released.
   * @param poolsize, the size of the connection pool. Default value is 4.
   * @param idleTimeBeforeReleasedInSeconds How long should a connection be in the pool unused
   *        before it is released. Call releaseIdleConnections() to actually release the idle connections.
   *        Default value is 60 seconds.
   * @param useCount, force release of a connection after it is used \e useCount times.
   *        Default value 1000.
   */
  ConnectionPool(std::function<Connection *()> createConnection,
                 std::function<void(Connection *)> releaseConnection, unsigned int poolsize = 4,
                 unsigned int idleTimeBeforeReleaseInSeconds = 60, UseCount useCount = 1000);
  ~ConnectionPool();



  ConnectionPtr get();

  /**
   * Release connection that has not be used in the last \e idleTimeBeforeReleaseInSeconds.
   * \see ConnectionPool constructor.
   */
  void releaseIdleConnections();

 private:
  typedef std::chrono::time_point<std::chrono::steady_clock> IdleTo;
  typedef std::tuple<IdleTo, UseCount, Connection*> CacheElement;
  std::function<Connection* ()> createConnection_;
  std::function<void(Connection*)> releaseConnection_;
  std::list<CacheElement> freeConnections_;
  unsigned int poolsize_;
  UseCount useCount_;
  std::chrono::seconds idleTimeout_;
  std::mutex mutex_;
};

}  // namespace db
}  // namespace dnmi

#endif  // SRC_LIB_KVDB_CONNECTIONPOOL_H_
