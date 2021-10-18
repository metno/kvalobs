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
#include "SqlKvApp.h"
#include "KvDataHandler.h"
#include <kvdb/kvdb.h>
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvPath.h>
#include <miutil/timeconvert.h>
#include <milog/milog.h>
#include <miutil/timeconvert.h>
#include <sstream>
#include <iomanip>

namespace kvservice {
namespace sql {
namespace {
std::string getValue(const std::string & key,
                     const miutil::conf::ConfSection *conf) {
  auto val = conf->getValue(key);
  if (val.empty())
    throw std::runtime_error("missing <" + key + "> in config file");
  if (val.size() > 1)
    throw std::runtime_error(
        "Too many entries for <" + key + "> in config file");
  return val.front().valAsString();
}


dnmi::db::Connection * createConnection(const miutil::conf::ConfSection *conf) {
  std::string connectString = getValue("database.dbconnect", conf);
  std::string driver = kvalobs::kvPath(kvalobs::pkglibdir) + "/db/"
      + getValue("database.dbdriver", conf);

  std::string driverId;
  if (!dnmi::db::DriverManager::loadDriver(driver, driverId))
    throw std::runtime_error("Unable to load driver " + driver);

  dnmi::db::Connection * connection = dnmi::db::DriverManager::connect(driverId, connectString);
  if (!connection or not connection->isConnected())
    throw std::runtime_error("Unable to connect to database");
  return connection;
}

void releaseConnection(dnmi::db::Connection * connection) {
  dnmi::db::DriverManager::releaseConnection(connection);
}
}

dnmi::db::Connection * SqlKvApp::ConnectionCreator::operator ()() const {
  return createConnection(conf_);
}

SqlKvApp::SqlKvApp(int &argc, char **argv, miutil::conf::ConfSection *conf)
    : corba::CorbaKvApp(argc, argv, conf),
      connectionCreator_(conf) {
  get_ = new SqlGet(connectionCreator_, releaseConnection);
}

SqlKvApp::~SqlKvApp() {
  delete get_;
}

bool SqlKvApp::getKvData(KvGetDataReceiver &dataReceiver,
                         const WhichDataHelper &wd) {
  return get_->getKvData(dataReceiver, wd);
}

bool SqlKvApp::getKvRejectDecode(
    const CKvalObs::CService::RejectDecodeInfo &decodeInfo,
    kvservice::RejectDecodeIterator &it) {
  return get_->getKvRejectDecode(decodeInfo, it);
}

bool SqlKvApp::getKvParams(std::list<kvalobs::kvParam> &paramList) {
  return get_->getKvParams(paramList);
}

bool SqlKvApp::getKvStations(std::list<kvalobs::kvStation> &stationList) {
  return get_->getKvStations(stationList);
}

bool SqlKvApp::getKvModelData(std::list<kvalobs::kvModelData> &dataList,
                              const WhichDataHelper &wd) {
  return get_->getKvModelData(dataList, wd);
}

bool SqlKvApp::getKvTypes(std::list<kvalobs::kvTypes> &typeList) {
  return get_->getKvTypes(typeList);
}

bool SqlKvApp::getKvOperator(std::list<kvalobs::kvOperator> &operatorList) {
  return get_->getKvOperator(operatorList);
}

bool SqlKvApp::getKvStationParam(std::list<kvalobs::kvStationParam> &stParam,
                                 int stationid, int paramid, int day) {
  return get_->getKvStationParam(stParam, stationid, paramid, day);
}

bool SqlKvApp::getKvStationMetaData(
    std::list<kvalobs::kvStationMetadata> &stMeta, int stationid,
    const boost::posix_time::ptime &obstime, const std::string & metadataName) {
  return get_->getKvStationMetaData(stMeta, stationid, obstime, metadataName);
}

bool SqlKvApp::getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
                           const std::list<long> &stationList, bool aUnion) {
  return get_->getKvObsPgm(obsPgm, stationList, aUnion);
}

bool SqlKvApp::getKvData(KvObsDataList &dataList, const WhichDataHelper &wd) {
  return get_->getKvData(dataList, wd);
}

bool SqlKvApp::getKvWorkstatistik(
    CKvalObs::CService::WorkstatistikTimeType timeType,
    const boost::posix_time::ptime &from, const boost::posix_time::ptime &to,
    kvservice::WorkstatistikIterator &it) {
  return get_->getKvWorkstatistik(timeType, from, to, it);
}

} /* namespace sql */
} /* namespace kvservice */
