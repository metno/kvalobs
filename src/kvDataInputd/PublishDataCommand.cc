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

#include "lib/milog/milog.h"
#include "kvDataInputd/PublishDataCommand.h"

using std::string;
using kvalobs::subscribe::KafkaProducer;

PublishDataCommand::PublishDataCommand(const std::string &rawData)
    : data(rawData) {
}

const char *PublishDataCommand::getData(unsigned int *size) const {
  *size = data.size();
  return data.data();
}

void PublishDataCommand::onSend(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName) {
  string::size_type i = data.find_first_of('\n');
  if (i != string::npos) {
    IDLOGDEBUG("kafka_pub", "Data(" << msgId << "): SENDT: " << data.substr(i));
  } else if (!data.empty()) {
    IDLOGDEBUG("kafka_pub", "Data(" << msgId << "): SENDT: " << data);
  } else {
    IDLOGDEBUG("kafka_pub", "Data(" << msgId << "): SENDT: empty message.");
  }
}

void PublishDataCommand::onSuccess(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName, const std::string &data) {
  string::size_type i = data.find_first_of('\n');
  if (i != string::npos) {
    IDLOGDEBUG("kafka_pub", threadName << ": Data(" << msgId << "): ACK: " << data.substr(i));
  } else if (!data.empty()) {
    IDLOGDEBUG("kafka_pub", threadName << ": Data(" << msgId << "): ACK: " << data);
  } else {
    IDLOGDEBUG("kafka_pub", threadName << ": Data(" << msgId << "): ACK: empty message.");
  }
}

void PublishDataCommand::onError(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName, const std::string & data,
                             const std::string & errorMessage) {
  string::size_type i = data.find_first_of('\n');

  if (i != string::npos) {
    IDLOGERROR("kafka_pub", threadName << ": Data(" << msgId << "): FAIL: " << data.substr(i) << "\n" << errorMessage);
  } else if (!data.empty()) {
    IDLOGERROR("kafka_pub", threadName << ": Data(" << msgId << "): FAIL: " << data << "\n" << errorMessage);
  } else {
    IDLOGDEBUG("kafka_pub", threadName << ": Data(" << msgId << "): FAIL: empty message.");
  }
}
