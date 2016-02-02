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

#include "CurrentKvApp.h"
#include <libgen.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <miconfparser/confsection.h>
#include <miconfparser/confparser.h>
#include <milog/milog.h>
#include <kvalobs/kvPath.h>
#include <kvdb/kvdb.h>
#include <kvdb/dbdrivermgr.h>

namespace kvservice {

using dnmi::db::Connection;

namespace {

using boost::filesystem::path;

miutil::conf::ConfSection * readConf(const path & configFile) {
  if ( ! exists(configFile) || ! is_regular_file(configFile) )
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

static dnmi::db::DriverManager dbMgr;

dnmi::db::Connection * createConnection(const miutil::conf::ConfSection *conf) {
  std::string connectString = getValue("database.dbconnect", conf);
  std::string driver = kvalobs::kvPath(kvalobs::libdir) + "/kvalobs/db/"
      + getValue("database.dbdriver", conf);

  std::string driverId;
  if (!dbMgr.loadDriver(driver, driverId))
    throw std::runtime_error("Unable to load driver " + driver);

  dnmi::db::Connection * connection = dbMgr.connect(driverId, connectString);
  if (!connection or not connection->isConnected())
    throw std::runtime_error("Unable to connect to database");
  return connection;
}

std::function<Connection*()> connector(int argc, char ** argv, const boost::filesystem::path & preferredConfigFile) {

  path baseConfigPath = kvPath("sysconfdir");

  std::vector<path> configAlternatives = {
      preferredConfigFile,
      baseConfigPath/boost::filesystem::basename(argv[0]),
      baseConfigPath/"kvalobs.conf"
  };

  miutil::conf::ConfSection * config = nullptr;
  for (const path & p : configAlternatives)  {
    try {
      config = readConf(p);
    }
    catch (std::exception & e) {
      LOGDEBUG(e.what());
    }
  }

  return [config]() {
    return createConnection(config);
  };
}

void releaseConnection(dnmi::db::Connection * connection) {
  dbMgr.releaseConnection(connection);
}

}

CurrentKvApp::CurrentKvApp(int argc, char ** argv, const std::string & preferredConfigFile)
    : sql::SqlGet(connector(argc, argv, preferredConfigFile), releaseConnection),
      kafka::KafkaSubscribe("test", "localhost") {
}

CurrentKvApp::~CurrentKvApp() {
}

} /* namespace kvservice */
