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

#include "service-libs/kvcpp/KvApp.h"
#include "CurrentKvApp.h"
#include <kvalobs/kvPath.h>
#include <milog/milog.h>
#include <miconfparser/confparser.h>
#include <fstream>

using boost::filesystem::path;

namespace kvservice {

KvApp *KvApp::kvApp = 0;
std::string KvApp::appName="";

miutil::conf::ConfSection * readConf(const boost::filesystem::path & configFile) {
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

std::shared_ptr<miutil::conf::ConfSection> KvApp::getDefaultConfiguration(const std::string & application) {
  std::shared_ptr<miutil::conf::ConfSection> preferredConf;

  if ( !application.empty() ) {
    return getConfiguration(preferredConf, application);
  } else if( !appName.empty() ) {
    return getConfiguration(preferredConf, appName);
  } else {
    return getConfiguration(preferredConf, "kvalobs");
  }
}


std::shared_ptr<miutil::conf::ConfSection> KvApp::getConfiguration(std::shared_ptr<miutil::conf::ConfSection> preferredConf, const std::string & application,
                                                            bool reset) {
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

std::string KvApp::getConfigValue(const std::string & key,
                  const std::string & fallback) 
{
  auto conf=getDefaultConfiguration();
  if ( !conf ) {
    return fallback;
  }
  auto value = conf->getValue(key);

  if (value.empty())
    return fallback;
  if (value.size() > 1)
    return fallback;

  return value.front().valAsString();
}




std::string KvApp::getConsumerGroupId(const std::string &consumerGroupIdKey) {
  std::string groupIdName="kafka.groupid";

  if( !consumerGroupIdKey.empty() ){
    groupIdName = consumerGroupIdKey;
  } else if( !KvApp::appName.empty() ) {
    groupIdName += "."+KvApp::appName;
  } 
  
  auto val=getConfigValue(groupIdName, "");

  LOGINFO("Consumer group id key: '" << groupIdName << "' value: '" << val << "'");
  return val;
}


KvApp * KvApp::create(const std::string & applicationName,
                      int argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection> conf) {
  if( appName.empty() ){
    appName=applicationName;
  }

  return new CurrentKvApp(argc, argv, conf);
}

KvApp::KvApp() {
  kvApp = this;
}

KvApp::~KvApp() {
  kvApp = 0;
}

}  // namespace kvservice
