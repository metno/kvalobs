/*
 * DataProcessor.h
 *
 *  Created on: Dec 14, 2015
 *      Author: vegardb
 */

#ifndef SRC_KVQABASED_SRC_DATAPROCESSOR_H_
#define SRC_KVQABASED_SRC_DATAPROCESSOR_H_

#include "LogFileCreator.h"
#include <memory>


namespace kvalobs
{
class kvStationInfo;
namespace subscribe
{
class KafkaProducer;
}
}

namespace qabase
{
class CheckRunner;
class Configuration;

/**
 * Frontend to CheckRunner. Calls Checkrunner, and sends modifications notifications to listeners.
 */
class DataProcessor
{
public:

	DataProcessor(
			std::shared_ptr<qabase::CheckRunner> checkRunner,
			const qabase::Configuration & config);

//	DataProcessor(
//			std::shared_ptr<qabase::CheckRunner> checkRunner,
//			const std::string & baseLogDir,
//			std::shared_ptr<kvalobs::subscribe::KafkaProducer> output);
	~DataProcessor();

	/**
	 * Process a single message.
	 */
	void process(const kvalobs::kvStationInfo & si);

	/**
	 * Parse and process message.
	 */
	void process(const std::string & message);

	/**
	 * Process the given set of messages. Errors when sending modification
	 * notifications may be given either after each message, or after all
	 * messages have been processed.
	 */
	template<typename StationInfoIterator>
	void process(StationInfoIterator begin, StationInfoIterator end);


private:
    void simpleProcess_(const kvalobs::kvStationInfo & si);
    void finalizeMessage_();

    std::shared_ptr<qabase::CheckRunner> checkRunner_;
	qabase::LogFileCreator logCreator_;
	std::shared_ptr<kvalobs::subscribe::KafkaProducer> output_;
};


template<typename StationInfoIterator>
void DataProcessor::process(StationInfoIterator begin, StationInfoIterator end)
{
	while ( begin != end )
	{
		simpleProcess_(*begin);
		++ begin;
	}
	finalizeMessage_();
}


} /* namespace qabase */

#endif /* SRC_KVQABASED_SRC_DATAPROCESSOR_H_ */
