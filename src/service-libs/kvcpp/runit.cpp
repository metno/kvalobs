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

#include "sql/SqlKvApp.h"
#include <boost/timer/timer.hpp>
#include <kvalobs/kvPath.h>
#include <miutil/timeconvert.h>
#include <milog/milog.h>
#include <iostream>
#include <stdexcept>
#include <memory>

using boost::timer::cpu_timer;
using kvservice::KvApp;

static const std::string OK = "\t\033[1;32m+";
static const std::string NOT_OK = "\t\033[1;31m-";
static const std::string EOL = "\033[0m\n";

kvservice::KvApp * getApp(int argc, char ** argv) {
  using kvservice::sql::SqlKvApp;
  return new SqlKvApp(argc, argv, SqlKvApp::readConf( { "runit.conf",
      "kvalobs.conf", kvPath("sysconfdir") + "/kvalobs.conf" }));
}

int test(std::string testDescription, bool test) {
  if (test) {
    std::cout << OK << testDescription << EOL;
    return 1;
  }
  std::cout << NOT_OK << testDescription << EOL;
  return 0;
}

int readModelData() {
  kvservice::WhichDataHelper whichData;
  whichData.addStation(
      100, boost::posix_time::time_from_string("2015-12-02 00:00:00"),
      boost::posix_time::time_from_string("2015-12-02 12:00:00"));

  std::list<kvalobs::kvModelData> data;
  int res = test(" retrieve model data.",
                 KvApp::kvApp->getKvModelData(data, whichData));
  //for ( auto d : data )
  //	std::cout << d.insertQuery(false) << std::endl;
  return res;
}

int readData() {
  kvservice::KvObsDataList dataList;
  kvservice::WhichDataHelper whichData;
  whichData.addStation(
      4260, boost::posix_time::time_from_string("2015-11-09 12:00:00"),
      boost::posix_time::time_from_string("2015-11-09 12:00:00"));
  whichData.addStation(
      180, boost::posix_time::time_from_string("2015-11-09 12:00:00"),
      boost::posix_time::time_from_string("2015-11-09 12:00:00"));
  int res = test(" retrieve observation data.",
                 KvApp::kvApp->getKvData(dataList, whichData));
  /*
   for ( auto dl : dataList )
   {
   for ( auto d: dl.dataList() )
   std::cout << "d :" << d.stationID() << '/' << d.paramID() << ":\t" << d.original() << std::endl;
   for ( auto d: dl.textDataList() )
   std::cout << "t: " << d.stationID() << '/' << d.paramID() << ":\t" << d.original() << std::endl;

   std::cout << std::endl;
   }
   */
  return res;
}

int readParams() {
  std::list<kvalobs::kvParam> paramList;
  return test(" retrieve parameter metadata for all parameters.",
              KvApp::kvApp->getKvParams(paramList));
  //for ( auto p : paramList )
  //	std::cout << p.name() << ":\t" << p.paramID() << std::endl;
}

int readWorkStatistik() {
  kvservice::WorkstatistikIterator it;
  int res = test(
      " retrieve wirk statistics metadata.",
      KvApp::kvApp->getKvWorkstatistik(
          CKvalObs::CService::TbTime,
          boost::posix_time::time_from_string("2015-11-09 12:00:00"),
          boost::posix_time::time_from_string("2015-11-09 12:30:00"), it));
  //kvalobs::kvWorkelement e;
  //while (it.next(e))
  //	std::cout << e.insertQuery(false) << std::endl;
  return res;
}

int readRejectDecode() {
  CKvalObs::CService::RejectDecodeInfo decodeInfo;
  decodeInfo.fromTime = "2015-11-10 12:00:00";
  decodeInfo.toTime = "2015-11-10 12:30:00";
  kvservice::RejectDecodeIterator it;
  int res = test(" retrieve rejectdecode messages.",
                 KvApp::kvApp->getKvRejectDecode(decodeInfo, it));
  //kvalobs::kvRejectdecode d;
  //while ( it.next(d) )
  //	std::cout << d.tbtime() << ":\t" << d.message() << std::endl;
  return res;
}

int readStationMetaData() {
  std::list<kvalobs::kvStationMetadata> stMeta;
  int stationid = 0;
  auto obstime = boost::posix_time::time_from_string("2015-11-24 12:00:00");
  std::string metadataName;  // = "VS";
  int res = test(
      " retrieve obspgm metadata.",
      KvApp::kvApp->getKvStationMetaData(stMeta, stationid, obstime,
                                         metadataName));
  //for ( auto m : stMeta )
  //	std::cout << "METADATA: " << m.paramID() << " (" << m.name() << "): " << m.metadata() << std::endl;
  return res;
}

int readOperator() {
  std::list<kvalobs::kvOperator> operatorList;
  int res = test(" retrieve operator metadata.",
                 KvApp::kvApp->getKvOperator(operatorList));
  //for ( auto o : operatorList )
  //	std::cout << o.insertQuery(false) << ';' << std::endl;
  return res;
}

int readStationParamAll() {
  std::list<kvalobs::kvStationParam> stParamList;
  int res = test(" retrieve parameter data for a given station. ",
                 KvApp::kvApp->getKvStationParam(stParamList, 18700, -1, -1));
  //for ( auto s : stParamList )
  //	std::cout << s.stationID() << ":\t" << s.paramID() << ":\t" << s.fromday()
  //			  << ":\t" << s.today() << std::endl;
  std::ostringstream log;
  log << "retrieved " << stParamList.size() << " rows";
  LOGDEBUG(log.str());
  return res;
}

int readStationParamForParamId() {
  std::list<kvalobs::kvStationParam> stParamList;
  int res = test(" retrieve parameter data for a given station and paramid.",
                 KvApp::kvApp->getKvStationParam(stParamList, 18700, 251, -1));
  std::ostringstream log;
  log << "retrieved " << stParamList.size() << " rows";
  LOGDEBUG(log.str());
  return res;
}

int readStationParamForDay() {
  std::list<kvalobs::kvStationParam> stParamList;
  int res = test(" retrieve parameter data for a given station and day.",
                 KvApp::kvApp->getKvStationParam(stParamList, 18700, -1, 200));
  std::ostringstream log;
  log << "retrieved " << stParamList.size() << " rows";
  LOGDEBUG(log.str());
  return res;
}

int readStationParamForParamIdAndDay() {
  std::list<kvalobs::kvStationParam> stParamList;
  int res = test(
      " retrieve parameter data for a given station, paramid, and day.",
      KvApp::kvApp->getKvStationParam(stParamList, 18700, 251, 200));
  std::ostringstream log;
  log << "retrieved " << stParamList.size() << " rows";
  LOGDEBUG(log.str());
  return res;
}

int readStations() {
  std::list<kvalobs::kvStation> stationList;
  int res = test(" retrieve stations metadata for all stations.",
                 KvApp::kvApp->getKvStations(stationList));
  //for ( auto s : stationList )
  //	std::cout << s.name() << ":\t" << s.stationID() << std::endl;
  return res;
}

int readObsPgm() {
  std::list<kvalobs::kvObsPgm> obsPgm;
  const std::list<long> stationList = { 180, 100 };
  int res = test(" retrieve obspgm metadata.",
                 KvApp::kvApp->getKvObsPgm(obsPgm, stationList, true));
  //for ( auto op : obsPgm )
  //  std::cout << op.insertQuery(false) << std::endl;
  return res;
}

int readTypes() {
  std::list<kvalobs::kvTypes> types;
  int res = test(" retrieve types metadata.", KvApp::kvApp->getKvTypes(types));
  //for ( auto tp : types )
  //  std::cout << tp.typeID() << std::endl;
  return res;
}

int main(int argc, char ** argv) {
  std::unique_ptr<kvservice::KvApp> app(getApp(argc, argv));
  const int totalTests = 13;
  int t = 0;
  std::cout << "\033[1;36mSqlKvApp should" << EOL;
  cpu_timer test_timer;
  // Actual tests
  t += readParams();
  t += readStations();
  t += readStationParamAll();
  t += readStationParamForParamId();
  t += readStationParamForDay();
  t += readStationParamForParamIdAndDay();
  t += readOperator();
  t += readWorkStatistik();
  t += readData();
  t += readObsPgm();
  t += readModelData();
  t += readStationMetaData();
  t += readTypes();
  // Print out results
  int time = test_timer.elapsed().wall / 1000000;
  std::cout << "\033[1;36mFinished in " << time << "ms" << EOL;
  std::cout << "\033[1;36mPassed " << t << " of " << totalTests << " tests"
            << EOL;
  return (t == totalTests ? 0 : 1);
}
