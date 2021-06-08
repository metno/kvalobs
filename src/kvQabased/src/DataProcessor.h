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

#ifndef SRC_KVQABASED_SRC_DATAPROCESSOR_H_
#define SRC_KVQABASED_SRC_DATAPROCESSOR_H_

#include "LogFileCreator.h"
#include "kvsubscribe/KafkaProducer.h"
#include <memory>
#include <string>
#include "CheckRunner.h"

namespace kvalobs {
class kvStationInfo;
namespace subscribe {
class KafkaProducer;
}
}

namespace qabase {
class Observation;
class CheckRunner;
class Configuration;

/**
 * Frontend to CheckRunner. Calls Checkrunner, and sends modifications notifications to listeners.
 */
class DataProcessor {
 public:

  static bool logXml;
  static bool logTransactions;

  explicit DataProcessor(std::shared_ptr<qabase::CheckRunner> checkRunner);

  ~DataProcessor();

  /**
   * Run checks on a single message, but do not post to kafka
   */
  qabase::CheckRunner::KvalobsDataPtr runChecks(const qabase::Observation & obs) const;

  /**
   * Send a single dataList to Kafka
   */
  void sendToKafka(const qabase::Observation & obs, const qabase::CheckRunner::KvalobsDataPtr dataList, bool * stop = nullptr);


  /**
   * Process a single message. Also post to kafka.
   */
  void process(const kvalobs::kvStationInfo & si);

  /**
   * Parse and process message. Also post to kafka.
   */
  void process(const std::string & message);

  /**
   * Process the given set of messages. Also post to kafka. Errors when
   * sending modification notifications may be given either after each
   * message, or after all messages have been processed.
   */
  template<typename StationInfoIterator>
  void process(StationInfoIterator begin, StationInfoIterator end);

  static void onKafkaSendSuccess(kvalobs::subscribe::KafkaProducer::MessageId id, const std::string & data);
  static void onKafkaSendError(kvalobs::subscribe::KafkaProducer::MessageId id, const std::string & data, const std::string & errorMessage);

 private:
  void finalizeMessage_();

  std::shared_ptr<qabase::CheckRunner> checkRunner_;
  qabase::LogFileCreator logCreator_;
  std::shared_ptr<kvalobs::subscribe::KafkaProducer> output_;
};

// template<typename StationInfoIterator>
// void DataProcessor::process(StationInfoIterator begin, StationInfoIterator end) {
//   while (begin != end) {
//     sendToKafka(runChecks(*begin));
//     ++begin;
//   }
//   finalizeMessage_();
// }

} /* namespace qabase */

#endif /* SRC_KVQABASED_SRC_DATAPROCESSOR_H_ */
