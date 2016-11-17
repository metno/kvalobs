/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2007 met.no

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

#include <sstream>
#include "lib/milog/milog.h"
#include "lib/decodeutility/kvalobsdataserializer.h"
#include "lib/miutil/timeconvert.h"
#include "kvDataInputd/PublishDataCommand.h"

using std::string;
using kvalobs::subscribe::KafkaProducer;

PublishDataCommand::PublishDataCommand(const kvalobs::serialize::KvalobsData &pubData)
    : data(kvalobs::serialize::KvalobsDataSerializer::serialize(pubData)), summary(pubData.summary()) {
  std::ostringstream o;
  o << "nObs: " << summary.size();
  if( !summary.empty() ) {
    kvalobs::kvStationInfo info(*summary.begin());
    o << ", first: " << info.stationID() << ":" << info.typeID() << ":" << boost::posix_time::to_kvalobs_string(info.obstime());
    toLog = o.str();
  }
}

const char *PublishDataCommand::getData(unsigned int *size) const {
  *size = data.size();
  return data.data();
}

void PublishDataCommand::onSend(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName) {
  if (!toLog.empty()) {
    IDLOGDEBUG("kafka_pub", "Data(" << msgId << "): SENDT: " << toLog);
  } else if (!data.empty()) {
    IDLOGDEBUG("kafka_pub", "Data(" << msgId << "): SENDT: " << data);
  } else {
    IDLOGDEBUG("kafka_pub", "Data(" << msgId << "): SENDT: empty message.");
  }
}

void PublishDataCommand::onSuccess(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName, const std::string &data) {
  if (!toLog.empty()) {
    IDLOGDEBUG("kafka_pub", threadName << ": Data(" << msgId << "): ACK: " << toLog);
  } else if (!data.empty()) {
    IDLOGDEBUG("kafka_pub", threadName << ": Data(" << msgId << "): ACK: " << data);
  } else {
    IDLOGDEBUG("kafka_pub", threadName << ": Data(" << msgId << "): ACK: empty message.");
  }
}

void PublishDataCommand::onError(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName, const std::string & data,
                             const std::string & errorMessage) {
  if (!toLog.empty()) {
    IDLOGERROR("kafka_pub", threadName << ": Data(" << msgId << "): FAIL: " << toLog << "\n" << errorMessage);
  } else if (!data.empty()) {
    IDLOGERROR("kafka_pub", threadName << ": Data(" << msgId << "): FAIL: " << data << "\n" << errorMessage);
  } else {
    IDLOGDEBUG("kafka_pub", threadName << ": Data(" << msgId << "): FAIL: empty message.");
  }
}
