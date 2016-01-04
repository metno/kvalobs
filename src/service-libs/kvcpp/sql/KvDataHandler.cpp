/*
 * KvDataHandler.cpp
 *
 *  Created on: Nov 20, 2015
 *      Author: vegardb
 */

#include "KvDataHandler.h"
#include <sstream>
#include <memory>
#include <limits>
#include <future>

namespace kvservice {
namespace sql {
namespace internal {

KvDataHandler::KvDataHandler(dnmi::db::Connection & connection1,
                             dnmi::db::Connection & connection2,
                             KvGetDataReceiver & dataReceiver)
    : connection1_(connection1),
      connection2_(connection2),
      dataReceiver_(dataReceiver) {
}

KvDataHandler::~KvDataHandler() {
}

namespace {
template<class C>
class QueryResult {
 public:
  typedef C Data;

  QueryResult(std::shared_ptr<dnmi::db::Result> result)
      : result_(result) {
  }

  bool hasNext() {
    return result_->hasNext() or not data_.empty();
  }

  std::list<Data> next() {
    while (result_->hasNext()) {
      Data current(result_->next());
      if (data_.empty() or current.stationID() == data_.front().stationID())
        data_.push_back(current);
      else {
        std::list<Data> ret(1, current);
        std::swap(ret, data_);
        return ret;
      }
    }
    std::list<Data> ret;
    std::swap(ret, data_);
    return ret;
  }

 private:
  std::list<Data> data_;
  std::shared_ptr<dnmi::db::Result> result_;
};
}

void KvDataHandler::operator()(const WhichDataHelper & wd) {
  std::future<QueryResult<kvalobs::kvData>> dataFetch =
      std::async(
          std::launch::async,
          [this, &wd]() {
            std::shared_ptr<dnmi::db::Result> result(connection1_.execQuery(query_(wd, "data")));
            if ( ! result )
            throw std::runtime_error("Unable to perform query");
            return QueryResult<kvalobs::kvData>(result);
          });

  std::future<QueryResult<kvalobs::kvTextData>> textDataFetch =
      std::async(
          std::launch::deferred,
          [this, &wd]() {
            std::shared_ptr<dnmi::db::Result> textResult(connection2_.execQuery(query_(wd, "text_data")));
            if ( ! textResult )
            throw std::runtime_error("Unable to perform text_data query");
            return QueryResult<kvalobs::kvTextData>(textResult);
          });

  QueryResult<kvalobs::kvTextData> textDataResult = textDataFetch.get();
  QueryResult<kvalobs::kvData> dataResult = dataFetch.get();

  std::list<kvalobs::kvData> d = dataResult.next();
  std::list<kvalobs::kvTextData> td = textDataResult.next();
  while (true) {
    if (d.empty() and td.empty())
      break;

    int dataStationId =
        d.empty() ? std::numeric_limits<int>::max() : d.front().stationID();
    int textDataStationId =
        td.empty() ? std::numeric_limits<int>::max() : td.front().stationID();
    int stationId = std::min(dataStationId, textDataStationId);

    KvObsData obsData = KvObsData(stationId);

    if (dataStationId == stationId) {
      obsData.dataList() = d;
      d = dataResult.next();
    }
    if (textDataStationId == stationId) {
      obsData.textDataList() = td;
      td = textDataResult.next();
    }

    KvObsDataList dataList = { obsData };
    if (not dataReceiver_.next(dataList))
      break;
  }
}

std::string KvDataHandler::query_(const WhichDataHelper & wd,
                                  const std::string & table) {
  const CKvalObs::CService::WhichDataList * whichDataList = wd.whichData();

  std::ostringstream query;
  query << "select * from " << table;

  if (whichDataList->length() > 0) {
    query << " where ";
    for (int i = 0; i < whichDataList->length(); ++i) {
      if (i != 0)
        query << "or ";
      auto whichData = (*whichDataList)[i];
      query << "(stationid=" << whichData.stationid << " ";
      query << "and obstime>='" << whichData.fromObsTime << "' ";
      if (whichData.toObsTime[0] != '\0')
        query << "and obstime<='" << whichData.toObsTime << "' ";
      query << ")";
    }
  }
  query << " order by stationid, obstime, typeid";

  return query.str();
}

}
}
}
