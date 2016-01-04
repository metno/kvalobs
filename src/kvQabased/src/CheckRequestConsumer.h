/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2010 met.no

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

#ifndef SRC_KVQABASED_SRC_CHECKREQUESTCONSUMER_H_
#define SRC_KVQABASED_SRC_CHECKREQUESTCONSUMER_H_

#include <kvsubscribe/KafkaConsumer.h>
#include "DataProcessor.h"
#include <memory>
#include <string>

namespace qabase {

// TODO Refactor into consumer and processor (last one is essentially checkrunner + post to kafka)
class CheckRequestConsumer : public kvalobs::subscribe::KafkaConsumer {
 public:
  CheckRequestConsumer();
  virtual ~CheckRequestConsumer();

 protected:
  virtual void data(const char * msg, unsigned length);
  virtual void error(int code, const std::string & msg);

 private:
  DataProcessor processor_;
};

} /* namespace qabase */

#endif /* SRC_KVQABASED_SRC_CHECKREQUESTCONSUMER_H_ */
