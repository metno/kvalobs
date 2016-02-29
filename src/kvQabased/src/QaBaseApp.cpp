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
#include "NewDataListener.h"

namespace qabase {

std::string QaBaseApp::kafkaBrokers_;
std::string QaBaseApp::kafkaDomain_;

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
}

QaBaseApp::QaBaseApp(int argc, char ** argv)
    : KvBaseApp(argc, argv) {

  miutil::conf::ConfSection * kafka = getConfiguration()->getSection("kafka");
  kafkaBrokers_ = val("brokers", kafka, "localhost");
  kafkaDomain_ = val("domain", kafka, "test");
}

QaBaseApp::~QaBaseApp() {
}

std::shared_ptr<kvalobs::subscribe::KafkaProducer> QaBaseApp::kafkaProducer() {

  using kvalobs::subscribe::KafkaProducer;

  std::string queue = kvalobs::subscribe::queue::checked(kafkaDomain_);

  LOGINFO("Creating kafka connection on " << kafkaBrokers_ << ", using topic " << queue);

  return std::make_shared < KafkaProducer > (queue, kafkaBrokers_, NewDataListener::onKafkaSendError, NewDataListener::onKafkaSendSuccess);
}

std::string QaBaseApp::baseLogDir() {
  return kvalobs::kvPath(kvalobs::logdir) + "/checks/";
}

} /* namespace qabase */
