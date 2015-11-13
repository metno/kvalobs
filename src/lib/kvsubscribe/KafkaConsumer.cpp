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


#include "KafkaConsumer.h"
#include <stdexcept>


namespace kvalobs
{
namespace subscribe
{


std::list<KafkaConsumer *> KafkaConsumer::allConsumers_;

KafkaConsumer::KafkaConsumer(ConsumptionStart startAt,
        const std::string & topic,
        const std::string & brokers) :
        stopping_(false)
{
    createConnection_(brokers);
    createTopic_(topic, startAt);
    allConsumers_.push_back(this);
}

KafkaConsumer::~KafkaConsumer()
{
    stop();
    allConsumers_.remove(this);
}

namespace
{
class FunctionConsumer: public RdKafka::ConsumeCb
{
public:
    FunctionConsumer(
            std::function<void(RdKafka::Message & message)> handler) :
            handler_(handler)
    {
    }
    virtual void consume_cb(RdKafka::Message & message, void * /*ignored*/)
    {
        handler_(message);
    }
private:
    std::function<void(RdKafka::Message & message)> handler_;
};
}

void KafkaConsumer::run()
{
    stopping_ = false;

    while (not stopping_)
        runOnce(1000);
}

void KafkaConsumer::runOnce(unsigned timeoutInMilliSeconds)
{
    FunctionConsumer consumer([this](RdKafka::Message & message) {
        handle_(message);
    });

    consumer_->consume_callback(topic_.get(), 0, timeoutInMilliSeconds, &consumer, nullptr);
    consumer_->poll(0);
}

void KafkaConsumer::stop()
{
    stopping_ = true;
}

void KafkaConsumer::stopAll()
{
    for (auto consumer : allConsumers_)
        consumer->stop();
}

void KafkaConsumer::handle_(RdKafka::Message & message)
{
    switch (message.err()) {
    case RdKafka::ERR_NO_ERROR:
        try
        {
            data((char*) message.payload(), message.len());
        }
        catch (std::exception & e)
        {
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


void KafkaConsumer::createConnection_(const std::string& brokers)
{
    std::string errstr;
    std::unique_ptr<RdKafka::Conf> conf(
            RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    conf->set("metadata.broker.list", brokers, errstr);
    consumer_.reset(RdKafka::Consumer::create(conf.get(), errstr));
    if (!consumer_)
        throw std::runtime_error("Failed to create consumer: " + errstr);
}

void KafkaConsumer::createTopic_(const std::string& topic, ConsumptionStart startAt)
{
    std::string errstr;

    std::unique_ptr<RdKafka::Conf> tconf(
            RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));
    topic_.reset(
            RdKafka::Topic::create(consumer_.get(), topic, tconf.get(),
                    errstr));
    if (!topic_)
        throw std::runtime_error("Failed to create topic: " + errstr);

    int64_t offset = RdKafka::Topic::OFFSET_BEGINNING;
    switch (startAt)
    {
    case Earliest:
        offset = RdKafka::Topic::OFFSET_BEGINNING;
        break;
    case Stored:
        offset = RdKafka::Topic::OFFSET_STORED;
        break;
    case Latest:
        offset = RdKafka::Topic::OFFSET_END;
        break;
    default:
        throw std::logic_error("Invalid offset spec");
    }
    RdKafka::ErrorCode resp = consumer_->start(topic_.get(), 0, offset);
    if (resp != RdKafka::ERR_NO_ERROR)
        throw std::runtime_error(
                "Failed to start consumer: " + RdKafka::err2str(resp));
}


}
}
