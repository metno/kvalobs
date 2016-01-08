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

#ifndef SRC_KVQABASED_SRC_QABASEAPP_H_
#define SRC_KVQABASED_SRC_QABASEAPP_H_

#include <kvalobs/kvbaseapp.h>
#include <memory>
#include <string>

namespace kvalobs {
namespace subscribe {
class KafkaProducer;
}
}

namespace qabase {

class QaBaseApp : public KvBaseApp {
 public:
  QaBaseApp(int argc, char ** argv);
  virtual ~QaBaseApp();

  static const std::string & kafkaDomain() {
    return kafkaDomain_;
  }
  static const std::string & kafkaBrokers() {
    return kafkaBrokers_;
  }
  static std::shared_ptr<kvalobs::subscribe::KafkaProducer> kafkaProducer();

  static std::string baseLogDir();

 private:
  static std::string kafkaBrokers_;
  static std::string kafkaDomain_;

};

} /* namespace qabase */

#endif /* SRC_KVQABASED_SRC_QABASEAPP_H_ */
