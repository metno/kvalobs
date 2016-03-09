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

miutil::conf::ConfSection * readConf(const path & configFile) {
  if (!exists(configFile) || !is_regular_file(configFile))
    throw std::runtime_error("Not a regular file");

  std::ifstream fis(configFile.c_str());
  if (!fis)
    throw std::runtime_error("Cant open the configuration file <" + configFile.string() + ">!");

  LOGINFO("Reading configuration from file <" << configFile.string() << ">!");
  miutil::conf::ConfParser parser;
  miutil::conf::ConfSection * conf = parser.parse(fis);
  if (!conf)
    throw std::runtime_error(parser.getError());
  LOGDEBUG("Configuration file loaded!");

  return conf;
}

std::shared_ptr<miutil::conf::ConfSection> getConfiguration(std::shared_ptr<miutil::conf::ConfSection> preferredConf, const std::string & application,
                                                            bool reset = false) {
  static std::shared_ptr<miutil::conf::ConfSection> conf;

  if (reset)  // Mostly for test.
    conf = std::shared_ptr<miutil::conf::ConfSection>();

  if (!conf && preferredConf) {
    conf = preferredConf;
  }

  if (!conf) {
    path baseConfigPath = kvPath("sysconfdir");
    std::ostringstream msg;
    std::vector<path> configAlternatives = { baseConfigPath / (application + ".conf"), baseConfigPath / "kvalobs.conf" };
    for (const path & p : configAlternatives) {
      try {
        msg << " " << p;
        conf.reset(readConf(p));
        break;
      } catch (std::exception & e) {
        LOGDEBUG(e.what());
      }
    }

    if (!conf)
      throw std::runtime_error("No config file found. Searched for: " + msg.str() + ".");
  }
  return conf;
}

std::string getValue(const std::string & key, std::shared_ptr<miutil::conf::ConfSection> conf) {
  auto val = conf->getValue(key);
  if (val.empty())
    throw std::runtime_error("missing <" + key + "> in config file");
  if (val.size() > 1)
    throw std::runtime_error("Too many entries for <" + key + "> in config file");
  return val.front().valAsString();
}

dnmi::db::Connection * createConnection(std::shared_ptr<miutil::conf::ConfSection> conf) {
  std::string connectString = getValue("database.dbconnect", conf);
  std::string driver = kvalobs::kvPath(kvalobs::libdir) + "/kvalobs/db/" + getValue("database.dbdriver", conf);

  std::string driverId;
  if (!dnmi::db::DriverManager::loadDriver(driver, driverId))
    throw std::runtime_error("Unable to load driver " + driver);

  dnmi::db::Connection * connection = dnmi::db::DriverManager::connect(driverId, connectString);
  if (!connection or not connection->isConnected())
    throw std::runtime_error("Unable to connect to database");
  return connection;
}

std::function<Connection*()> connector(int argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection> preferredConfig) {
  std::shared_ptr<miutil::conf::ConfSection> config = getConfiguration(preferredConfig, boost::filesystem::basename(argv[0]));
  return [config]() {
    return createConnection(config);
  };
}

void releaseConnection(dnmi::db::Connection * connection) {
  dnmi::db::DriverManager::releaseConnection(connection);
}

std::string kafkaDomain(int argc, char **argv, std::shared_ptr<miutil::conf::ConfSection> preferredConfig) {
  std::shared_ptr<miutil::conf::ConfSection> config = getConfiguration(preferredConfig, boost::filesystem::basename(argv[0]));
  return getValue("kafka.domain", config);
}

std::string kafkaBrokers(int argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection> preferredConfig) {
  std::shared_ptr<miutil::conf::ConfSection> config = getConfiguration(preferredConfig, boost::filesystem::basename(argv[0]));
  return getValue("kafka.brokers", config);
}

}

CurrentKvApp::CurrentKvApp(int argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection> preferredConfig)
    : sql::SqlGet(connector(argc, argv, preferredConfig), releaseConnection),
      kafka::KafkaSubscribe(kafkaDomain(argc, argv, preferredConfig), kafkaBrokers(argc, argv, preferredConfig)) {
  std::shared_ptr<miutil::conf::ConfSection> conf = getConfiguration(preferredConfig, boost::filesystem::basename(argv[0]));
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
  return kvservice::getConfiguration(preferredConf, application, reset);
}

miutil::conf::ConfSection * readConf(const path & configFile) {
  return kvservice::readConf(configFile);
}

}  // namespace test

} /* namespace kvservice */
