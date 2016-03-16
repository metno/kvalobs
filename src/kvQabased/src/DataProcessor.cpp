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

#include "DataProcessor.h"
#include "CheckRunner.h"
#include "QaBaseApp.h"
#include <kvsubscribe/KafkaProducer.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <decodeutility/kvalobsdata.h>
#include <milog/milog.h>
#include <set>
#include <string>
#include <thread>

namespace qabase {
namespace {
// Must be protected by mutex if we want to make this thread-safe
std::set<kvalobs::subscribe::KafkaProducer::MessageId> messages;
}

DataProcessor::DataProcessor(std::shared_ptr<qabase::CheckRunner> checkRunner)
    : checkRunner_(checkRunner),
      logCreator_(QaBaseApp::baseLogDir()),
      output_(QaBaseApp::kafkaProducer()) {
}

DataProcessor::~DataProcessor() {
}

qabase::CheckRunner::KvalobsDataPtr DataProcessor::runChecks(const kvalobs::kvStationInfo & si) const {
  qabase::LogFileCreator::LogStreamPtr logStream = logCreator_.getLogStream(si);
  qabase::CheckRunner::KvalobsDataPtr modified(checkRunner_->newObservation(si, logStream.get()));
  return modified;
}

void DataProcessor::sendToKafka(const qabase::CheckRunner::KvalobsDataPtr & dataList) {
  int sendAttempts = 0;
  while (true) {
    auto messageid = output_->send(kvalobs::serialize::KvalobsDataSerializer::serialize(*dataList));
    messages.insert(messageid);
    finalizeMessage_();

    if (messages.find(messageid) == messages.end()) {
      if (sendAttempts > 0)
        LOGWARN("Successfully sent data after " << sendAttempts << " retries");
      break;
    } else {
      if (sendAttempts == 0)
        LOGWARN("Could not send data to Kafka. Send queue size=" << messages.size() << ". Retrying...");
      sendAttempts++;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

void DataProcessor::process(const kvalobs::kvStationInfo & si) {
  qabase::CheckRunner::KvalobsDataPtr d = runChecks(si);
  sendToKafka(d);
  finalizeMessage_();
}

void DataProcessor::process(const std::string & message) {
  kvalobs::serialize::KvalobsDataSerializer s(message);
  const kvalobs::serialize::KvalobsData & data = s.toData();
  auto modified = data.summary();

  process(modified.begin(), modified.end());
}

void DataProcessor::onKafkaSendSuccess(kvalobs::subscribe::KafkaProducer::MessageId id, const std::string & data) {
  if (messages.erase(id) == 0) {  // should never happen
    LOGWARN("Got confirmation for invalid message id! Data: <" + data + ">");
  } else {
    LOGDEBUG("Successfully sent data: <" + data + ">");
  }
}

void DataProcessor::onKafkaSendError(kvalobs::subscribe::KafkaProducer::MessageId id, const std::string & data, const std::string & errorMessage) {
  LOGDEBUG("Unable to send data: " + errorMessage + "  Data: <" + data + ">");
}

void DataProcessor::finalizeMessage_() {
  output_->catchup(2000);
}

} /* namespace qabase */
