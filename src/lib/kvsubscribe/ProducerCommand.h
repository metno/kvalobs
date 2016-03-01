/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2007 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 email: kvalobs-dev@met.no

 This file is part of KVALOBS

 KVALOBS is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 KVALOBS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with KVALOBS; if not, write to the Free Software Foundation Inc.,
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef SRC_LIB_KVSUBSCRIBE_PRODUCERCOMMAND_H_
#define SRC_LIB_KVSUBSCRIBE_PRODUCERCOMMAND_H_

#include <string>
#include "kvsubscribe/KafkaProducer.h"
#include "miutil/blockingqueue.h"

namespace kvalobs {
namespace service {


/**
 * \brief This is the message that is passed to the
 * kafka producers.
 */

class ProducerCommand {
  ProducerCommand(const ProducerCommand &);
  ProducerCommand& operator=(const ProducerCommand &);

 public:
  ProducerCommand();
  virtual ~ProducerCommand();

  virtual const char *getData(unsigned int *size) const = 0;

  /**
   * If the MessageId is 0, then there was no data to send.
   */
  virtual void onSend(kvalobs::subscribe::KafkaProducer::MessageId msgId,  const std::string &threadName);

  /**
   * If the MessageId is 0, then there was no data to send. We treat this as a success.
   */
  virtual void onSuccess(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName, const std::string &data);
  virtual void onError(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName, const std::string & data, const std::string & errorMessage);
};

typedef miutil::concurrent::BlockingQueuePtr<ProducerCommand> ProducerQue;
typedef std::shared_ptr<ProducerQue> ProducerQuePtr;

}  //  namespace service
}  //  namespace kvalobs

#endif  // SRC_LIB_KVSUBSCRIBE_PRODUCERCOMMAND_H_
