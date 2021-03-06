/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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

#ifndef SRC_LIB_KVSUBSCRIBE_DATAPRODUCER_H_
#define SRC_LIB_KVSUBSCRIBE_DATAPRODUCER_H_

#include "KafkaProducer.h"
#include <string>

namespace kvalobs {
namespace serialize {
class KvalobsData;
}

namespace subscribe {

class DataProducer {
 public:
  typedef KafkaProducer::MessageId MessageId;

  DataProducer(const std::string & domain, const std::string & brokers =
                   "localhost",
               KafkaProducer::ErrorHandler onFailedDelivery =
                   [](MessageId, const std::string &, const std::string &) {},
               KafkaProducer::SuccessHandler onSuccessfulDelivery =
                   [](MessageId, const std::string &) {});
  ~DataProducer();

  MessageId send(const serialize::KvalobsData & data);

  /**
   * Process all awaiting delivery reports.
   *
   * @param timeout Maximum time to wait for delivery report to become available, in milliseconds
   */
  void catchup(unsigned timeout = 0) {
    producer_.catchup(timeout);
  }

 private:
  KafkaProducer producer_;
};

} /* namespace kafka */
} /* namespace kvservice */

#endif /* SRC_LIB_KVSUBSCRIBE_DATAPRODUCER_H_ */
