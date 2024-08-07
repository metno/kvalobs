/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2010 met.no

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
#include "returntypes/Observation.h"
#include "databaseResultFilter.h"
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvPath.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvTextDataOperations.h>
#include <milog/milog.h>
#include <miutil/timeconvert.h>
#include <decodeutility/kvalobsdata.h>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <iomanip>

namespace pt = boost::posix_time;

namespace db {
namespace {
typedef boost::scoped_ptr<dnmi::db::Result> ResultPtr;
}  // namespace

class KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection {
 public:
  TransactionEnforcingDatabaseConnection(dnmi::db::Connection * connection,
                                         bool takeOwnershipOfConnection);
  ~TransactionEnforcingDatabaseConnection();
  dnmi::db::Result *execQuery(const std::string & SQLstmt);

  std::string esc(const std::string & stringToEscape) const;

  void exec(const std::string & SQLstmt);
  void beginTransaction(dnmi::db::Connection::IsolationLevel isolation = dnmi::db::Connection::READ_COMMITTED);
  void commit();
  void rollback();
  bool transactionInProgress() const;
 private:
  dnmi::db::Connection * connection_;
  bool transactionInProgress_;
  bool ownsConnection_;
};

KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::TransactionEnforcingDatabaseConnection(
    dnmi::db::Connection * connection, bool takeOwnershipOfConnection)
    : connection_(connection),
      transactionInProgress_(false),
      ownsConnection_(takeOwnershipOfConnection) {
  if (!connection)
    throw std::runtime_error("No database connection");
}

KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::~TransactionEnforcingDatabaseConnection() {
  if (transactionInProgress_) {
    try {
      rollback();
    } catch (std::exception &) {
    }
  }
  if (ownsConnection_)
    delete connection_;
}

dnmi::db::Result * KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::execQuery(
    const std::string & SQLstmt) {
  if (transactionInProgress_)
    return connection_->execQuery(SQLstmt);
  throw std::runtime_error("No transaction in progress");
}

std::string KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::esc(
    const std::string & stringToEscape) const {
  return connection_->esc(stringToEscape);
}

void KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::exec(
    const std::string & SQLstmt) {
  if (transactionInProgress_)
    connection_->exec(SQLstmt);
  else
    throw std::runtime_error("No transaction in progress");
}
void KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::beginTransaction(dnmi::db::Connection::IsolationLevel isolation) {
  connection_->beginTransaction(isolation);
  transactionInProgress_ = true;
}
void KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::commit() {
  connection_->endTransaction();
  transactionInProgress_ = false;
}
void KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::rollback() {
  if (transactionInProgress_)
    connection_->rollBack();
  transactionInProgress_ = false;
}
bool KvalobsDatabaseAccess::TransactionEnforcingDatabaseConnection::transactionInProgress() const {
  return transactionInProgress_;
}


KvalobsDatabaseAccess::KvalobsDatabaseAccess(
    const std::string & databaseConnect) {
  dnmi::db::Connection * conn = createConnection(databaseConnect);
  if (!conn)
    throw std::runtime_error("Unable to connect to database");
  connection_ = new TransactionEnforcingDatabaseConnection(conn, true);
}

KvalobsDatabaseAccess::KvalobsDatabaseAccess(dnmi::db::Connection * connection,
                                             bool takeOwnershipOfConnection)
    : connection_(
        new TransactionEnforcingDatabaseConnection(connection,
                                                   takeOwnershipOfConnection)) {
}

std::string KvalobsDatabaseAccess::modelDataName_;

void KvalobsDatabaseAccess::setModelDataName(
    const std::string & modelDataName) {
  modelDataName_ = modelDataName;
}

KvalobsDatabaseAccess::~KvalobsDatabaseAccess() {
  delete connection_;
}

void KvalobsDatabaseAccess::beginTransaction() {
  connection_->beginTransaction();
  fetchedData_.clear();
}

void KvalobsDatabaseAccess::commit() {
  connection_->commit();
}

void KvalobsDatabaseAccess::rollback() {
  connection_->rollback();
}

void KvalobsDatabaseAccess::getChecks(CheckList * out,
                                      const qabase::Observation & obs) const {
  std::ostringstream query;
  query << "SELECT * FROM checks WHERE ";
  query << "(stationid=0 OR stationid=" << obs.stationID() << ")";
  query << " AND fromtime<='" << to_kvalobs_string(obs.obstime()) << "'";
  query << " ORDER BY stationid, fromtime;";

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  std::map<std::string, kvalobs::kvChecks> checks;

  ResultPtr result(connection_->execQuery(query.str()));
  while (result->hasNext()) {
    // "Order by" in query will first insert 0-station values, and then
    // overwrite with specific-value stations likewise for varying
    // fromtime - latest fromtime for station is preferred
    kvalobs::kvChecks check(result->next());
    checks[check.qcx()] = check;
  }
  for (std::map<std::string, kvalobs::kvChecks>::const_iterator it = checks
      .begin(); it != checks.end(); ++it)
    out->push_back(it->second);
}

int KvalobsDatabaseAccess::getQcxFlagPosition(const std::string & qcx) const {
  std::string mediumQcx(qcx, 0, qcx.find_last_of('-'));

  std::ostringstream query;
  query << "SELECT controlpart FROM qcx_info WHERE medium_qcx='" << mediumQcx
        << "';";

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  ResultPtr result(connection_->execQuery(query.str()));

  if (result->hasNext())
    return boost::lexical_cast<int>(result->next()[0]);
  else
    throw std::runtime_error(mediumQcx + ": No such check type");
}

void KvalobsDatabaseAccess::getParametersToCheck(
    ParameterList * out, const qabase::Observation & obs) const {
  milog::LogContext context("query");
  {
    std::ostringstream query;
    query << "SELECT * FROM obs_pgm WHERE stationid=" << obs.stationID()
        << " LIMIT 1;";
    LOGDEBUG1(query.str());
    ResultPtr result(connection_->execQuery(query.str()));
    if (not result->hasNext())
      return;  // no expected parameters
  }

  std::ostringstream query;
  query << "SELECT distinct name FROM param WHERE "
      "paramid IN (SELECT paramid FROM obsdata WHERE "
      "observationid=" << obs.id() << ')';
  LOGDEBUG1(query.str());
  ResultPtr result(connection_->execQuery(query.str()));
  while (result->hasNext()) {
    dnmi::db::DRow & row = result->next();
    out->insert(row[0]);
  }
}

kvalobs::kvAlgorithms KvalobsDatabaseAccess::getAlgorithm(
    const std::string & algorithmName) const {
  std::ostringstream query;
  query << "SELECT * FROM algorithms WHERE ";
  query << "checkname='" << algorithmName << "';";

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  ResultPtr result(connection_->execQuery(query.str()));

  if (result->hasNext())
    return kvalobs::kvAlgorithms(result->next());
  else
    throw std::runtime_error(algorithmName + ": No such algorithm");
}

std::string KvalobsDatabaseAccess::getStationParam(
    const kvalobs::kvStationInfo & si, 
    const std::string & parameter, int sensor, int level, 
    const std::string & qcx) const {
  const boost::gregorian::date & date = si.obstime().date();
  const boost::gregorian::date firstDayOfYear(date.year(), 1, 1);
  int dayNumber = (date - firstDayOfYear).days() + 1;
  //int dayNumber = si.obstime().dayOfYear();

  std::ostringstream query;
  query << "SELECT metadata FROM station_param WHERE "
        "stationid in (0, "
        << si.stationID() << ") AND "
        "paramid=(SELECT paramid FROM param WHERE name='"
        << parameter << "') AND "
        "fromday<="
        << dayNumber << " AND " << dayNumber << "<=today AND "
        "qcx='"
        << qcx << "' AND "
        "level=" << level << " AND sensor='" << sensor <<"' AND "
        "fromtime<='"
        << to_kvalobs_string(si.obstime()) << "' "
        "ORDER BY stationid DESC, fromtime DESC "
        "LIMIT 1;";

  ResultPtr result(connection_->execQuery(query.str()));

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  if (result->hasNext()) {
    std::string metadata = result->next()[0];
    return metadata;
  } else {
    throw std::runtime_error(
        "Unable to find station_param for " + parameter + " for qcx=" + qcx);
  }
}


void KvalobsDatabaseAccess::getStationParamAll( qabase::StationParamList &outResult,
                                   const kvalobs::kvStationInfo & si,
                                   const std::string & parameter, 
                                   const std::string & qcx) const{
  const boost::gregorian::date & date = si.obstime().date();
  const boost::gregorian::date firstDayOfYear(date.year(), 1, 1);
  int dayNumber = (date - firstDayOfYear).days() + 1;
  outResult.clear();
  qabase::StationParamSet myRes;

  std::ostringstream query;
  query << "SELECT sensor, level, metadata FROM station_param WHERE "
        "stationid in (0, "
        << si.stationID() << ") AND "
        "paramid=(SELECT paramid FROM param WHERE name='"
        << parameter << "') AND "
        "fromday<="
        << dayNumber << " AND " << dayNumber << "<=today AND "
        "qcx='"
        << qcx << "' AND fromtime <= '" << to_kvalobs_string(si.obstime()) << "' "
        "ORDER BY stationid ASC, fromtime ASC;";

  ResultPtr result(connection_->execQuery(query.str()));

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  qabase::StationParam stp;
  stp.stationid=si.stationID();
  stp.qcx = qcx;
  stp.paramid = parameter;
  while (result->hasNext()) {
    auto &row = result->next();
    try {
      stp.sensor=std::stoi(row[0]);
      stp.level=std::stoi(row[1]);
    }
    catch( const std::out_of_range &ex) {
      std::ostringstream ost;
      ost << "Failed to convert 'sensor=" << row[0] << "' or 'level="<<row[1]<<"' to integer (out_of_range)";
      throw std::runtime_error(ost.str());
    }
    catch( const std::invalid_argument &ex){
      std::ostringstream ost;
      ost << "Failed to convert 'sensor=" << row[0] << "' or 'level="<<row[1]<<"' to integer (invalid_argument)";
      throw std::runtime_error(ost.str());
    } 
    stp.metadata=row[2];
    myRes.insertOrReplace(stp);
  }

  if( myRes.empty() ) {
    throw std::runtime_error(
        "Unable to find station_param for " + parameter + " for qcx=" + qcx);                                  
  }
  
  myRes.toVector(outResult);
}

qabase::Observation KvalobsDatabaseAccess::getObservation(const kvalobs::kvStationInfo & si) const {
  std::ostringstream query;
  query << "SELECT * FROM observations WHERE ";
  query << "stationid=" << si.stationID();
  query << " and typeid=" << si.typeID();
  query << " and obstime='" << to_kvalobs_string(si.obstime()) << "'";
  ResultPtr result(connection_->execQuery(query.str()));

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  if (!result->hasNext()) {
    std::ostringstream s;
    s << "unable to find observation for " << si;
    throw std::runtime_error(s.str());
  }

  //Observation(long long id, int stationid, int type, const boost::posix_time::ptime & obstime, const boost::posix_time::ptime & tbtime)
  auto row = result->next();
  auto id = boost::lexical_cast<long long>(row[0]);
  auto stationid = boost::lexical_cast<int>(row[1]);
  auto type = boost::lexical_cast<int>(row[2]);
  auto obstime = boost::posix_time::time_from_string(row[3]);
  auto tbtime = boost::posix_time::time_from_string(row[4]);
  return qabase::Observation(id, stationid, type, obstime, tbtime);
}


kvalobs::kvStation KvalobsDatabaseAccess::getStation(int stationid) const {
  std::ostringstream query;
  query << "SELECT * FROM station WHERE stationid=" << stationid
        << " ORDER BY fromtime DESC LIMIT 1;";

  ResultPtr result(connection_->execQuery(query.str()));

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  if (not result->hasNext()) {
    std::ostringstream s;
    s << "Unable to find station information for station id " << stationid;
    throw std::runtime_error(s.str());
  }

  kvalobs::kvStation ret(result->next());
  return ret;
}

void KvalobsDatabaseAccess::getModelData(
    ModelDataList * out, const kvalobs::kvStationInfo & si,
    const qabase::DataRequirement::Parameter & parameter,
    int minutesBackInTime) const {
  std::ostringstream query;
  query << "SELECT * FROM model_data WHERE "
        "stationid="
        << si.stationID() << " AND ";

  if (not modelDataName_.empty())
    query << "modelid IN (SELECT modelid FROM model WHERE name='"
          << connection_->esc(modelDataName_) << "') AND ";

  if (minutesBackInTime != 0) {
    boost::posix_time::ptime first = si.obstime()
        + boost::posix_time::minutes(minutesBackInTime);
    if (first == si.obstime())
      query << "obstime='" << to_kvalobs_string(first) << "' AND ";
    else
      query << "obstime BETWEEN '" << to_kvalobs_string(first) << "' AND '"
            << to_kvalobs_string(si.obstime()) << "' AND ";
  } else
    query << "obstime = '" << to_kvalobs_string(si.obstime()) << "' AND ";
  query << "paramid = (SELECT paramid FROM param WHERE name='"
        << parameter.baseName() << "')";
  if (parameter.haveLevel())
    query << " AND level=" << parameter.level();
  query << " ORDER BY obstime DESC;";

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  ResultPtr result(connection_->execQuery(query.str()));

  while (result->hasNext())
    out->push_back(kvalobs::kvModelData(result->next()));
}

namespace {
  /**
   * Read database rows with any data entries after the kvdata-specific ones
   */
  kvalobs::kvData kvDataFromRow(dnmi::db::DRow & row) {
    return kvalobs::kvData(
      boost::lexical_cast<int>(row[0]),
      boost::posix_time::time_from_string(row[1]),
      boost::lexical_cast<float>(row[2]),
      boost::lexical_cast<int>(row[3]),
      boost::posix_time::time_from_string(row[4]),
      boost::lexical_cast<int>(row[5]),
      boost::lexical_cast<int>(row[6]),
      boost::lexical_cast<int>(row[7]),
      boost::lexical_cast<float>(row[8]),
      kvalobs::kvControlInfo(row[9]),
      kvalobs::kvUseInfo(row[10]),
      row[11]
    );
  }

  kvalobs::kvTextData kvTextDataFromRow(dnmi::db::DRow & row) {
    return kvalobs::kvTextData(
      boost::lexical_cast<int>(row[0]),
      boost::posix_time::time_from_string(row[1]),
      row[2],
      boost::lexical_cast<int>(row[3]),
      boost::posix_time::time_from_string(row[4]),
      boost::lexical_cast<int>(row[5])
    );
  }
}

void KvalobsDatabaseAccess::getData(
    DataList * out, const qabase::Observation & obs,
    const qabase::DataRequirement::Parameter & parameter,
    int minuteOffset) const {
  std::ostringstream query;
  query << "SELECT "
    "o.stationid, o.obstime, d.original, d.paramid, o.tbtime, o.typeid, d.sensor, d.level, d.corrected, d.controlinfo, d.useinfo, d.cfailed, o.observationid "
    "FROM "
    "observations o, obsdata d "
    "WHERE "
    "o.observationid = d.observationid AND ";
    query << "o.stationid=" << obs.stationID() << " AND ";
    query << "d.paramid IN (SELECT paramid FROM param WHERE name='"
          << parameter.baseName() << "') AND ";
    if (parameter.haveLevel())
      query << "d.level=" << parameter.level() << " AND ";
    if (parameter.haveSensor())
      query << "d.sensor='" << parameter.sensor() << "' AND ";
    if (parameter.haveType())
      query << "o.typeid=" << parameter.type() << " AND ";
    boost::posix_time::ptime t = obs.obstime()
        + boost::posix_time::minutes(minuteOffset);
    if (t == obs.obstime())
      query << "obstime='" << to_kvalobs_string(t) << "'";
    else
      query << "obstime BETWEEN '" << to_kvalobs_string(t) << "' AND '"
            << to_kvalobs_string(obs.obstime()) << "'";
    query << " ORDER BY obstime DESC";
    query << " FOR UPDATE;";

    milog::LogContext context("query");
    LOGDEBUG1(query.str());

    ResultPtr result(connection_->execQuery(query.str()));

    db::DatabaseAccess::DataList data;
    while (result->hasNext()) {
      auto r = result->next();
      kvalobs::kvData d = kvDataFromRow(r);
      long long obsid = boost::lexical_cast<long long>(r[12]);
      storeFetched(obsid, d);
      data.push_back(d);
    }

    db::resultfilter::filter(data, obs.typeID());

    out->swap(data);
}

bool KvalobsDatabaseAccess::pin(const qabase::Observation & obs) const {
  std::ostringstream query;
  query << "SELECT "
    "o.stationid, o.obstime, d.original, d.paramid, o.tbtime, o.typeid, d.sensor, d.level, d.corrected, d.controlinfo, d.useinfo, d.cfailed, o.observationid "
    "FROM "
    "observations o, obsdata d "
    "WHERE "
    "o.observationid = d.observationid AND "
    "o.observationid = " << obs.id();
  query << " ORDER BY obstime DESC;";

  milog::LogContext context("query");
  LOGDEBUG1(query.str());

  ResultPtr result(connection_->execQuery(query.str()));

  if (result->size() == 0)
    return false;

  while (result->hasNext()) {
    auto r = result->next();
    kvalobs::kvData d = kvDataFromRow(r);
    long long obsid = boost::lexical_cast<long long>(r[12]);
    storeFetched(obsid, d);
  }
  return true;
}


void KvalobsDatabaseAccess::getTextData(
    TextDataList * out, const qabase::Observation & obs,
    const qabase::DataRequirement::Parameter & parameter,
    int minuteOffset) const {
  std::ostringstream query;
  query << "SELECT "
    "o.stationid, o.obstime, d.original, d.paramid, o.tbtime, o.typeid, o.observationid "
    "FROM "
    "observations o, obstextdata d "
    "WHERE "
    "o.observationid = d.observationid AND ";
    query << "o.stationid=" << obs.stationID() << " AND ";
    query << "d.paramid IN (SELECT paramid FROM param WHERE name='"
          << parameter.baseName() << "') AND ";
    if (parameter.haveType())
      query << "o.typeid=" << parameter.type() << " AND ";
    boost::posix_time::ptime t = obs.obstime()
        + boost::posix_time::minutes(minuteOffset);
    if (t == obs.obstime())
      query << "obstime='" << to_kvalobs_string(t) << "'";
    else
      query << "obstime BETWEEN '" << to_kvalobs_string(t) << "' AND '"
            << to_kvalobs_string(obs.obstime()) << "'";
    query << " ORDER BY obstime DESC";

    milog::LogContext context("query");
    LOGDEBUG1(query.str());

  ResultPtr result(connection_->execQuery(query.str()));

  db::DatabaseAccess::TextDataList data;
  while (result->hasNext()) {
    auto r = result->next();
    kvalobs::kvTextData d = kvTextDataFromRow(r);
    long long obsid = boost::lexical_cast<long long>(r[6]);
    storeFetched(obsid, d);
    data.push_back(d);
  }

  db::resultfilter::filter(data, obs.typeID());

  out->swap(data);
}

namespace {
std::string obsdataQuery(const qabase::Observation & obs) {
  std::ostringstream query;
  query << "SELECT "
  "o.stationid, o.obstime, d.original, d.paramid, o.tbtime, o.typeid, d.sensor, d.level, d.corrected, d.controlinfo, d.useinfo, d.cfailed "
  "FROM "
  "observations o, obsdata d "
  "WHERE "
  "o.observationid = d.observationid AND "
  "o.observationid=" << obs.id();
  milog::LogContext context("query");
  LOGDEBUG1(query.str());
  return query.str();
}

std::string obstextdataQuery(const qabase::Observation & obs) {
  std::ostringstream query;
  query << "SELECT "
  "o.stationid, o.obstime, d.original, d.paramid, o.tbtime, o.typeid "
  "FROM "
  "observations o, obstextdata d "
  "WHERE "
  "o.observationid = d.observationid AND "
  "o.observationid=" << obs.id();
  milog::LogContext context("query");
  LOGDEBUG1(query.str());
  return query.str();
}

}  // namespace

void KvalobsDatabaseAccess::complete_(const qabase::Observation & obs, DataList * out) const {
  ResultPtr result(connection_->execQuery(obsdataQuery(obs)));
  while (result->hasNext()) {
    kvalobs::kvData d = kvDataFromRow(result->next());
    auto func = std::bind1st(kvalobs::compare::same_obs_and_parameter(), d);
    auto previous = std::find_if(out->begin(), out->end(), func);
    if (previous == out->end())
      out->push_back(d);
  }
}

void KvalobsDatabaseAccess::complete_(const qabase::Observation & obs, TextDataList * out) const {
  ResultPtr result(connection_->execQuery(obstextdataQuery(obs)));
  while (result->hasNext()) {
    kvalobs::kvTextData d = kvTextDataFromRow(result->next());
    auto func = std::bind1st(kvalobs::compare::kvTextData_same_obs_and_parameter(), d);
    auto previous = std::find_if(out->begin(), out->end(), func);
    if (previous == out->end())
      out->push_back(d);
  }
}

KvalobsDatabaseAccess::KvalobsDataPtr KvalobsDatabaseAccess::complete(const qabase::Observation & obs, const DataList & d, const TextDataList & td) const {
  DataList dl = d;
  complete_(obs, & dl);
  TextDataList tdl = td;
  complete_(obs, & tdl);
  return std::make_shared<kvalobs::serialize::KvalobsData>(dl, tdl);
}

void KvalobsDatabaseAccess::write(const DataList & data) {
#ifdef QABASE_NO_SAVE
  LOGINFO("Pretending to save " << data.size() << " elements to database");
#else
  milog::LogContext context("query");
  LOGINFO("Saving " << data.size() << " elements to database");
  for (DataList::const_iterator it = data.begin(); it != data.end(); ++it) {
    auto fetched = fetchedData_.find(*it);
    if (fetched == fetchedData_.end()) {
      LOGWARN("Attempt to update non-existing row: " << *it);
      continue;
    }
    long long obsid = fetched->second;

    std::ostringstream query;
    query << "UPDATE obsdata SET ";
    query << "controlinfo='" << it->controlinfo().flagstring() << "', ";
    query << "useinfo='" << it->useinfo().flagstring() << "', ";
    query << "corrected=" << it->corrected() << ", ";
    query << "cfailed='" << it->cfailed() << "' ";
    query << " WHERE ";
    query << "observationid=" << obsid << " AND ";
    query << "paramid=" << it->paramID() << " AND ";
    query << "sensor='" << it->sensor() << "' AND ";
    query << "level=" << it->level();
    query << " RETURNING observationid";
    
    LOGDEBUG1(query.str());
    ResultPtr result(connection_->execQuery(query.str()));
    if (result->size() != 1)
      throw std::runtime_error("Update statement did not affect exactly one row");
  }
#endif
}




std::list<qabase::Observation *> KvalobsDatabaseAccess::selectFailedDataForControl(int limit ) {
    //Check for any data that has not completed the check in the last 10 minutes. Possibly crached or stopped 
    //qaBase process. We run the checks again.
   
    //SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o WHERE q.observationid=o.observationid AND qa_start<now()-'10 minutes'::interval AND qa_stop is null ORDER BY priority, tbtime LIMIT 1;";

    std::ostringstream query; 
 
    if( qaId>-1) {
       query << "SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o "
        "WHERE q.observationid=o.observationid AND qa_start<now()-'10 minutes'::interval AND qa_stop is null"
        " AND (qa_id=" << qaId << " OR qa_id is null) "
        "ORDER BY qa_id, priority, tbtime LIMIT " << limit << ";";
    } else
      query << "SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o "
        "WHERE q.observationid=o.observationid AND qa_start<now()-'10 minutes'::interval AND qa_stop is null "
        "ORDER BY priority, tbtime LIMIT " << limit << ";";

    std::unique_ptr<dnmi::db::Result> result(connection_->execQuery(query.str()));

    return newObservation(result);
    
}


std::list<qabase::Observation *> KvalobsDatabaseAccess::selectLatestDataBasedOnObstimeForControl( int limit) {
  //Ensure that we process the latest data the last 3 hours first.
  //Truncate the search criteria to hour, with minutes, seconds, microsecond set to zero.
  
  //SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o WHERE q.observationid=o.observationid AND qa_start IS NULL AND process_start is not null AND obstime >= date_trunc('hour', now()-'3 hours'::interval) ORDER BY priority, obstime LIMIT 1;


  std::ostringstream query; 
  
  if( qaId>-1) {
    query << "SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o "
      "WHERE q.observationid=o.observationid AND qa_start IS NULL AND process_start is not null" 
      " AND obstime >= date_trunc('hour', now()-'3 hours'::interval) AND (qa_id=" << qaId << " OR qa_id is null) "
      "ORDER BY qa_id, priority, obstime LIMIT "<< limit << ";";
  } else {
    query << "SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o "
      "WHERE q.observationid=o.observationid AND qa_start IS NULL AND process_start is not null" 
      " AND obstime >= date_trunc('hour', now()-'3 hours'::interval) "
      "ORDER BY priority, obstime LIMIT "<< limit << ";";
  }

  std::unique_ptr<dnmi::db::Result> result(connection_->execQuery(query.str()));

  return newObservation(result);
}  

std::list<qabase::Observation *> KvalobsDatabaseAccess::selectOlderDataControl(int limit) {
  //Process older data sorted by tbtime. Process the latest observations first. 
  //SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o WHERE q.observationid=o.observationid AND process_start is not null AND qa_start IS NULL ORDER BY priority, tbtime LIMIT 1;
  std::ostringstream query;
  
  if( qaId>-1) {
    query << "SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o "
      "WHERE q.observationid=o.observationid AND process_start is not null AND qa_start IS NULL AND (qa_id=" << qaId << " OR qa_id is null ) "
      "ORDER BY qa_id, priority, tbtime LIMIT " << limit <<";";
  } else {
    query << "SELECT o.observationid, stationid, typeid, obstime, o.tbtime FROM workque q, observations o "
      "WHERE q.observationid=o.observationid AND process_start is not null AND qa_start IS NULL "
      "ORDER BY priority, tbtime LIMIT " << limit <<";";
  }

  std::unique_ptr<dnmi::db::Result> result(connection_->execQuery(query.str()));
  return newObservation(result);
}

std::list<qabase::Observation *>
KvalobsDatabaseAccess::newObservation(std::unique_ptr<dnmi::db::Result> &r)
{
  std::list<qabase::Observation *> ret;
  if (!r || !r->hasNext()) {
    return ret;
  }

  while( r->hasNext() ) {
    auto row = r->next();
    long long observationid = boost::lexical_cast<long long>(row[0]);
    int station = boost::lexical_cast<int>(row[1]);
    int type = boost::lexical_cast<int>(row[2]);
    boost::posix_time::ptime obstime =
      boost::posix_time::time_from_string(row[3]);
    boost::posix_time::ptime tbtime = boost::posix_time::time_from_string(row[4]);
    ret.push_back(new qabase::Observation(observationid, station, type, obstime, tbtime));
  }
  return ret;
}

std::list<qabase::Observation *> KvalobsDatabaseAccess::selectDataForControl(int limit) 
{
  std::string whichSelect="(Crached checks)";
  bool needOwnTransaction = !connection_->transactionInProgress();
  if (needOwnTransaction) {
    LOGINFO("trying to select data for control - SERIALIZABLE" );
    connection_->beginTransaction(dnmi::db::Connection::SERIALIZABLE);
  } else {
    LOGWARN("trying to select data for control with previous transaction - isolation level not guaranteed");
  }

  try {
    std::list<qabase::Observation *> obs=selectFailedDataForControl(10);

    if( obs.empty() ) {
      whichSelect="(Latest obs)";
      obs = selectLatestDataBasedOnObstimeForControl( limit );
    }

    int n=limit/2;
    if ( n<=0 )
      n=1;

    if( obs.empty() ) {
      whichSelect="(Older obs)";
      obs = selectOlderDataControl(n);
    }

    if(obs.empty()) {
      LOGINFO("No observation selected for controll.");
      if (needOwnTransaction)
        connection_->commit();
    
      return obs;
    }

    std::ostringstream query;
    for ( auto o : obs ) {
      query << "UPDATE workque SET qa_start=statement_timestamp() WHERE "
            << "observationid=" << o->id() << ";";
    }

    milog::LogContext context("query");
    LOGDEBUG1(query.str());

    connection_->exec(query.str());
    if (needOwnTransaction)
      connection_->commit();
    
    auto p =(*obs.begin())->id();


    if ( obs.size() == 1 ) {
      LOGINFO("Selected for control "<< whichSelect <<": " << (*obs.begin())->stationID() <<"/" 
        << (*obs.begin())->typeID()  << "/" << pt::to_kvalobs_string((*obs.begin())->obstime()) <<"/" 
        << pt::to_kvalobs_string((*obs.begin())->tbtime()) << " (" << (*obs.begin())->id() << ")");
    } else {
      std::ostringstream log;
      log << "Selected for control "<< whichSelect <<": #observations " << obs.size() << "\n";
      for ( auto a : obs ) {
        log << (*obs.begin())->stationID() <<"/" << (*obs.begin())->typeID()  << "/" 
            << pt::to_kvalobs_string((*obs.begin())->obstime()) <<"/" 
            << pt::to_kvalobs_string((*obs.begin())->tbtime()) << " (" << (*obs.begin())->id() << ")";
      }
      LOGINFO(log.str()); 
    }
    return obs;
  }
  catch (const std::exception &e ) {
    LOGWARN("EXCEPTION: selectDataForControl (1): " << e.what());
    throw;
  }
  catch ( ... ) {
    LOGWARN("EXCEPTION: selectDataForControl: Unknown exception");
    throw;
  }
}


#if 0
qabase::Observation * KvalobsDatabaseAccess::selectDataForControl() {
  std::string whichSelect="(Crached checks)";
  std::string selectProgres="(Crached checks)";
  bool needOwnTransaction = !connection_->transactionInProgress();
  if (needOwnTransaction) {
    LOGINFO("trying to select data for control - SERIALIZABLE" );
    connection_->beginTransaction(dnmi::db::Connection::SERIALIZABLE);
  } else {
    LOGWARN("trying to select data for control with previous transaction - isolation level not guaranteed");
  }

  try {
    qabase::Observation *obs=selectFailedDataForControl();

    if( ! obs ) {
      whichSelect="(Latest obs)";
      obs = selectLatestDataBasedOnObstimeForControl();
    }

    if( ! obs ) {
      whichSelect="(Older obs)";
      obs = selectOlderDataControl();
    }

    if(!obs) {
      LOGINFO("No observation selected for controll.");
      if (needOwnTransaction)
        connection_->commit();
    
      return nullptr;
    }

    std::ostringstream query;
    query << "UPDATE workque SET qa_start=statement_timestamp() WHERE ";
    query << "observationid=" << obs->id();

    milog::LogContext context("query");
    LOGDEBUG1(query.str());

    connection_->exec(query.str());
    if (needOwnTransaction)
      connection_->commit();
    
    LOGINFO("Selected for control "<< whichSelect <<": " << obs->stationID() <<"/" << obs->typeID() << "/" << pt::to_kvalobs_string(obs->obstime())<<"/" << pt::to_kvalobs_string(obs->tbtime())
      <<"("<< obs->id() << ")");
    return obs;
  }
  catch (const std::exception &e ) {
    LOGWARN("EXCEPTION: selectDataForControl: " << e.what());
    throw;
  }
  catch (const std::exception &e ) {
    LOGWARN("EXCEPTION: selectDataForControl: Unknown exception");
    throw;
  }
}
#endif


/*
qabase::Observation * KvalobsDatabaseAccess::selectDataForControl() {

    bool needOwnTransaction = !connection_->transactionInProgress();
    if (needOwnTransaction)
      connection_->beginTransaction(dnmi::db::Connection::SERIALIZABLE);
    else
      LOGWARN("trying to select data for control with previous transaction - isolation level not guaranteed");

    std::vector<int> runningChecks;
    std::unique_ptr<dnmi::db::Result> result(connection_->execQuery("select o.stationid from workque q, observations o where o.observationid=q.observationid and qa_start is not null and qa_stop is null;"));
    while (result->hasNext()) {
      auto row = result->next();
      int station = boost::lexical_cast<int>(row[0]);
      runningChecks.push_back(station);
    }

    std::ostringstream query;

    query << "select o.observationid, stationid, typeid, obstime, o.tbtime from workque q, observations o "
      "where q.observationid=o.observationid and process_start is not null and "
      "((qa_start<now()-'10 minutes'::interval and qa_stop is null) or "
      "(qa_start is null ";
    if (not runningChecks.empty()) {
      query << " and stationid not in (";
      auto it = runningChecks.begin();
      query << *it;
      while (++it != runningChecks.end()) {
        query << ", " << *it;
      }
      query << ")";
    }
    query << ")) order by priority, tbtime limit 1;";

    result.reset(connection_->execQuery(query.str()));
    if (!result || !result->hasNext()) {
      return nullptr;
    }

    auto row = result->next();

    long long observationid = boost::lexical_cast<long long>(row[0]);
    int station = boost::lexical_cast<int>(row[1]);
    int type = boost::lexical_cast<int>(row[2]);
    boost::posix_time::ptime obstime = boost::posix_time::time_from_string(row[3]);
    boost::posix_time::ptime tbtime = boost::posix_time::time_from_string(row[4]);

    query.str("");
    query << "UPDATE workque SET qa_start=statement_timestamp() WHERE ";
    query << "observationid=" << observationid;

    milog::LogContext context("query");
    LOGDEBUG1(query.str());

    connection_->exec(query.str());
    if (needOwnTransaction)
      connection_->commit();
    
    LOGINFO("Selected for controll: "<< station <<"/" << type << "/" << pt::to_kvalobs_string(obstime)<<"/" << pt::to_kvalobs_string(tbtime)<<"("<< observationid << ")");
    return new qabase::Observation(observationid, station, type, obstime, tbtime);
}
*/
void KvalobsDatabaseAccess::markProcessDone(const qabase::Observation & obs) {
    std::ostringstream query;

    query << "UPDATE workque SET qa_stop=statement_timestamp() WHERE observationid=" << obs.id();

    milog::LogContext context("query");
    LOGDEBUG1(query.str());

    connection_->exec(query.str());
}

dnmi::db::Connection * KvalobsDatabaseAccess::createConnection(
    const std::string & databaseConnect) {
  static std::string driverId;
  if (driverId.empty()) {
    std::string driver = kvalobs::kvPath(kvalobs::pkglibdir)
        + "/db/pgdriver.so";

    if (!dnmi::db::DriverManager::loadDriver(driver, driverId))
      throw std::runtime_error("Unable to load driver " + driver);
  }

  dnmi::db::Connection * conn = dnmi::db::DriverManager::connect(driverId, databaseConnect);
  return conn;
}

void KvalobsDatabaseAccess::storeFetched(long long obsid, const kvalobs::kvData & d) const {
  DataID::value_type toInsert(d, obsid);
  auto insertResult = fetchedData_.insert(toInsert);
  if (!insertResult.second) {
    long long oldId = insertResult.first->second;
    if (obsid != oldId) {
      std::ostringstream s;
      s << "obsid mismatch: got " << obsid << " previous was " << oldId << " data: " << d;
      throw std::runtime_error(s.str());
    }
  }
}

void KvalobsDatabaseAccess::storeFetched(long long obsid, const kvalobs::kvTextData & d) const {
  TextDataID::value_type toInsert(d, obsid);
  auto insertResult = fetchedTextData_.insert(toInsert);
  if (!insertResult.second) {
    long long oldId = insertResult.first->second;
    if (obsid != oldId) {
      std::ostringstream s;
      s << "textdata obsid mismatch: got " << obsid << " previous was " << oldId << " data: " << d;
      throw std::runtime_error(s.str());
    }
  }
}

}  // namespace db
