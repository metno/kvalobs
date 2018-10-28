/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: decoder.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $

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

#ifndef SRC_LIB_DECODER_DECODERBASE_DATAUPDATETRANSACTION_H_
#define SRC_LIB_DECODER_DECODERBASE_DATAUPDATETRANSACTION_H_

#include <list>
#include <string>
#include <sstream>
#include "boost/shared_ptr.hpp"
#include "decodeutility/kvalobsdata.h"
#include "kvdb/transaction.h"
#include "kvalobs/kvData.h"
#include "kvalobs/kvStationInfo.h"
#include "kvalobs/kvTextData.h"
#include "kvalobs/observation.h"

namespace kvalobs {
namespace decoder {

/**
 * \brief A transaction class to help in inserting new data.
 *
 * Old data for the observation is deleted if the observation
 * allready exist and is not equal to the new data. ie a duplicate
 * message.
 *
 */
class DataUpdateTransaction : public dnmi::db::Transaction {
  DataUpdateTransaction operator=(const DataUpdateTransaction &rhs);
  std::list<kvalobs::kvData> *newData;
  std::list<kvalobs::kvTextData> *newTextData;
  boost::posix_time::ptime obstime;
  int stationid;
  int typeid_;
  boost::shared_ptr<kvalobs::serialize::KvalobsData> data_;  // Data that is inserted or updated
  boost::shared_ptr<bool> ok_;
  std::ostringstream log;
  std::string logid;
  std::string insertType;
  int nRetry;
  bool onlyAddOrUpdateData;

 public:
  int getPriority(dnmi::db::Connection *conection, int stationid, int typeid_, const boost::posix_time::ptime &obstime);
  void updateWorkQue(dnmi::db::Connection *conection, long observationid, int priority);
  bool updateObservation(dnmi::db::Connection *conection, kvalobs::Observation *obs);
  bool replaceObservation(dnmi::db::Connection *conection, long observationid);
  void setTbtime(dnmi::db::Connection *conection);
  bool addDataToList(const kvalobs::kvData &data,
                     std::list<kvalobs::kvData> &dataList, bool replaceOnly =
                         false);
  bool isEqual(const std::list<kvalobs::kvData> &oldData,
               const std::list<kvalobs::kvTextData> &oldTextData);

  DataUpdateTransaction(const boost::posix_time::ptime &obstime, int stationid,
                        int typeid_,
                        std::list<kvalobs::kvData> *newData,
                        std::list<kvalobs::kvTextData> *newTextData,
                        const std::string &logid, bool onlyAddOrUpdateData =
                            false);
  DataUpdateTransaction(const DataUpdateTransaction &dut);

  virtual ~DataUpdateTransaction();
  virtual bool operator()(dnmi::db::Connection *conection);
  virtual void onAbort(const std::string &driverid,
                       const std::string &errorMessage,
                       const std::string &errorCode);
  virtual void onSuccess();
  virtual void onRetry();
  virtual void onMaxRetry(const std::string &lastError);

  
  kvalobs::serialize::KvalobsData insertedOrUpdatedData() const {
    return *data_;
  }

  bool ok() const {
    return *ok_;
  }
};

}  // namespace decoder
}  // namespace kvalobs
#endif  // SRC_LIB_DECODER_DECODERBASE_DATAUPDATETRANSACTION_H_
