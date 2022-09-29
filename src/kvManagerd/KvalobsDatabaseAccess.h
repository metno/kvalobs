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

#ifndef SRC_KVMANAGERD_KVALOBSDATABASEACCESS_H_
#define SRC_KVMANAGERD_KVALOBSDATABASEACCESS_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/noncopyable.hpp>
#include <string>
#include <vector>
#include <memory>
#include "DataIdentifier.h"

namespace dnmi {
namespace db {
class Connection;
class Result;
}  // namespace db
}  // namespace dnmi

/**
 * Methods to access kvalobs database
 */
class KvalobsDatabaseAccess : boost::noncopyable {
 public:
  typedef boost::posix_time::ptime Time;
  typedef std::vector<DataIdentifier> MissingList;

  /**
   * @throws std::runtime_error if unable to connect to database for any
   *                            reason.
   */
  explicit KvalobsDatabaseAccess(const std::string & connectString);
  ~KvalobsDatabaseAccess();

  /**
   * Get a list of all stations that have failed to deliver observations for
   * the given obstime.
   *
   * Only stations that are supposed to report at the given time are listed.
   */
  MissingList findAllMissing(const Time & obstime);

  /**
   * When was the last time findAllMissing() was run with success?
   */
  Time lastFindAllMissingRuntime();

  /**
   * Set time for last findAllMissing() call
   */
  void setLastFindAllMissingRuntime(const Time & obstime);

  /**
   * Add all missing data elements for the given location
   */
  void addMissingData(const DataIdentifier & di);

  /**
   * Get next data element to process. Will return empty ptr if no data
   * available.
   */
  DataIdentifier nextDataToProcess();

  void cleanWorkQueue();


  /**
   * A transaction handler. Starts transaction upon construction. Unless
   * commit() is called on it, the transaction will be rolled back upon
   * destruction.
   */
  class Transaction {
   public:
    ~Transaction();
    void commit();
    void rollback();

   private:
    explicit Transaction(dnmi::db::Connection & connection, bool serializable);
    dnmi::db::Connection & connection_;
    bool handled_;
    friend class KvalobsDatabaseAccess;
  };
  typedef std::shared_ptr<Transaction> TransactionPtr;

  /**
   * Start a transaction, possibly with serializable isolation level.
   */
  TransactionPtr transaction(bool serializable = false);

 private:
  typedef std::shared_ptr<dnmi::db::Result> ResultPtr;
  ResultPtr exec_(const std::string & query);

  /**
   * Get an sql query to find all stations that have not reported at all for
   * the given obstime.
   *
   * @param type only the given typeid will be checked
   * @param timeToFind obstime we want missing stations for
   */
  std::string qFindMissingStations_(int type, const boost::posix_time::ptime & timeToFind) const;

  /**
   * Get an sql query to insert "missing rows" for the given station
   *
   * @param di dataset to complete
   */
  std::string insertMissingDataQuery_(const DataIdentifier & di) const;
  std::string selectWorkQueueQuery_(const DataIdentifier & di) const;
  std::string insertWorkQueueQuery_(const DataIdentifier & di, int priority) const;
  std::string updateWorkQueueQuery_(const DataIdentifier & di) const;

  DataIdentifier createObservation_(int stationid, int type, const boost::posix_time::ptime & obstime);

  dnmi::db::Connection * connection_;
};

#endif  // SRC_KVMANAGERD_KVALOBSDATABASEACCESS_H_
