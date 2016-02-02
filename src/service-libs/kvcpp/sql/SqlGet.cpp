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
#include "SqlGet.h"
#include "KvDataHandler.h"
#include <kvdb/kvdb.h>
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvPath.h>
#include <miutil/timeconvert.h>
#include <milog/milog.h>
#include <miutil/timeconvert.h>
#include <sstream>
#include <iomanip>

namespace kvservice {
namespace sql {
namespace {
bool query(std::shared_ptr<dnmi::db::Connection> connection, std::string q,
           std::function<void(const dnmi::db::DRow &)> handler) {
  if (q.empty())
    return false;

  if (q.back() != ';')
    q = q + ';';

  try {
    if (not connection->isConnected())
      if (!connection->tryReconnect())
        throw std::runtime_error("Database connection unavailable!");

    LOGDEBUG(q);

    std::unique_ptr<dnmi::db::Result> result(connection->execQuery(q));
    while (result->hasNext())
      handler(result->next());
    return true;
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}
}

SqlGet::SqlGet(std::function<dnmi::db::Connection *()> createConnection,
               std::function<void(dnmi::db::Connection *)> releaseConnection)
    : connections_(createConnection, releaseConnection) {
}

SqlGet::~SqlGet() {
}

bool SqlGet::getKvData(KvGetDataReceiver &dataReceiver,
                       const WhichDataHelper &wd) {
  try {
    auto c1 = connections_.get();
    auto c2 = connections_.get();
    internal::KvDataHandler handler(*c1, *c2,
                                    dataReceiver);
    handler(wd);
    return true;
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvRejectDecode(
    const CKvalObs::CService::RejectDecodeInfo &decodeInfo,
    kvservice::RejectDecodeIterator &it) {
  std::ostringstream q;
  q << "select * from rejectdecode where tbtime between '"
    << decodeInfo.fromTime << "' and '" << decodeInfo.toTime << "'";
  if (decodeInfo.decodeList.length() > 0) {
    q << " and (";
    q << "decoder='" << decodeInfo.decodeList[0] << "'";
    for (int i = 1; i < decodeInfo.decodeList.length(); ++i)
      q << " or decoder='" << decodeInfo.decodeList[i] << "'";
    q << ")";
  }
  q << " order by tbtime";

  try {
    std::vector<kvalobs::kvRejectdecode> rejected;
    bool ok = query(connections_.get(), q.str(),
                    [& rejected](const dnmi::db::DRow & row) {
                      kvalobs::kvRejectdecode r(row);
                      rejected.push_back(r);
                    });
    it = kvservice::RejectDecodeIterator(rejected);
    return ok;
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvParams(std::list<kvalobs::kvParam> &paramList) {
  try {
    return query(connections_.get(), "select * from param",
                 [&paramList](const dnmi::db::DRow & row) {
                   paramList.push_back(kvalobs::kvParam(row));
                 });
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvStations(std::list<kvalobs::kvStation> &stationList) {
  try {
    return query(connections_.get(), "select * "
                 "from station",
                 [&stationList](const dnmi::db::DRow & row) {
                   stationList.push_back(kvalobs::kvStation(row));
                 });
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvModelData(std::list<kvalobs::kvModelData> &dataList,
                            const WhichDataHelper &wd) {
  std::ostringstream q;
  q << "select * from model_data";
  auto * whichData = wd.whichData();
  if (whichData and whichData->length() > 0) {
    q << " where ";
    for (int i = 0; i < whichData->length(); ++i) {
      if (i != 0)
        q << " or ";
      auto dataElement = (*whichData)[i];
      q << "(stationid=" << dataElement.stationid << " and ";
      if (strcmp(dataElement.fromObsTime, dataElement.toObsTime) == 0)
        q << "obstime='" << dataElement.fromObsTime << "')";
      else
        q << "obstime between '" << dataElement.fromObsTime << "' and '"
          << dataElement.toObsTime << "')";
    }
  }

  try {
    return query(connections_.get(), q.str(),
                 [&dataList](const dnmi::db::DRow & row) {
                   dataList.push_back(kvalobs::kvModelData(row));
                 });
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvTypes(std::list<kvalobs::kvTypes> &typeList) {
  try {
    return query(connections_.get(), "select * "
                 "from types "
                 "order by typeid",
                 [&typeList](const dnmi::db::DRow & row) {
                   typeList.push_back(kvalobs::kvTypes(row));
                 });
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvOperator(std::list<kvalobs::kvOperator> &operatorList) {
  try {
    return query(connections_.get(), "select * from operator",
                 [&operatorList](const dnmi::db::DRow & row) {
                   operatorList.push_back(row);
                 });
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvStationParam(std::list<kvalobs::kvStationParam> &stParam,
                               int stationid, int paramid, int day) {
  try {
    std::ostringstream q;
    q << "select * " << "from station_param " << "where stationid="
      << stationid;
    if (paramid >= 0)
      q << " and paramid=" << paramid;
    if (day >= 0)
      q << " and fromday<=" << day << " and today>=" << day;
    q << " order by paramid, fromday";
    return query(connections_.get(), q.str(), [&stParam](const dnmi::db::DRow & row) {
      stParam.push_back(kvalobs::kvStationParam(row));
    });
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvStationMetaData(std::list<kvalobs::kvStationMetadata> &stMeta,
                                  int stationid,
                                  const boost::posix_time::ptime &obstime,
                                  const std::string & metadataName) {
  try {
    std::ostringstream q;
    q << "select * from station_metadata where ";
    if (stationid != 0)
      q << " stationid=" << stationid << " and ";
    q << " fromtime<='" << to_kvalobs_string(obstime) << "' and ";
    q << " (totime is NULL or totime <'" << to_kvalobs_string(obstime) << "')";
    if (not metadataName.empty())
      q << " and metadatatypename='" << metadataName << "'";

    query(connections_.get(), q.str(), [&stMeta](const dnmi::db::DRow & row) {

      dnmi::db::DRow & r = const_cast<dnmi::db::DRow &>(row);
      kvalobs::kvStationMetadata d(r);
      stMeta.push_back(d);
    });

    return true;
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

bool SqlGet::getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
                         const std::list<long> &stationList, bool aUnion) {
  std::ostringstream q;
  q << "select ";
  if (aUnion) {
    q << "stationid, -1 as paramid, 0 as level, max(nr_sensor) as nr_sensor, ";
    q << "min(typeid) as typeid, 'false'::bool as priority_message, ";
    q << "'false'::bool as collector, ";
    for (int i = 0; i < 24; ++i)
      q << "bool_or(kl" << std::setw(2) << std::setfill('0') << i << ") as kl"
        << std::setw(2) << std::setfill('0') << i << ", ";
    for (auto day : { "mon", "tue", "wed", "thu", "fri", "sat", "sun" })
      q << "bool_or(" << day << ") as " << day << ", ";
    q << "min(fromtime) as fromtime, max(totime) as totime";
  } else
    q << "*";
  q << " from obs_pgm";
  if (not stationList.empty()) {
    auto it = stationList.begin();
    q << " where stationid in (" << *it;
    while (it != stationList.end())
      q << ", " << *(it++);
    q << ")";
  }
  if (aUnion)
    q << " group by stationid";
  q << " order by stationid";

  return query(connections_.get(), q.str(), [&obsPgm](const dnmi::db::DRow & row) {
    obsPgm.push_back(kvalobs::kvObsPgm(row));
  });
}

bool SqlGet::getKvData(KvObsDataList &dataList, const WhichDataHelper &wd) {
  class Appender : public KvGetDataReceiver {
   public:
    Appender(KvObsDataList & dataList)
        : dataList_(dataList) {
    }
    virtual bool next(KvObsDataList & d) {
      dataList_.insert(dataList_.end(), d.begin(), d.end());
      return true;
    }
   private:
    KvObsDataList & dataList_;
  } appender(dataList);

  return getKvData(appender, wd);
}

namespace {
std::string workstatistikRowName(
    CKvalObs::CService::WorkstatistikTimeType timeType) {
  switch (timeType) {
    case CKvalObs::CService::ObsTime:
      return "obstime";
    case CKvalObs::CService::TbTime:
      return "tbtime";
    case CKvalObs::CService::ProcessStartTime:
      return "process_start";
    case CKvalObs::CService::QaStartTime:
      return "qa_start";
    case CKvalObs::CService::QaStopTime:
      return "qa_stop";
    case CKvalObs::CService::ServiceStartTime:
      return "service_start";
    case CKvalObs::CService::ServiceStopTime:
      return "service_stop";
    default:
      throw std::logic_error("Invalid WorkstatistikTimeType");
  }
}
}

bool SqlGet::getKvWorkstatistik(
    CKvalObs::CService::WorkstatistikTimeType timeType,
    const boost::posix_time::ptime &from, const boost::posix_time::ptime &to,
    kvservice::WorkstatistikIterator &it) {
  std::ostringstream q;
  q << "select * from workstatistik where " << workstatistikRowName(timeType);
  if (from == to)
    q << "='" << to_kvalobs_string(from) << "' ";
  else
    q << " between '" << to_kvalobs_string(from) << "' and '"
      << to_kvalobs_string(to) << "' ";
  q << "order by tbtime";

  try {
    std::vector<kvalobs::kvWorkelement> ret;
    bool ok = query(connections_.get(), q.str(), [&ret](const dnmi::db::DRow & row) {
      ret.push_back(kvalobs::kvWorkelement(row));
    });
    if (ok)
      it = kvservice::WorkstatistikIterator(ret);
    return ok;
  } catch (std::exception & e) {
    LOGERROR(e.what());
    return false;
  }
}

} /* namespace sql */
} /* namespace kvservice */
