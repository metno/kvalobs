#include "../DataReinserter.h"
#include "../kvDataFormatter.h"
#include "../kvalobsdataserializer.h"
#include <kvalobs/kvDataFlag.h>
#include <stdexcept>
#include <fstream>
#include <sstream>

// using namespace kvalobs;
// using namespace CKvalObs::CDataSource;

namespace kvalobs {

template<class kvApp>
const char *DataReinserter<kvApp>::decoderID = "kv2kvDecoder";

template<class kvApp>
DataReinserter<kvApp>::DataReinserter(kvApp *app, int operatorID)
    : app(app),
      operatorID(operatorID)
//  , exceptionOnLogError( true )
{
}

template<class kvApp>
DataReinserter<kvApp>::~DataReinserter() {
}

template<class kvApp>
const CKvalObs::CDataSource::Result_var DataReinserter<kvApp>::insert(
    kvData & d) const {
  kvUseInfo i = d.useinfo();
  i.HQCid(this->operatorID);
  d.useinfo(i);

  std::string s = decodeutility::kvdataformatter::createString(d);
  const char * send = s.c_str();

  return app->sendDataToKv(send, decoderID);
}

template<class kvApp>
const CKvalObs::CDataSource::Result_var DataReinserter<kvApp>::insert(
    std::list<kvData> &dl) const {
  for (std::list<kvData>::iterator it = dl.begin(); it != dl.end(); it++) {
    kvUseInfo i = it->useinfo();
    i.HQCid(this->operatorID);
    it->useinfo(i);
  }

  std::string s = decodeutility::kvdataformatter::createString(dl);
  const char * send = s.c_str();

  const CKvalObs::CDataSource::Result_var ret = app->sendDataToKv(send,
                                                                  decoderID);

  return ret;
}

template<class kvApp> const CKvalObs::CDataSource::Result_var DataReinserter<
    kvApp>::insert(const kvalobs::serialize::KvalobsData & data) const {
  // We make a copy and modify that instead of the sent in data:
  kvalobs::serialize::KvalobsData dcopy(data);

  std::list < kvData > dlist;
  dcopy.getData(dlist);

  for (std::list<kvData>::iterator it = dlist.begin(); it != dlist.end();
      ++it) {
    kvUseInfo i = it->useinfo();
    i.HQCid(this->operatorID);
    it->useinfo(i);
    dcopy.insert(*it);
  }

  kvalobs::serialize::KvalobsDataSerializer s(dcopy);
  std::string xml = s.toString();

  const CKvalObs::CDataSource::Result_var ret = app->sendDataToKv(xml.c_str(),
                                                                  decoderID);

  return ret;
}

}
