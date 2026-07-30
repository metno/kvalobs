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

#ifndef KAFKACONSUMER_H_
#define KAFKACONSUMER_H_

#include "Consumer.h"
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace RdKafka {
class KafkaConsumer;
class Topic;
class Conf;
class Message;
} // namespace RdKafka

namespace kvalobs {
namespace subscribe {

/**
 * Base class for subscribing to data from kvalobs.
 *
 * Subclasses wil be fed data from the specified stream, and the abstract
 * methods data(...) and error(...) will be called as appropriate by this
 * class' event loop.
 *
 * TODO:
 *   - Ensure that stored works
 *     - we will need a name for consumer
 *     - automatically set some config options
 *   - figure out partitions
 */
class KafkaConsumer : public Consumer {
public:
  KafkaConsumer(const std::string &topic, const std::string &brokers,
                const std::string &groupId = "");

  virtual ~KafkaConsumer();

  /**
   * Stop this consumer.
   */
  void stop() override;

protected:
  void runOnce(unsigned timeoutInMilliSeconds) override;

private:
  typedef std::function<void(RdKafka::Message &message)> BasicHandler;
  void handle_(RdKafka::Message &message);
  void createConnection_(const std::string &brokers,
                         const std::string &groupId);
  void subscribe_();

  bool initialized_;
  std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
  std::vector<std::string> topics_;
  std::string groupId_;
};

} // namespace subscribe
} // namespace kvalobs

#endif /* KAFKACONSUMER_H_ */
