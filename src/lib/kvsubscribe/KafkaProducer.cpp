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
#include "KafkaConfig.h"
#include <iostream>
#include <librdkafka/rdkafkacpp.h>

namespace kvalobs {
namespace subscribe {

namespace {
class DeliveryReport : public RdKafka::DeliveryReportCb {
 public:
  DeliveryReport(KafkaProducer::ErrorHandler onFailedDelivery, KafkaProducer::SuccessHandler onSuccessfulDelivery)
      : onFailedDelivery_(onFailedDelivery),
        onSuccessfulDelivery_(onSuccessfulDelivery) {
  }

  virtual void dr_cb(RdKafka::Message & message) {
    std::unique_ptr<KafkaProducer::MessageId> id(static_cast<KafkaProducer::MessageId *>(message.msg_opaque()));

    std::string data((char*) message.payload(), message.len());
    if (message.err() == RdKafka::ERR_NO_ERROR) {
      onSuccessfulDelivery_(*id, data);
    } else { 
      std::cerr << "FAILED Kafka delivery " << *id << ": " << message.errstr() << ".\n";
      onFailedDelivery_(*id, data, message.errstr());
    }
  }

 private:
  KafkaProducer::ErrorHandler onFailedDelivery_;
  KafkaProducer::SuccessHandler onSuccessfulDelivery_;
};
}


/* 
KafkaProducer::KafkaProducer(const std::string & topic, const std::string & brokers, KafkaProducer::ErrorHandler onFailedDelivery,
                             KafkaProducer::SuccessHandler onSuccessfulDelivery)
    : deliveryReportHandler_(new DeliveryReport(onFailedDelivery, onSuccessfulDelivery)),
      messageId_(0) {
  if (brokers.empty())
    throw std::logic_error("Empty kafka broker list");

  std::string errstr;

  std::unique_ptr<RdKafka::Conf> conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  conf->set("metadata.broker.list", brokers, errstr);
  //conf->set("bootstrap.servers", brokers, errstr);
  conf->set("dr_cb", deliveryReportHandler_.get(), errstr);

  producer_.reset(RdKafka::Producer::create(conf.get(), errstr));
  if (!producer_)
    throw std::runtime_error("Failed to create producer: " + errstr);

  std::unique_ptr<RdKafka::Conf> tconf(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));


  topic_.reset(RdKafka::Topic::create(producer_.get(), topic, tconf.get(), errstr));
  if (!topic_)
    throw std::runtime_error("Failed to create topic: " + errstr);
}
 */

KafkaProducer::KafkaProducer(const std::string & topic,
                         const std::string & brokers,
                         ErrorHandler onFailedDelivery,
                         SuccessHandler onSuccessfulDelivery) 
{
  KafkaConfig conf;
  conf.brokers=brokers;
  conf.topic=topic;
  init(conf, onFailedDelivery, onSuccessfulDelivery);
} 

KafkaProducer::KafkaProducer(const KafkaConfig &config,
                         ErrorHandler onFailedDelivery,
                         SuccessHandler onSuccessfulDelivery)
{
  init(config, onFailedDelivery, onSuccessfulDelivery);
}
                       
  

void KafkaProducer::init( const KafkaConfig &config, 
  ErrorHandler onFailedDelivery, SuccessHandler onSuccessfulDelivery) 
{
  deliveryReportHandler_.reset(new DeliveryReport(onFailedDelivery, onSuccessfulDelivery));
  messageId_=0;
  if (config.brokers.empty())
    throw std::logic_error("Empty kafka broker list");

  std::string errstr;

  std::unique_ptr<RdKafka::Conf> conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  conf->set("metadata.broker.list", config.brokers, errstr);
  //conf->set("bootstrap.servers", config.brokers, errstr);
  conf->set("dr_cb", deliveryReportHandler_.get(), errstr);

  producer_.reset(RdKafka::Producer::create(conf.get(), errstr));
  if (!producer_)
    throw std::runtime_error("Failed to create producer: " + errstr);

  std::unique_ptr<RdKafka::Conf> tconf(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  if ( conf->set("request.required.acks", std::to_string(config.requestRequiredAcks), errstr)!=RdKafka::Conf::CONF_OK ) {
    throw std::runtime_error("Failed to configure topic (request.required.acks): " + errstr);
  }

  if ( conf->set("request.timeout.ms", std::to_string(config.requestTimeoutMs), errstr)!=RdKafka::Conf::CONF_OK ) {
    throw std::runtime_error("Failed to configure topic (request.timeout.ms): " + errstr);
  }

  topic_.reset(RdKafka::Topic::create(producer_.get(), config.topic, tconf.get(), errstr));
  if (!topic_)
    throw std::runtime_error("Failed to create topic: " + errstr);
}


KafkaProducer::~KafkaProducer() {
  catchup();
}

KafkaProducer::MessageId KafkaProducer::send(const std::string & data) {
  return send(data.c_str(), data.size());
}

KafkaProducer::MessageId KafkaProducer::send(const char * data, unsigned length) {
  MessageId * id = new MessageId(messageId_++);

  RdKafka::ErrorCode resp = producer_->produce(topic_.get(), RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                                               const_cast<char*>(data), length, nullptr, static_cast<void*>(id));

  if (resp != RdKafka::ERR_NO_ERROR) {
    delete id;
    throw std::runtime_error(RdKafka::err2str(resp));
  }

  return *id;
}

void KafkaProducer::catchup(unsigned timeout) {
  producer_->poll(timeout);
}

std::string KafkaProducer::topic() const {
  return topic_->name();
}

}
}
