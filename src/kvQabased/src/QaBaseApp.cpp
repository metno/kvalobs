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

#include "QaBaseApp.h"
#include <string>
#include "kvalobs/kvPath.h"
#include "kvsubscribe/KafkaProducer.h"
#include "kvsubscribe/queue.h"
#include "miconfparser/miconfparser.h"
#include "DataProcessor.h"




namespace qabase {

PgQueueConfig QaBaseApp::pgQueueConf_;
std::shared_ptr<PgCluster> QaBaseApp::pgCluster_; 
bool QaBaseApp::pgQueueEnabled_;

namespace {
std::string val(const std::string & name, miutil::conf::ConfSection * conf, const std::string & defaultValue) {
  if (conf) {
    miutil::conf::ValElementList ret = conf->getValue(name);
    if (ret.empty())
      return defaultValue;
    if (ret.size() > 1)
      throw std::runtime_error("Several " + name + " elements in config!");
    return ret.valAsString(defaultValue);
  } else
    return defaultValue;
}

int valAsInt(const std::string & name, miutil::conf::ConfSection * conf, int  defaultValue) {
  if (conf) {
    miutil::conf::ValElementList ret = conf->getValue(name);
    if (ret.empty())
      return defaultValue;
    if (ret.size() > 1)
      throw std::runtime_error("Several " + name + " elements in config!");
    return ret.valAsInt(defaultValue);
  } else
    return defaultValue;
}

bool valAsBool(const std::string & name, miutil::conf::ConfSection * conf, bool  defaultValue) {
  if (conf) {
    miutil::conf::ValElementList ret = conf->getValue(name);
    if (ret.empty())
      return defaultValue;
    if (ret.size() > 1)
      throw std::runtime_error("Several " + name + " elements in config!");
    return ret.valAsBool(defaultValue);
  } else
    return defaultValue;
}


}

QaBaseApp::QaBaseApp(int argc, char ** argv)
    : KvBaseApp(argc, argv) {

  miutil::conf::ConfSection * pg = getConfiguration()->getSection("pgqueue");

  pgQueueConf_.enable = false;
  if ( valAsBool("enable", pg, false)) {
    pgQueueConf_.enable = true;
  }
  pgQueueEnabled_ = pgQueueConf_.enable;

  std::string con = val("database_a", pg, "");
  if (!con.empty()){
    pgQueueConf_.dbconnect.push_back(con);
  }
  con = val("database_b", pg, "");
  if (!con.empty()){
    pgQueueConf_.dbconnect.push_back(con);
  }
  
  pgQueueConf_.domain = val("domain", pg, "development");
  if (pgQueueConf_.enable && pgQueueConf_.dbconnect.empty()) {
    throw std::runtime_error("No database connection specified in pgqueue config!");
  }
  
  std::stringstream ss;
  ss << "PgQueue Configuration:\n"
     << "     enable: " << (pgQueueConf_.enable?"true":"false") << "\n"
     << "     domain: " << pgQueueConf_.domain << "\n"
     << "  dbconnect: ";
  for (const auto & con : pgQueueConf_.dbconnect) {
    ss << "\n    - " << con;
  }
  LOGINFO("PgQueue Configuration:\n" << ss.str() << "\n");
}

QaBaseApp::~QaBaseApp() {
}

bool QaBaseApp::queueEnabledInConfig() {
  return pgQueueEnabled_;
};


std::shared_ptr<kvalobs::subscribe::PgProducer> QaBaseApp::pgqueueProducer() {
  using kvalobs::subscribe::PgProducer;

  if( !pgQueueEnabled_) {
    return nullptr;
  }
  std::string topic = pgQueueConf_.getPublishTopic();

  LOGINFO("Creating pgqueue connection on " << topic);

  return std::make_shared<PgProducer>(topic, pgQueueConf_.dbconnect, "qabased", DataProcessor::onSendError, DataProcessor::onSendSuccess);
}


std::string QaBaseApp::baseLogDir() {
  return kvalobs::kvPath(kvalobs::logdir) + "/checks/";
}

} /* namespace qabase */
