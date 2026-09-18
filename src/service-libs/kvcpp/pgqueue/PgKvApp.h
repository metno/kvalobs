/*
 * KafkaKvApp.h
 *
 *  Created on: Nov 12, 2015
 *      Author: vegardb
 */

#ifndef SRC_SERVICE_LIBS_KVCPP_PG_PGKVAPP_H_
#define SRC_SERVICE_LIBS_KVCPP_PG_PGKVAPP_H_

#include "corba/CorbaKvApp.h"
#include "PgSubscribe.h"

namespace kvservice {
namespace pg {

class PgKvApp : public corba::CorbaKvApp {
public:
  PgKvApp(int &argc, char **argv, miutil::conf::ConfSection *conf,
          const char *options[][2] = nullptr);
  virtual ~PgKvApp();

  /**
   * getGroupIdFromConf, returns the group id from the groupIdKey in the
   * configuration file. If the groupIdKey is an empty string it search the for
   * the following keys in order.
   *
   *   - kafka.groupid.appname
   *   - kafka.gropuid
   *
   *  appname is what was given when the AppClass was created. Can also set it
   * with. KvApp::appName="ny app name"
   *
   * Returns the groupId or an empty string if no groupId is given for the key.
   */
  // virtual std::string getConsumerGroupIdFromConf(const std::string
  // &groupIdKey="");

  virtual SubscriberID
  subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
                      dnmi::thread::CommandQue &queue);
  virtual SubscriberID subscribeData(const KvDataSubscribeInfoHelper &info,
                                     dnmi::thread::CommandQue &queue);
  virtual SubscriberID
  subscribeDataWithGroupId(const KvDataSubscribeInfoHelper &info,
                           dnmi::thread::CommandQue &queue,
                           const std::string &groupId);
  //
  //    virtual SubscriberID subscribeKvHint( dnmi::thread::CommandQue &queue );
  virtual void unsubscribe(const SubscriberID &subscriberid);
  virtual void unsubscribeAll();

private:
  PgSubscribe subscriptionHandler_;
  
};

} /* namespace pg */
} /* namespace kvservice */

#endif /* SRC_SERVICE_LIBS_KVCPP_PG_PGKVAPP_H_ */
