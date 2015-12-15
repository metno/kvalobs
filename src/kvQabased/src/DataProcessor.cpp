/*
 * DataProcessor.cpp
 *
 *  Created on: Dec 14, 2015
 *      Author: vegardb
 */

#include "DataProcessor.h"
#include "CheckRunner.h"
#include "Configuration.h"
#include <kvsubscribe/KafkaProducer.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <decodeutility/kvalobsdata.h>
#include <milog/milog.h>

namespace qabase
{

DataProcessor::DataProcessor(std::shared_ptr<qabase::CheckRunner> checkRunner, const qabase::Configuration & config) :
		checkRunner_(checkRunner),
		logCreator_(config.baseLogDir()),
		output_(config.kafkaProducer())
{}

//DataProcessor::DataProcessor(
//		std::shared_ptr<qabase::CheckRunner> checkRunner,
//		const std::string & baseLogDir,
//		std::shared_ptr<kvalobs::subscribe::KafkaProducer> output) :
//						checkRunner_(checkRunner),
//						logCreator_(baseLogDir),
//						output_(output)
//{
//}

DataProcessor::~DataProcessor()
{
}


void DataProcessor::process(const kvalobs::kvStationInfo & si)
{
	simpleProcess_(si);
	finalizeMessage_();
}

void DataProcessor::process(const std::string & message)
{
	kvalobs::serialize::KvalobsDataSerializer s(message);
	const kvalobs::serialize::KvalobsData & data = s.toData();
	auto modified = data.summary();

	process(modified.begin(), modified.end());
}


void DataProcessor::simpleProcess_(const kvalobs::kvStationInfo & si)
{
	qabase::LogFileCreator::LogStreamPtr logStream = logCreator_.getLogStream(si);
	qabase::CheckRunner::DataListPtr modified = checkRunner_->newObservation(si, logStream.get());
	if ( not modified->empty() )
	{
		kvalobs::serialize::KvalobsData d(* modified);
		output_->send(kvalobs::serialize::KvalobsDataSerializer::serialize(d));
	}
}

void DataProcessor::finalizeMessage_()
{
	output_->catchup(1000);
}


} /* namespace qabase */
