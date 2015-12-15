/*
 * CheckRequestConsumer.h
 *
 *  Created on: Dec 11, 2015
 *      Author: vegardb
 */

#ifndef SRC_KVQABASED_SRC_CHECKREQUESTCONSUMER_H_
#define SRC_KVQABASED_SRC_CHECKREQUESTCONSUMER_H_

#include <kvsubscribe/KafkaConsumer.h>
#include "DataProcessor.h"
#include <memory>

namespace qabase
{
class Configuration;


// TODO Refactor into consumer and processor (last one is essentially checkrunner + post to kafka)
class CheckRequestConsumer: public kvalobs::subscribe::KafkaConsumer
{
public:
	explicit CheckRequestConsumer(const qabase::Configuration & config);
	virtual ~CheckRequestConsumer();

protected:
    virtual void data(const char * msg, unsigned length);
    virtual void error(int code, const std::string & msg);

private:
    DataProcessor processor_;
};

} /* namespace qabase */

#endif /* SRC_KVQABASED_SRC_CHECKREQUESTCONSUMER_H_ */
