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

#include <string>
#include <memory>
#include <librdkafka/rdkafkacpp.h>

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
class KafkaConsumer {
 public:

  /**
   * List of where to start reading data.
   */
  enum ConsumptionStart {
    Earliest,  //< Start at the earliest available data in queue - this may give you very old data
    Stored,  //< Pick up where you left. First time this is called, it will behave like Latest. Note that there may be some overlap in data - the first data you receive may be equal to the last data from last time
    Latest  //< Only get data that arrives after having started this consumer
  };

  KafkaConsumer(ConsumptionStart startAt, const std::string & topic,
                const std::string & brokers);

  virtual ~KafkaConsumer();

  /**
   * Run until stop() has been called, processing events, calling data(...)
   * and error(...) as appropriate.
   *
   * It may make sense to run this in a std::thread
   */
  void run();

  /**
   * Process one message, waiting maximum for the given time if no messages are available.
   */
  void runOnce(unsigned timeoutInMilliSeconds);

  /**
   * Has stop() been called?
   */
  bool stopping() const {
    return stopping_;
  }

  /**
   * Stop this consumer.
   */
  void stop();

  /**
   * call stop() an all consumers
   */
  static void stopAll();

 protected:

  /**
   * Process incoming data
   */
  virtual void data(const char * msg, unsigned length) =0;

  /**
   * Handle errors on message arrival.
   */
  virtual void error(int code, const std::string & msg) =0;

 private:
  typedef std::function<void(RdKafka::Message & message)> BasicHandler;
  void handle_(RdKafka::Message & message);
  void createConnection_(const std::string& brokers);
  void createTopic_(const std::string& topic, ConsumptionStart startAt);

  bool stopping_;
  std::unique_ptr<RdKafka::Consumer> consumer_;
  std::unique_ptr<RdKafka::Topic> topic_;

  static std::list<KafkaConsumer *> allConsumers_;

};

}
}

#endif /* KAFKACONSUMER_H_ */
