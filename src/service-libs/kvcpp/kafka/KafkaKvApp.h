/*
 * KafkaKvApp.h
 *
 *  Created on: Nov 12, 2015
 *      Author: vegardb
 */

#ifndef SRC_SERVICE_LIBS_KVCPP_KAFKA_KAFKAKVAPP_H_
#define SRC_SERVICE_LIBS_KVCPP_KAFKA_KAFKAKVAPP_H_

#include "../corba/CorbaKvApp.h"
#include "KafkaSubscribe.h"

namespace kvservice {
namespace kafka {

class KafkaKvApp : public corba::CorbaKvApp {
 public:
  KafkaKvApp(int &argc, char **argv, miutil::conf::ConfSection *conf,
             const char *options[][2] = nullptr);
  virtual ~KafkaKvApp();

  /**
   * getGroupIdFromConf, returns the group id from the groupIdKey in the configuration file.
   * If the groupIdKey is an empty string it search the for the following keys in order.
   * 
   *   - kafka.groupid.appname
   *   - kafka.gropuid
   * 
   *  appname is what was given when the AppClass was created. Can also set it with.
   * KvApp::appName="ny app name"
   * 
   * Returns the groupId or an empty string if no groupId is given for the key.
   */
  //virtual std::string getConsumerGroupIdFromConf(const std::string &groupIdKey="");

  virtual SubscriberID subscribeDataNotify(
      const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue);
  virtual SubscriberID subscribeData(const KvDataSubscribeInfoHelper &info,
                                     dnmi::thread::CommandQue &queue);
  virtual SubscriberID subscribeDataWithGroupId(const KvDataSubscribeInfoHelper &info,
                                     dnmi::thread::CommandQue &queue, const std::string &groupId);
//
//    virtual SubscriberID subscribeKvHint( dnmi::thread::CommandQue &queue );
  virtual void unsubscribe(const SubscriberID &subscriberid);
  virtual void unsubscribeAll();

 private:
  KafkaSubscribe subscriptionHandler_;
};

} /* namespace kafka */
} /* namespace kvservice */

#endif /* SRC_SERVICE_LIBS_KVCPP_KAFKA_KAFKAKVAPP_H_ */
