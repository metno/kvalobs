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

#ifndef KAFKAPRODUCER_H_
#define KAFKAPRODUCER_H_

#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <ostream>

namespace RdKafka {
class Producer;
class Topic;
class Message;
class DeliveryReportCb;
}

namespace kvalobs {
namespace subscribe {

class KafkaConfig;

class KafkaProducer {
 public:
  typedef uint64_t MessageId;
  typedef std::function<void(MessageId id, const std::string & data)> SuccessHandler;
  typedef std::function<void(MessageId id, const std::string & data, const std::string & errorMessage)> ErrorHandler;

  explicit KafkaProducer(const std::string & topic,
                         const std::string & brokers = "localhost",
                         ErrorHandler onFailedDelivery = [](MessageId, const std::string &, const std::string &) {},
                         SuccessHandler onSuccessfulDelivery = [](MessageId, const std::string &) {});

  explicit KafkaProducer(const KafkaConfig &config,
                         ErrorHandler onFailedDelivery = [](MessageId, const std::string &, const std::string &) {},
                         SuccessHandler onSuccessfulDelivery = [](MessageId, const std::string &) {});
                       
  void init( const KafkaConfig &config,
             ErrorHandler onFailedDelivery, SuccessHandler onSuccessfulDelivery);

  ~KafkaProducer();

  /**
   * Asynchronous sending of data. Remember to call catchup() at some point
   * to check results of send.
   *
   * On error, may either throw an exception right away, or deliver an
   * error report on KafkaProducer's deliveryReportHandler after having
   * called catchup, or destroying this object.
   *
   * @throws exception if it fails right away
   *
   * @return a message id, that will be available in this object's constructor's
   *         onFailedDelivery and onSuccessfulDelivery functions
   */
  MessageId send(const std::string & data);

  MessageId send(const char * data, unsigned length);

  /**
   * Process all awaiting delivery reports.
   *
   * @param timeout Maximum time to wait for delivery report to become available, in milliseconds
   */
  void catchup(unsigned timeout = 0);

  std::string topic() const;

 private:
  MessageId messageId_;
  
  std::unique_ptr<RdKafka::Producer> producer_;
  std::unique_ptr<RdKafka::Topic> topic_;

  std::unique_ptr<RdKafka::DeliveryReportCb> deliveryReportHandler_;
};

}
}

#endif /* KAFKAPRODUCER_H_ */
