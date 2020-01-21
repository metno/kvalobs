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

#include <stdexcept>
#include <librdkafka/rdkafkacpp.h>
#include "KafkaConsumer.h"

namespace kvalobs {
namespace subscribe {

std::list<KafkaConsumer *> KafkaConsumer::allConsumers_;

KafkaConsumer::KafkaConsumer(const std::string & topic,
                             const std::string & brokers)
    : initialized_(false),
      stopping_(false),
      topicConf_(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC)),
      topicName_(topic),
      topicOffset_(RdKafka::Topic::OFFSET_END) {
  createConnection_(brokers);
  allConsumers_.push_back(this);
}

KafkaConsumer::~KafkaConsumer() {
  stop();
  allConsumers_.remove(this);
}

void KafkaConsumer::startAtEarliestData() {
  topicOffset_ = RdKafka::Topic::OFFSET_BEGINNING;
}

namespace {
void set(RdKafka::Conf & c, const std::string & key, const std::string & value) {
  std::string errstr;
  if ( c.set(key, value, errstr) != RdKafka::Conf::CONF_OK)
    throw std::runtime_error(errstr);
}
}

void KafkaConsumer::startAtStored(const std::string & fileName) {
  topicOffset_ = RdKafka::Topic::OFFSET_STORED;
  set(* topicConf_, "offset.store.method", "file");
  set(* topicConf_, "offset.store.path", fileName);
  set(* topicConf_, "offset.store.sync.interval.ms", "0");
  set(* topicConf_, "auto.commit.interval.ms", "1000");
}


namespace {
class FunctionConsumer : public RdKafka::ConsumeCb {
 public:
  FunctionConsumer(std::function<void(RdKafka::Message & message)> handler)
      : handler_(handler) {
  }
  virtual void consume_cb(RdKafka::Message & message, void * /*ignored*/) {
    handler_(message);
  }
 private:
  std::function<void(RdKafka::Message & message)> handler_;
};
}

void KafkaConsumer::run() {
  stopping_ = false;

  while (not stopping_)
    runOnce(1000);
}

void KafkaConsumer::runOnce(unsigned timeoutInMilliSeconds) {
  if (!initialized_) {
    createTopic_();
    initialized_ = true;
  }

  FunctionConsumer consumer([this](RdKafka::Message & message) {
    handle_(message);
  });

  consumer_->consume_callback(topic_.get(), 0, timeoutInMilliSeconds, &consumer,
                              nullptr);
  consumer_->poll(0);
}

void KafkaConsumer::stop() {
  stopping_ = true;
}

void KafkaConsumer::stopAll() {
  for (auto consumer : allConsumers_)
    consumer->stop();
}

void KafkaConsumer::handle_(RdKafka::Message & message) {
  switch (message.err()) {
    case RdKafka::ERR_NO_ERROR:
      try {
        data((char*) message.payload(), message.len());
      } catch (std::exception & e) {
        error(0, e.what());
      }
      break;
    case RdKafka::ERR__PARTITION_EOF:
      // ignored
      break;
    default:
      error(message.err(), message.errstr());
      break;
  }
}

void KafkaConsumer::createConnection_(const std::string & brokers) {
  std::string errstr;
  std::unique_ptr<RdKafka::Conf> conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  set(* conf, "metadata.broker.list", brokers);
  consumer_.reset(RdKafka::Consumer::create(conf.get(), errstr));
  if (!consumer_)
    throw std::runtime_error("Failed to create consumer: " + errstr);
}

void KafkaConsumer::createTopic_() {
  std::string errstr;
  topic_.reset(
      RdKafka::Topic::create(consumer_.get(), topicName_, topicConf_.get(), errstr));
  if (!topic_)
    throw std::runtime_error("Failed to create topic: " + errstr);

  RdKafka::ErrorCode resp = consumer_->start(topic_.get(), 0, topicOffset_);
  if (resp != RdKafka::ERR_NO_ERROR)
    throw std::runtime_error(
        "Failed to start consumer: " + RdKafka::err2str(resp));
}

}
}
