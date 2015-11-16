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

namespace kvservice
{
namespace kafka
{

class KafkaKvApp: public corba::CorbaKvApp
{
public:
	KafkaKvApp(int &argc, char **argv,  miutil::conf::ConfSection *conf, const char *options[][2] = nullptr);
	virtual ~KafkaKvApp();

    virtual SubscriberID subscribeDataNotify( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue );
    virtual SubscriberID subscribeData( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue );
//    virtual SubscriberID subscribeKvHint( dnmi::thread::CommandQue &queue );
    virtual void unsubscribe( const SubscriberID &subscriberid );
    virtual void unsubscribeAll();

private:
    KafkaSubscribe subscriptionHandler_;
};

} /* namespace kafka */
} /* namespace kvservice */

#endif /* SRC_SERVICE_LIBS_KVCPP_KAFKA_KAFKAKVAPP_H_ */
