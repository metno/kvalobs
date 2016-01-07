/*
 * CurrentKvApp.h
 *
 *  Created on: Dec 31, 2015
 *      Author: vegardb
 */

#ifndef SRC_SERVICE_LIBS_KVCPP_CURRENT_CURRENTKVAPP_H_
#define SRC_SERVICE_LIBS_KVCPP_CURRENT_CURRENTKVAPP_H_

#include "KvApp.h"
#include "sql/SqlGet.h"
#include "kafka/KafkaSubscribe.h"


namespace kvservice {

class FakeSend : virtual public details::KvalobsSend {
 public:
  virtual const CKvalObs::CDataSource::Result_var sendDataToKv(
      const char *data, const char *obsType) {
    CKvalObs::CDataSource::Result_var r( new CKvalObs::CDataSource::Result );
    r->res = CKvalObs::CDataSource::OK;
    r->message = "Not implemented, but pretending it is ok";
    return r;
  }
};

class CurrentKvApp : public KvApp, virtual FakeSend, virtual sql::SqlGet, virtual kafka::KafkaSubscribe {
 public:
  CurrentKvApp(int argc, char ** argv, const std::string & preferredConfigFile = std::string());
  virtual ~CurrentKvApp();
};


} /* namespace kvservice */

#endif /* SRC_SERVICE_LIBS_KVCPP_CURRENT_CURRENTKVAPP_H_ */
