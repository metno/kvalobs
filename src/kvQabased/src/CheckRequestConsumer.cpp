/*
 * CheckRequestConsumer.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: vegardb
 */

#include "CheckRequestConsumer.h"
#include "CheckRunner.h"
#include "Configuration.h"
#include <kvsubscribe/queue.h>
#include <milog/milog.h>

namespace qabase
{
namespace
{
kvalobs::subscribe::KafkaConsumer::ConsumptionStart startTime = kvalobs::subscribe::KafkaConsumer::Stored;
}

CheckRequestConsumer::CheckRequestConsumer(const qabase::Configuration & config) :
		kvalobs::subscribe::KafkaConsumer(startTime,
			kvalobs::subscribe::queue::decoded(config.kafkaDomain(), true),
			config.kafkaBrokers()),
		processor_(CheckRunner::create(config.databaseConnectString()),	config)

{

}

CheckRequestConsumer::~CheckRequestConsumer()
{
}

void CheckRequestConsumer::data(const char * msg, unsigned length)
{
	processor_.process(std::string(msg, length));
}

void CheckRequestConsumer::error(int code, const std::string & msg)
{
	LOGERROR(msg);
}


} /* namespace qabase */
