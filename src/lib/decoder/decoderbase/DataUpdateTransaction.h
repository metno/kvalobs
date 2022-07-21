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
public:
  typedef enum { 
    Partial,  //! Use only the original value for the test
    Complete  //! Use all values original, corrected, controlinfo, useinfo and cfailed.
    } DuplicateTestType;
  
private:
  DataUpdateTransaction operator=(const DataUpdateTransaction &rhs);
  std::list<kvalobs::kvData> *newData;
  std::list<kvalobs::kvTextData> *newTextData;
  boost::posix_time::ptime obstime;
  int stationid;
  int typeid_;
  long long observationid;
  boost::posix_time::time_duration duration;
  boost::posix_time::ptime startTime;
  
  // Data that is inserted or updated. This is not used in the kv2018 update
  //but maybe we will reuse it later.
  boost::shared_ptr<kvalobs::serialize::KvalobsData> data_;  

  /**
   * dataToPublish - 
   *  Data that is inserted or updated and added to the database, but not to the workqueue
   *  this data will not be processed by kvQabase and not published by kvQabase.
   *  At the momment it is only HQC only data that bypass kvQabase
   */
  boost::shared_ptr<kvalobs::serialize::KvalobsData> dataToPublish_;  
  boost::shared_ptr<bool> ok_;
  mutable std::ostringstream log;
  std::string logid;
  std::string insertType;
  int nRetry;
  bool onlyAddOrUpdateData;
  bool addToWorkQueue;
  bool tryToUseDataTbTime;
  DuplicateTestType  duplicateTestType;
  bool onlyHqcData;
  int qaId;  //if > -1, assign to the kvQabased with qa_id == qaId.
  
  bool partialIsEqual(const std::list<kvalobs::kvData> &oldData_, const std::list<kvalobs::kvTextData> &oldTextData, bool replace)const;
  bool completeIsEqual(const std::list<kvalobs::kvData> &oldData, const std::list<kvalobs::kvTextData> &oldTextData, bool replace)const;
  void onlyHqcDataCheck();
  std::string transactionLogString();
  

 public:
  int getPriority(dnmi::db::Connection *conection, int stationid, int typeid_, const boost::posix_time::ptime &obstime);
  void updateWorkQue(dnmi::db::Connection *conection, long observationid, int priority);
  void checkWorkQue(dnmi::db::Connection *conection, long observationid);
  void worqueToWorkStatistik(dnmi::db::Connection *conection, long observationid);
  bool updateObservation(dnmi::db::Connection *conection, kvalobs::Observation *obs);
  bool replaceObservation(dnmi::db::Connection *conection, long observationid);
  void setTbtime(dnmi::db::Connection *conection);
  bool addDataToList(const kvalobs::kvData &data,
                     std::list<kvalobs::kvData> &dataList, bool replaceOnly =
                         false);
  boost::posix_time::ptime useTbTime(const std::list<kvalobs::kvData> &data, const std::list<kvalobs::kvTextData> &textData, const boost::posix_time::ptime &defaultTbTime)const;
  
  bool isEqual(const std::list<kvalobs::kvData> &oldData,
               const std::list<kvalobs::kvTextData> &oldTextData);

  // DataUpdateTransaction(const boost::posix_time::ptime &obstime, int stationid,
  //                       int typeid_,
  //                       std::list<kvalobs::kvData> *newData,
  //                       std::list<kvalobs::kvTextData> *newTextData,
  //                       const std::string &logid, 
  //                       bool onlyAddOrUpdateData = false, 
  //                       bool addToWorkQueue=true,
  //                       bool tryToUseDataTbTime = false,
  //                       DataUpdateTransaction::DuplicateTestType  duplicateTestType=Partial);

  //false, true, false, kvalobs::decoder::DataUpdateTransaction::Partial

  DataUpdateTransaction(const boost::posix_time::ptime &obstime, int stationid,
                        int typeid_,
                        std::list<kvalobs::kvData> *newData,
                        std::list<kvalobs::kvTextData> *newTextData,
                        const std::string &logid, 
                        bool onlyAddOrUpdateData, 
                        bool addToWorkQueue,
                        bool tryToUseDataTbTime,
                        DataUpdateTransaction::DuplicateTestType  duplicateTestType, int useQaId);          
  DataUpdateTransaction(const DataUpdateTransaction &dut);



  virtual ~DataUpdateTransaction();
  virtual bool operator()(dnmi::db::Connection *conection);
  virtual void onAbort(const std::string &driverid,
                       const std::string &errorMessage,
                       const std::string &errorCode);
  virtual void onSuccess();
  virtual void onFailure();
  virtual void onRetry();
  virtual void onMaxRetry(const std::string &lastError, const std::string &errorCode, bool mayRecover);

  
  kvalobs::serialize::KvalobsData insertedOrUpdatedData() const {
    return *data_;
  }

  kvalobs::serialize::KvalobsData dataToPublish() const {
    return *dataToPublish_;
  }

  bool ok() const {
    return *ok_;
  }
};

}  // namespace decoder
}  // namespace kvalobs
#endif  // SRC_LIB_DECODER_DECODERBASE_DATAUPDATETRANSACTION_H_
