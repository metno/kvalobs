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

#include "KafkaProducer.h"
#include <iostream>
#include <librdkafka/rdkafkacpp.h>

namespace kvalobs {
namespace subscribe {

namespace {
class DeliveryReport : public RdKafka::DeliveryReportCb {
 public:
  DeliveryReport(KafkaProducer::ErrorHandler onFailedDelivery,
                 KafkaProducer::SuccessHandler onSuccessfulDelivery)
      : onFailedDelivery_(onFailedDelivery),
        onSuccessfulDelivery_(onSuccessfulDelivery) {
  }

  virtual void dr_cb(RdKafka::Message & message) {
    std::string data((char*) message.payload(), message.len());
    if (message.err() == RdKafka::ERR_NO_ERROR)
      onSuccessfulDelivery_(data);
    else
      onFailedDelivery_(data, message.errstr());
  }

 private:
  KafkaProducer::ErrorHandler onFailedDelivery_;
  KafkaProducer::SuccessHandler onSuccessfulDelivery_;
};
}

KafkaProducer::KafkaProducer(const std::string & topic,
                             const std::string & brokers,
                             KafkaProducer::ErrorHandler onFailedDelivery,
                             KafkaProducer::SuccessHandler onSuccessfulDelivery)
    : deliveryReportHandler_(
        new DeliveryReport(onFailedDelivery, onSuccessfulDelivery)) {
  if (brokers.empty())
    throw std::logic_error("Empty kafka broker list");

  std::string errstr;

  std::unique_ptr<RdKafka::Conf> conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  conf->set("metadata.broker.list", brokers, errstr);
  conf->set("dr_cb", deliveryReportHandler_.get(), errstr);

  producer_.reset(RdKafka::Producer::create(conf.get(), errstr));
  if (!producer_)
    throw std::runtime_error("Failed to create producer: " + errstr);

  std::unique_ptr<RdKafka::Conf> tconf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  topic_.reset(
      RdKafka::Topic::create(producer_.get(), topic, tconf.get(), errstr));
  if (!topic_)
    throw std::runtime_error("Failed to create topic: " + errstr);
}

KafkaProducer::~KafkaProducer() {
  catchup();
}

void KafkaProducer::send(const std::string & data) {
  send(data.c_str(), data.size());
}

void KafkaProducer::send(const char * data, unsigned length) {
  RdKafka::ErrorCode resp = producer_->produce(
      topic_.get(), RdKafka::Topic::PARTITION_UA,
      RdKafka::Producer::RK_MSG_COPY /* Copy payload */, (void*) data, length,
      nullptr, nullptr);

  if (resp != RdKafka::ERR_NO_ERROR)
    throw std::runtime_error(RdKafka::err2str(resp));
}

void KafkaProducer::catchup(unsigned timeout) {
  producer_->poll(timeout);
}

std::string KafkaProducer::topic() const {
  return topic_->name();
}

}
}
