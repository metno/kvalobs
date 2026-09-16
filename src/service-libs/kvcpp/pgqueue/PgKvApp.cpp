/*
 * PgKvApp.cpp
 *
 *  Created on: Nov 12, 2015
 *      Author: vegardb
 */

#include "PgKvApp.h"
#include <kvsubscribe/DataSubscriber.h>
#include <milog/milog.h>

namespace kvservice {
namespace pg {
namespace {
std::string value(miutil::conf::ConfSection *conf, const std::string &key,
                  const std::string &fallback) {
  auto value = conf->getValue(key);

  if (value.empty())
    return fallback;
  if (value.size() > 1)
    return fallback;

  return value.front().valAsString();
}

std::string getDomain(miutil::conf::ConfSection *conf) {
  return value(conf, "pgqueue.domain", "development");
}

std::vector<std::string> getConnectStr(miutil::conf::ConfSection *conf) {
  std::vector<std::string> result;
  if (auto val = value(conf, "pgqueue.database_a", ""); !val.empty())
    result.push_back(val);
  if (auto val = value(conf, "pgqueue.database_b", ""); !val.empty())
    result.push_back(val);
  return result;
}

int getConsumerPollSize(miutil::conf::ConfSection *conf) {
  std::vector<std::string> result;
  if (auto val = value(conf, "consumer_poll_size", "100"); !val.empty())
    return std::stoi(val);
  return 100; // default poll size
}

} // namespace

PgKvApp::PgKvApp(int &argc, char **argv, miutil::conf::ConfSection *conf,
                 const char *options[][2])
    : corba::CorbaKvApp(argc, argv, conf, options),
      subscriptionHandler_(getDomain(conf), getConnectStr(conf), getConsumerPollSize(conf)) {}

PgKvApp::~PgKvApp() {}

PgKvApp::SubscriberID
PgKvApp::subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
                             dnmi::thread::CommandQue &queue) {
  return subscriptionHandler_.subscribeDataNotify(info, queue);
}

PgKvApp::SubscriberID
PgKvApp::subscribeData(const KvDataSubscribeInfoHelper &info,
                       dnmi::thread::CommandQue &queue) {
  auto groupId = getConsumerGroupId();
  LOGINFO("PgKvApp::subscribeData: groupId: '" << groupId << "'.");
  return subscriptionHandler_.subscribeDataWithGroupId(info, queue, groupId);
}

PgKvApp::SubscriberID
PgKvApp::subscribeDataWithGroupId(const KvDataSubscribeInfoHelper &info,
                                  dnmi::thread::CommandQue &queue,
                                  const std::string &groupId) {
  return subscriptionHandler_.subscribeDataWithGroupId(info, queue, groupId);
}

// KafkaKvApp::SubscriberID KafkaKvApp::subscribeKvHint(
// dnmi::thread::CommandQue &queue )
//{
//	return CorbaKvApp::subscribeKvHint(queue);
// }

void PgKvApp::unsubscribe(const PgKvApp::SubscriberID &subscriberid) {
  if (subscriptionHandler_.knowsAbout(subscriberid))
    subscriptionHandler_.unsubscribe(subscriberid);
  else
    corba::CorbaKvApp::unsubscribe(subscriberid);
}

void PgKvApp::unsubscribeAll() {
  subscriptionHandler_.unsubscribeAll();
  corba::CorbaKvApp::unsubscribeAll();
}

} // namespace pg
} /* namespace kvservice */
