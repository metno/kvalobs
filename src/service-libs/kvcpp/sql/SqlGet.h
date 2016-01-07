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

#ifndef SRC_SERVICE_LIBS_KVCPP_SQL_SQLGET_H_
#define SRC_SERVICE_LIBS_KVCPP_SQL_SQLGET_H_

#include "../KvApp.h"
#include <memory>
#include <functional>

namespace dnmi {
namespace db {
class Connection;
}
}

namespace kvservice {
namespace sql {

class SqlGet : virtual public details::KvalobsGet {
 public:
  explicit SqlGet(std::function<dnmi::db::Connection *()> createConnection);
  virtual ~SqlGet();

  virtual bool getKvData( KvGetDataReceiver &dataReceiver, const WhichDataHelper &wd );
  virtual bool getKvRejectDecode( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, kvservice::RejectDecodeIterator &it );
  virtual bool getKvParams( std::list<kvalobs::kvParam> &paramList );
  virtual bool getKvStations( std::list<kvalobs::kvStation> &stationList );
  virtual bool getKvModelData( std::list<kvalobs::kvModelData> &dataList, const WhichDataHelper &wd );
  virtual bool getKvTypes( std::list<kvalobs::kvTypes> &typeList );
  virtual bool getKvOperator( std::list<kvalobs::kvOperator> &operatorList );
  virtual bool getKvStationParam( std::list<kvalobs::kvStationParam> &stParam, int stationid, int paramid = -1, int day = -1 );
  virtual bool getKvStationMetaData( std::list<kvalobs::kvStationMetadata> &stMeta,
                                 int stationid, const boost::posix_time::ptime &obstime,
                                 const std::string & metadataName = "");
  virtual bool getKvObsPgm( std::list<kvalobs::kvObsPgm> &obsPgm, const std::list<long> &stationList, bool aUnion );
  virtual bool getKvData( KvObsDataList &dataList, const WhichDataHelper &wd );
  virtual bool getKvWorkstatistik(CKvalObs::CService::WorkstatistikTimeType timeType,
                                  const boost::posix_time::ptime &from, const boost::posix_time::ptime &to,
                                  kvservice::WorkstatistikIterator &it
                                  );

private:
  dnmi::db::Connection * connection(int id = 0);

  std::function<dnmi::db::Connection *()> createConnection_;

  std::map<int, std::shared_ptr<dnmi::db::Connection>> connections_;
};

} /* namespace sql */
} /* namespace kvservice */

#endif /* SRC_SERVICE_LIBS_KVCPP_SQL_SQLGET_H_ */
