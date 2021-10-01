/*
 * KafkaKvApp.cpp
 *
 *  Created on: Nov 12, 2015
 *      Author: vegardb
 */

#include "KafkaKvApp.h"
#include <kvsubscribe/DataSubscriber.h>
#include <milog/milog.h>

namespace kvservice {
namespace kafka {
namespace {
std::string value(miutil::conf::ConfSection * conf, const std::string & key,
                  const std::string & fallback) {
  auto value = conf->getValue(key);

  if (value.empty())
    return fallback;
  if (value.size() > 1)
    return fallback;

  return value.front().valAsString();
}

std::string getDomain(miutil::conf::ConfSection * conf) {
  return value(conf, "kafka.domain", "test");
}

std::string getBrokers(miutil::conf::ConfSection * conf) {
  return value(conf, "kafka.brokers", "localhost");
}

}

KafkaKvApp::KafkaKvApp(int &argc, char **argv, miutil::conf::ConfSection *conf,
                       const char *options[][2])
    : corba::CorbaKvApp(argc, argv, conf, options),
      subscriptionHandler_(getDomain(conf), getBrokers(conf)) {
}

KafkaKvApp::~KafkaKvApp() {
}


KafkaKvApp::SubscriberID KafkaKvApp::subscribeDataNotify(
    const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue) {
  return subscriptionHandler_.subscribeDataNotify(info, queue);
}

KafkaKvApp::SubscriberID KafkaKvApp::subscribeData(
    const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue & queue) {
  auto groupId=getConsumerGroupId();
  LOGDEBUG("KafkaKvApp::subscribeData: groupId: '" << groupId << "'.");
  return subscriptionHandler_.subscribeDataWithGroupId(info, queue, groupId);
}

KafkaKvApp::SubscriberID KafkaKvApp::subscribeDataWithGroupId(
    const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue & queue, const std::string &groupId) {
  return subscriptionHandler_.subscribeDataWithGroupId(info, queue, groupId);
}


//KafkaKvApp::SubscriberID KafkaKvApp::subscribeKvHint( dnmi::thread::CommandQue &queue )
//{
//	return CorbaKvApp::subscribeKvHint(queue);
//}

void KafkaKvApp::unsubscribe(const KafkaKvApp::SubscriberID &subscriberid) {
  if (subscriptionHandler_.knowsAbout(subscriberid))
    subscriptionHandler_.unsubscribe(subscriberid);
  else
    CorbaKvApp::unsubscribe(subscriberid);
}

void KafkaKvApp::unsubscribeAll() {
  subscriptionHandler_.unsubscribeAll();
  CorbaKvApp::unsubscribeAll();
}

} /* namespace kafka */
} /* namespace kvservice */
