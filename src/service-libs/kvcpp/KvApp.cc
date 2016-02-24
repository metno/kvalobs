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

namespace kvservice {

KvApp *KvApp::kvApp = 0;

namespace {

miutil::conf::ConfSection * readConf(const std::string &fname) {
  miutil::conf::ConfParser parser;
  miutil::conf::ConfSection *conf;
  std::ifstream fis;

  fis.open(fname.c_str());

  if (!fis) {
    LOGERROR("Cant open the configuration file <" << fname << ">!");
  } else {
    LOGINFO("Reading configuration from file <" << fname << ">!");
    conf = parser.parse(fis);
    if (!conf) {
      LOGERROR(
          "Error while reading configuration file: <" << fname << ">!\n"
              << parser.getError());
    } else {
      LOGINFO("Configuration file loaded!");
      return conf;
    }
  }
  return 0;
}

miutil::conf::ConfSection * getConfSection(
    const std::string & applicationName) {

  std::string myconf = applicationName + ".conf";
  miutil::conf::ConfSection * confSec = readConf(myconf);
  if (!confSec) {
    myconf = kvPath("sysconfdir") + "/kvalobs.conf";
    confSec = readConf(myconf);
  }
  if (!confSec)
    throw std::runtime_error("Cant open conf file: " + myconf);
  return confSec;
}

}

KvApp * KvApp::create(const std::string & applicationName,
                      int argc, char ** argv, std::shared_ptr<miutil::conf::ConfSection> conf) {

  return new CurrentKvApp(argc, argv, conf);
}

KvApp::KvApp() {
  kvApp = this;
}

KvApp::~KvApp() {
  kvApp = 0;
}

}  // namespace kvservice
