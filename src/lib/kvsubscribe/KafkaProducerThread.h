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

#ifndef SRC_LIB_KVSUBSCRIBE_KAFKAPRODUCERTHREAD_H_
#define SRC_LIB_KVSUBSCRIBE_KAFKAPRODUCERTHREAD_H_

#include <string>
#include <thread>
#include "kvsubscribe/ProducerCommand.h"

namespace kvalobs {
namespace service {

/**
 * KafkaProducerThread is a helper class to run a kafka produser in it own
 * thread. The communication with the thread is trough the message queue
 * 'queue'.
 *
 * Remember that kvalobs topics is on the form kvalobs.domain.queue.
 * Where:
 *   domain is: kvalobs, test, etc.
 *      The domain 'kvalobs' is reserved for the production machines.
 *   queue is: raw, decoded and checked
 *       - raw - for raw data, mostly used by kvDataInputd
 *       - decoded - for decoded data used by kvDataInputd
 *       - checked - Used by the QC application to send out checked data.
 *
 *  We can use the helper functions in kvsubscribe/queue.h to get the topics.
 *  there is three methods that is of interest.
 *
 *    - kvalobs::subscribe::queue::raw(const std::string &domain)
 *    - kvalobs::subscribe::queue::decoded(const std::string &domain)
 *    - kvalobs::subscribe::queue::checked(const std::string &domain)
 *
 *  For the QC applications the function checked(const std::string &domain) is used
 *  to get the topic to checked data.
 *
 *  Kafka hosts is a comma separated list of kafka hosts on the form
 *  "host1:port, host2:port, ..., hostN:port", where port is optional.
 *  If port is not given the default port is used.
 *
 ~~~~{.cc}

 Ex. To start a thread to send data to the checked queue (topic) for the test domain. We have
 a kafka cluster with 4 nodes.

 using kvalobs::subscribe::queue::checked;
 KafkaProducerThread checkedProducer;
 string kafkaBrokers("kafka1.met.no,kafka2.met.no,kafka3.met.no,kafka4.met.no");
 checkedProducer.start(kafkaBrokers, checked("test"));

 // We can now send data with.
 std::list<kvalobs::kvData> dataList=produceData();
 KvDataSerializeCommand *dataCmd = new  KvDataSerializeCommand(dataList);
 checkedProducer.queue.add( dataCmd );

 // or use the convenience method  checkedProducer.send( dataCmd )

 ~~~~
 */

class KafkaProducerThread {
  typedef std::shared_ptr<miutil::concurrent::BlockingQueuePtr<std::string>> StatusQue;
  StatusQue statusQue;
  std::thread kafkaThread;
  std::string name;

 public:
  ProducerQuePtr queue;
  explicit KafkaProducerThread(const std::string &name = "", unsigned int queueSize = 50);
  ~KafkaProducerThread();

  /**
   * Must be called before start is called.
   */
  void setName(const std::string &name_);
  std::string getName() const {
    return name;
  }

  void send(ProducerCommand *cmd);

  /**
   * @throws std::runtime_error if the kafka thread can't start.
   */
  void start(const std::string &brokers, const std::string &topic);
  void shutdown();
  void join(const std::chrono::high_resolution_clock::duration &timeout);
};
}  //  namespace service
}  //  namespace kvalobs

#endif  // SRC_LIB_KVSUBSCRIBE_KAFKAPRODUCERTHREAD_H_
