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

#ifndef SRC_KVDATAINPUTD_KAFKAPRODUCERTHREAD_H_
#define SRC_KVDATAINPUTD_KAFKAPRODUCERTHREAD_H_

#include <string>
#include <thread>
#include "kvDataInputd/ProducerCommand.h"

class KafkaProducerThread {
  typedef std::shared_ptr<miutil::concurrent::BlockingQueuePtr<std::string>> StatusQue;
  StatusQue statusQue;
  std::thread kafkaThread;

 public:
  ProducerQuePtr que;
  KafkaProducerThread();
  ~KafkaProducerThread();

  /**
   * @throws std::runtime_error if the kafka thread can't start.
   */
  void start(const std::string &brokers, const std::string &topic);
  void shutdown();
  void join(const std::chrono::high_resolution_clock::duration &timeout);
};

#endif  // SRC_KVDATAINPUTD_KAFKAPRODUCERTHREAD_H_
