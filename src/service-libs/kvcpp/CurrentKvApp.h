/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2016 met.no

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

#ifndef SRC_SERVICE_LIBS_KVCPP_CURRENT_CURRENTKVAPP_H_
#define SRC_SERVICE_LIBS_KVCPP_CURRENT_CURRENTKVAPP_H_

#include <memory>
#include "KvApp.h"
#include "sql/SqlGet.h"
#include "kafka/KafkaSubscribe.h"


namespace kvalobs {
namespace datasource {
class SendData;
}  // namespace datasource
}  // namespace kvalobs

namespace kvservice {

class CurrentKvApp : public KvApp, virtual sql::SqlGet, virtual kafka::KafkaSubscribe {
 public:
  CurrentKvApp(int argc, char ** argv, const std::string & preferredConfigFile = std::string());
  virtual ~CurrentKvApp();

  const CKvalObs::CDataSource::Result_var sendDataToKv(const char *data, const char *obsType) override;

 private:
  std::unique_ptr<kvalobs::datasource::SendData> sendData_;
};


}  // namespace kvservice

#endif /* SRC_SERVICE_LIBS_KVCPP_CURRENT_CURRENTKVAPP_H_ */
