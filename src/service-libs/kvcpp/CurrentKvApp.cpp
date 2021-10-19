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

#include <string>
#include <vector>
#include <fstream>
#include <libgen.h>
#include <boost/filesystem.hpp>
#include <boost/static_assert.hpp>
#include "miconfparser/confsection.h"
#include "miconfparser/confparser.h"
#include "milog/milog.h"
#include "kvalobs/kvPath.h"
#include "kvdb/kvdb.h"
#include "kvdb/dbdrivermgr.h"
#include "kvsubscribe/HttpSendData.h"
#include "test/configuration.h"
#include "CurrentKvApp.h"

namespace kvservice {

using dnmi::db::Connection;

namespace {

using boost::filesystem::path;

std::string getValue(const std::string & key, std::shared_ptr<miutil::conf::ConfSection> conf) {
  auto val = conf->getValue(key);
  if (val.empty())
    throw std::runtime_error("missing <" + key + "> in config file");
  if (val.size() > 1)
    throw std::runtime_error("Too many entries for <" + key + "> in config file");
  return val.front().valAsString();
}

std::string removePassword(const std::string &connect) {
  using namespace std;
  string con(connect);
  int i = con.find("password");
  int k;

  if( i == string::npos )
    return con;

  k=con.find_first_of("=", i);
  if( k == string::npos) {
    k=con.length();
  } else {
    k=con.find_first_of(" \n\t\r", k);
    if( k==string::npos)
      k=con.length();
  }

  return con.replace(i, k-i, "password=*******");
}

dnmi::db::Connection * createConnection(std::shared_ptr<miutil::conf::ConfSection> conf) {
  std::string connectString = getValue("database.dbconnect", conf);
  std::string driver = kvalobs::kvPath(kvalobs::pkglibdir) + "/db/" + getValue("database.dbdriver", conf);

  std::string driverId;
  if (!dnmi::db::DriverManager::loadDriver(driver, driverId))
    throw std::runtime_error("Unable to load driver " + driver);

  dnmi::db::Connection * connection = dnmi::db::DriverManager::connect(driverId, connectString);
  if (!connection or not connection->isConnected()) {

    throw std::runtime_error("Unable to connect to database ("+driverId+") connect: '"+removePassword(connectString)+"'. Reason: "+dnmi::db::DriverManager::getErr());
  }
  return connection;
}

std::function<Connection*()> connector(int argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection> preferredConfig) {
  std::shared_ptr<miutil::conf::ConfSection> config = KvApp::getConfiguration(preferredConfig, boost::filesystem::basename(argv[0]));
  return [config]() {
    return createConnection(config);
  };
}

void releaseConnection(dnmi::db::Connection * connection) {
  dnmi::db::DriverManager::releaseConnection(connection);
}

std::string kafkaDomain(int argc, char **argv, std::shared_ptr<miutil::conf::ConfSection> preferredConfig) {
  std::shared_ptr<miutil::conf::ConfSection> config = KvApp::getConfiguration(preferredConfig, boost::filesystem::basename(argv[0]));
  auto ret = getValue("kafka.domain", config);
  LOGINFO("kafka.domain: '"<< ret << "'");
  return ret;
}

std::string kafkaBrokers(int argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection> preferredConfig) {
  std::shared_ptr<miutil::conf::ConfSection> config = KvApp::getConfiguration(preferredConfig, boost::filesystem::basename(argv[0]));

  return getValue("kafka.brokers", config);
}

}

CurrentKvApp::CurrentKvApp(int argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection> preferredConfig)
    : sql::SqlGet(connector(argc, argv, preferredConfig), releaseConnection),
      kafka::KafkaSubscribe(kafkaDomain(argc, argv, preferredConfig), kafkaBrokers(argc, argv, preferredConfig)) {
  std::shared_ptr<miutil::conf::ConfSection> conf = KvApp::getConfiguration(preferredConfig, boost::filesystem::basename(argv[0]));
  sendData_ = std::unique_ptr<kvalobs::datasource::SendData>(new kvalobs::datasource::HttpSendData(*conf));
  // needed for correct handling of CORBA::string_dup, below
  int ac = 1;
  char * av = const_cast<char*>("fake");
  CORBA::ORB_init(ac, &av);
}

CurrentKvApp::~CurrentKvApp() {
}

// If any of these fail, you must reimplement sendDataToKv
BOOST_STATIC_ASSERT(kvalobs::datasource::OK == (kvalobs::datasource::EResult) CKvalObs::CDataSource::OK);
BOOST_STATIC_ASSERT(kvalobs::datasource::NODECODER == (kvalobs::datasource::EResult) CKvalObs::CDataSource::NODECODER);
BOOST_STATIC_ASSERT(kvalobs::datasource::DECODEERROR == (kvalobs::datasource::EResult) CKvalObs::CDataSource::DECODEERROR);
BOOST_STATIC_ASSERT(kvalobs::datasource::NOTSAVED == (kvalobs::datasource::EResult) CKvalObs::CDataSource::NOTSAVED);
BOOST_STATIC_ASSERT(kvalobs::datasource::ERROR == (kvalobs::datasource::EResult) CKvalObs::CDataSource::ERROR);

const CKvalObs::CDataSource::Result_var CurrentKvApp::sendDataToKv(const char *data, const char *obsType) {
  try {
    kvalobs::datasource::Result result = sendData_->newData(data, obsType);

    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = (CKvalObs::CDataSource::EResult) result.res;
    r->message = CORBA::string_dup(result.message.c_str());
    return r;
  } catch (std::exception & e) {
    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
    r->res = CKvalObs::CDataSource::ERROR;
    r->message = CORBA::string_dup(e.what());
    return r;
  }
}

namespace test {

std::shared_ptr<miutil::conf::ConfSection> getConfiguration(std::shared_ptr<miutil::conf::ConfSection> preferredConf, const std::string & application,
                                                            bool reset) {
  return kvservice::KvApp::getConfiguration(preferredConf, application, reset);
}

miutil::conf::ConfSection * readConf(const path & configFile) {
  return kvservice::readConf(configFile);
}

}  // namespace test

} /* namespace kvservice */
