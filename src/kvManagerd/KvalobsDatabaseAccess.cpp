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

#include "KvalobsDatabaseAccess.h"
#include <boost/date_time.hpp>
#include <milog/milog.h>
#include <sstream>
#include <memory>
#include <string>
#include "kvdb/kvdb.h"
#include "kvdb/dbdrivermgr.h"
#include "kvalobs/kvPath.h"
#include "miutil/timeconvert.h"

using dnmi::db::Connection;
using dnmi::db::Result;
typedef std::unique_ptr<Result> ResultPtr;
using dnmi::db::DRow;
using boost::posix_time::ptime;
using boost::posix_time::hours;
using boost::posix_time::minutes;

namespace {

dnmi::db::Connection * createConnection(const std::string & databaseConnect) {
  static std::string driverId;
  if (driverId.empty()) {
    std::string driver = kvalobs::kvPath(kvalobs::libdir)
        + "/kvalobs/db/pgdriver.so";
    if (!dnmi::db::DriverManager::loadDriver(driver, driverId))
      throw std::runtime_error("Unable to load driver " + driver);
  }

  dnmi::db::Connection * conn = dnmi::db::DriverManager::connect(driverId, databaseConnect);
  if (!conn)
    throw std::runtime_error(dnmi::db::DriverManager::getErr());
  return conn;
}
}  // namespace

KvalobsDatabaseAccess::KvalobsDatabaseAccess(const std::string & connectString)
    : connection_(createConnection(connectString)) {
}

KvalobsDatabaseAccess::~KvalobsDatabaseAccess() {
  dnmi::db::DriverManager::releaseConnection(connection_);
}

KvalobsDatabaseAccess::MissingList KvalobsDatabaseAccess::findAllMissing(
    const Time & obstime) {
  MissingList ret;

  std::ostringstream expiry;
  expiry << "select typeid, ";
  expiry << "date_trunc('hour', '" << to_kvalobs_string(obstime)
         << "'::timestamp - (lateobs * '1 minute'::interval)) ";
  expiry << "from types where lateobs<100000";

  ResultPtr result = exec_(expiry.str());
  while (result->hasNext()) {
    DRow & row = result->next();
    int type = boost::lexical_cast<int>(row[0]);
    auto obstime = boost::posix_time::time_from_string(row[1]);

    ResultPtr missing = exec_(qFindMissingStations_(type, obstime));
    while (missing->hasNext()) {
      DRow & m = missing->next();
      int stationid = boost::lexical_cast<int>(m[0]);
      DataIdentifier di = createObservation_(stationid, type, obstime);
      ret.push_back(di);
    }
  }
  return ret;
}

KvalobsDatabaseAccess::Time KvalobsDatabaseAccess::lastFindAllMissingRuntime() {
  std::string query =
      "SELECT val FROM key_val WHERE package='kvManagerd' AND key='LastMissingRun'";
  ResultPtr r = exec_(query);
  if (r->hasNext()) {
    DRow row = r->next();
    try {
      Time last = boost::posix_time::time_from_string(row[0]);
      return ptime(last.date(), hours(last.time_of_day().hours()) + minutes(30));
    } catch (...) {
      // ignore errors, and continue to fallback method
    }
  }
  // No information available, pretend last run was 7 days ago
  auto t = boost::posix_time::second_clock::universal_time();
  ptime obstime(t.date(), hours(t.time_of_day().hours()) + minutes(30));
  return obstime - (hours(24 * 7));
}

void KvalobsDatabaseAccess::setLastFindAllMissingRuntime(
    const KvalobsDatabaseAccess::Time & obstime) {
  connection_->beginTransaction();
  exec_(
      "DELETE FROM key_val WHERE package='kvManagerd' AND key='LastMissingRun'");
  exec_(
      "INSERT INTO key_val VALUES ('kvManagerd', 'LastMissingRun', '"
          + to_kvalobs_string(obstime) + "')");
  connection_->endTransaction();
}

namespace {
bool isWholeHour(const boost::posix_time::ptime & time) {
  const auto & t = time.time_of_day();
  return t.minutes() == 0 && t.seconds() == 0 && t.fractional_seconds() == 0;
}
}

void KvalobsDatabaseAccess::addMissingData(const DataIdentifier & di) {
  if (isWholeHour(di.obstime())) {
    // Only process missing value for data a whole hours (minutes and seconds is 0)
    exec_(insertMissingDataQuery_(di));
  }

  ResultPtr r = exec_(selectWorkQueueQuery_(di));
  if (!r->hasNext() )
    exec_(insertWorkQueueQuery_(di, 10));
  else
    exec_(updateWorkQueueQuery_(di));
}

DataIdentifier KvalobsDatabaseAccess::nextDataToProcess() {
  std::string selectQuery = "SELECT o.observationid, o.stationid, o.typeid, o.obstime FROM observations o, workque q WHERE o.observationid=q.observationid AND q.process_start is NULL limit 1";

  ResultPtr r = exec_(selectQuery);
  if (r->hasNext()) {
    return DataIdentifier(r->next());
  }
  return DataIdentifier::invalid();
}

void KvalobsDatabaseAccess::cleanWorkQueue() {
  auto t = transaction();
  const std::string criteria = "qa_stop<now()-'15 minutes'::interval";

  exec_("delete from workstatistik s using workque q "
    "where s.observationid=q.observationid "
    "and q." + criteria);
  exec_("INSERT INTO workstatistik (SELECT "
    "o.stationid, o.obstime, o.typeid, o.tbtime, q.priority, q.process_start, q.qa_start, q.qa_stop, q.service_start, q.service_stop, o.observationid"
    " FROM workque q, observations o WHERE"
    " q.observationid=o.observationid "
    " and q." + criteria + ")");
  exec_("DELETE FROM workque WHERE " + criteria);

  t->commit();
}


KvalobsDatabaseAccess::Transaction::Transaction(
    dnmi::db::Connection & connection,
    bool serializable)
    : connection_(connection),
      handled_(false) {
  Connection::IsolationLevel il =
      serializable ? Connection::SERIALIZABLE : Connection::READ_COMMITTED;
  connection_.beginTransaction(il);
}

KvalobsDatabaseAccess::Transaction::~Transaction() {
  if (!handled_)
    connection_.rollBack();
}

void KvalobsDatabaseAccess::Transaction::commit() {
  connection_.endTransaction();
  handled_ = true;
}

void KvalobsDatabaseAccess::Transaction::rollback() {
  connection_.rollBack();
  handled_ = true;
}

KvalobsDatabaseAccess::TransactionPtr KvalobsDatabaseAccess::transaction(bool serializable) {
  return TransactionPtr(new Transaction(*connection_, serializable));
}

KvalobsDatabaseAccess::ResultPtr KvalobsDatabaseAccess::exec_(
    const std::string & query) {
  LOGDEBUG(query);
  return ResultPtr(connection_->execQuery(query + ';'));
}

std::string KvalobsDatabaseAccess::qFindMissingStations_(
    int type, const boost::posix_time::ptime & timeToFind) const {
  std::ostringstream q;
  // Make a selection of all obs_pgm_rows with correct typeid and klXX entry
  q << "SELECT\n";
  q << "  stationid\n";
  q << "FROM\n";
  q << "  obs_pgm\n";
  q << "WHERE\n";
  q << "  fromtime<'" << timeToFind << "' and\n";
  q << "  ( totime is null or totime>'" << timeToFind << "') and\n";
  q << "  typeid=" << type << " and\n";
  q << "  kl" << std::setw(2) << std::setfill('0')
    << timeToFind.time_of_day().hours() << "\n";
  q << "EXCEPT\n";
  // But remove any stations that are already in data table with the correct
  // obstime and typeid
  q << "SELECT\n";
  q << "  stationid\n";
  q << "FROM\n";
  q << "  observations\n";
  q << "WHERE\n";
  q << "  typeid=" << type << " and\n";
  q << "  obstime='" << timeToFind << "'";
  return q.str();
}

std::string KvalobsDatabaseAccess::insertMissingDataQuery_(const DataIdentifier & di) const {
  // Big query ahead!
  // The query is of the form INSERT INTO data (SELECT ....), where the select
  // is essentially all possible values from obs_pgm except those that already
  // exist in the data table.

  std::ostringstream q;
  q << "INSERT INTO obsdata (\n";
  q << "  SELECT\n";
  //        What to insert
  q << "    " << di.obsid() << ", \n";
  q << "    mv.value,\n";
  q << "    q.paramid,\n";
  q << "    q.sensor::char,\n";
  q << "    q.level,\n";
  q << "    mv.value,\n";
  q << "    mv.controlinfo,\n";
  q << "    '0000000000000000',\n";
  q << "    ''\n";
  q << "  FROM \n";
  //        Create an iterator over all possible sensor values (0-9)
  q << "    (WITH RECURSIVE number_series(number) AS (\n";
  q << "      VALUES(0)\n";
  q << "    UNION ALL\n";
  q << "      SELECT number+1 \n";
  q << "      FROM number_series \n";
  q << "      WHERE number<10\n";
  q << "    )\n";
  //        Get all paramid/sensor/level combinations from obs_pgm for the given DataIdentifier
  q << "    SELECT\n";
  q << "      o.paramid, \n";
  q << "      n.number as sensor,\n";
  q << "      o.level\n";
  q << "    FROM \n";
  q << "      obs_pgm o,\n";
  q << "      number_series n,\n";
  q << "      observations obs\n";
  q << "    WHERE \n";
  q << "      obs.observationid=" << di.obsid() << " and\n";
  q << "      o.stationid=obs.stationid and\n";
  q << "      o.typeid=obs.typeid and\n";
  q << "      n.number < o.nr_sensor and\n";
  q << "      fromtime<obs.obstime and \n";
  q << "      ( totime is null or \n";
  q << "        totime>obs.obstime) and \n";
  q << "      kl" << std::setw(2) << std::setfill('0') << di.obstime().time_of_day().hours() << "\n";
  q << "    EXCEPT \n"; 
  //        ...but remove any combinations that already exist in data table
  q << "    SELECT \n";
  q << "      d.paramid, \n";
  q << "      d.sensor::int,\n";
  q << "      d.level\n";
  q << "    FROM \n";
  q << "      obsdata d,\n";
  q << "      observations o\n";
  q << "    WHERE \n";
  q << "     d.observationid=o.observationid and\n";
  q << "     " << di.sqlWhere("o");
  q << "    ) q,\n";
  q << "    default_missing_values mv\n";
  q << "  WHERE\n";
  q << "    q.paramid = mv.paramid\n";
  q << "  )";
  return q.str();
}

std::string KvalobsDatabaseAccess::selectWorkQueueQuery_(const DataIdentifier & di) const {
  std::ostringstream q;
  q << "SELECT priority FROM workque WHERE " << di.sqlWhere();
  return q.str();
}

std::string KvalobsDatabaseAccess::insertWorkQueueQuery_(const DataIdentifier & di, int priority) const {
  std::ostringstream q;
  q << "INSERT INTO workque VALUES (" << priority << ",now(),NULL,NULL,NULL,NULL," << di.obsid() << ")";
  return q.str();
}

std::string KvalobsDatabaseAccess::updateWorkQueueQuery_(const DataIdentifier & di) const {
  std::ostringstream q;
  q << "UPDATE workque SET ";
  q << "process_start=now(), ";
  q << "qa_start=NULL, ";
  q << "qa_stop=NULL, ";
  q << "service_start=NULL, ";
  q << "service_stop=NULL ";
  q << "WHERE ";
  q << di.sqlWhere();
  return q.str();
}

DataIdentifier KvalobsDatabaseAccess::createObservation_(int stationid, int type, const boost::posix_time::ptime & obstime) {
  std::ostringstream query;
  query << "INSERT INTO observations (stationid, typeid, obstime, tbtime) VALUES (" << stationid << ", " << type << ", '" << obstime << "', now()) RETURNING observationid, stationid, typeid, obstime";
  auto result = exec_(query.str());
  if (!result->hasNext())
    throw std::runtime_error("Unable to create observation");
  return DataIdentifier(result->next());
}