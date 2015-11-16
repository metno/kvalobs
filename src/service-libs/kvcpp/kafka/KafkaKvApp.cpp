/*
 * KafkaKvApp.cpp
 *
 *  Created on: Nov 12, 2015
 *      Author: vegardb
 */

#include "KafkaKvApp.h"
#include <kvsubscribe/DataSubscriber.h>


namespace kvservice
{
namespace kafka
{
namespace
{
std::string getBrokers(miutil::conf::ConfSection * conf)
{
	static const std::string DEFAULT = "localhost";

	auto value = conf->getValue("kafka.brokers");

	if ( value.empty() )
		return DEFAULT;
	if ( value.size() > 1 )
		return DEFAULT;

	return value.front().valAsString();
}
}

KafkaKvApp::KafkaKvApp(int &argc, char **argv,  miutil::conf::ConfSection *conf, const char *options[][2]) :
		corba::CorbaKvApp(argc, argv, conf, options),
		subscriptionHandler_(getBrokers(conf))
{
}

KafkaKvApp::~KafkaKvApp()
{
}

KafkaKvApp::SubscriberID KafkaKvApp::subscribeDataNotify( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue )
{
	return subscriptionHandler_.subscribeDataNotify(info, queue);
}

KafkaKvApp::SubscriberID KafkaKvApp::subscribeData( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue & queue )
{
	return subscriptionHandler_.subscribeData(info, queue);
}

//KafkaKvApp::SubscriberID KafkaKvApp::subscribeKvHint( dnmi::thread::CommandQue &queue )
//{
//	return CorbaKvApp::subscribeKvHint(queue);
//}

void KafkaKvApp::unsubscribe( const KafkaKvApp::SubscriberID &subscriberid )
{
	if ( subscriptionHandler_.knowsAbout(subscriberid) )
		subscriptionHandler_.unsubscribe(subscriberid);
	else
		CorbaKvApp::unsubscribe(subscriberid);
}

void KafkaKvApp::unsubscribeAll()
{
	subscriptionHandler_.unsubscribeAll();
	CorbaKvApp::unsubscribeAll();
}


} /* namespace kafka */
} /* namespace kvservice */
