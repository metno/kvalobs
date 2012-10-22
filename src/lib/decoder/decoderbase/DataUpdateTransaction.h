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

#ifndef __DATAUPDATETRANSACTION_H__
#define __DATAUPDATETRANSACTION_H__

#include <list>
#include <boost/shared_ptr.hpp>
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>
#include <kvdb/transaction.h>
#include <sstream>

namespace kvalobs{
namespace decoder{


/**
 * \brief A transaction class to help in inserting new data.
 *
 * Old data for the observation is deleted if the observation
 * allready exist and is not equal to the new data. ie a duplicate
 * message.
 *
 * A message is identified by stationid, typeid and tbtime. It is
 * importent that new data is inserted with a unique stationid,
 * typeid, tbtime for a message that comes with a nominal obstime.
 */

class DataUpdateTransaction : public dnmi::db::Transaction
{
   DataUpdateTransaction operator=( const DataUpdateTransaction &rhs);
   std::list<kvalobs::kvData> *newData;
   std::list<kvalobs::kvTextData> *newTextData;
   miutil::miTime obstime;
   int stationid;
   int typeid_;
   int priority;
   boost::shared_ptr<kvalobs::kvStationInfoList> stationInfoList_;
   boost::shared_ptr<bool> ok_;
   std::ostringstream log;
   std::string logid;
   std::string insertType;
   int nRetry;

public:
   void setTbtime( dnmi::db::Connection *conection );
   bool addDataToList( const kvalobs::kvData &data,
                       std::list<kvalobs::kvData> &dataList,
                       bool replaceOnly=false );

   bool addTextDataToList( const kvalobs::kvTextData &data,
                           std::list<kvalobs::kvTextData> &dataList,
                           bool replaceOnly=false );

   /**
    * Add a query to the list qList if it not all ready exist in the list.
    *
    * @param qList The list to add the query.
    * @param query The query to add.
    */
   void addQuery( std::list<std::string> &qList, const std::string &query );
   miutil::miTime getTimestamp( dnmi::db::Connection *conection, int &msec );
   void addStationInfo( dnmi::db::Connection *con,
                        long stationid,
                        const miutil::miTime &obstime,
                        long typeid_,
                        const miutil::miTime &tbtime );

   bool getData( dnmi::db::Connection *conection,
                 int stationid,
                 int typeid_,
                 const miutil::miTime &obstime,
                 std::list<kvalobs::kvData> &data,
                 std::list<kvalobs::kvTextData> &textData );

   void getData( dnmi::db::Connection *conection,
                 const std::list<std::string> &query,
                 std::list<kvalobs::kvData> &data );
   void getTextData( dnmi::db::Connection *conection,
                     const std::list<std::string> &query,
                     std::list<kvalobs::kvTextData> &data );


   bool getDataWithTbtime( dnmi::db::Connection *con,
                           int stationid,
                           int typeid_,
                           const std::string &tbtime,
                           std::list<kvalobs::kvData> &data,
                           std::list<kvalobs::kvTextData> &textData );

   bool hasDataWithTbtime( dnmi::db::Connection *con, const miutil::miTime &tbtime, int &msec );
   miutil::miTime getUniqTbtime( dnmi::db::Connection *con, int &msec );
   bool isEqual( std::list<kvalobs::kvData> &oldData,
                 std::list<kvalobs::kvTextData> &oldTextData );
   void insertData( dnmi::db::Connection *conection,
                    const std::list<kvalobs::kvData> &data,
                    const std::list<kvalobs::kvTextData> &textData );

   void insert( dnmi::db::Connection *conection,
                const kvalobs::kvDbBase &elem,
                const std::string &tblName );
   void update( dnmi::db::Connection *connection,
                const std::list<kvalobs::kvData> &data,
                const std::list<kvalobs::kvTextData> &textData );


   void replaceData( dnmi::db::Connection *conection,
                     const std::list<kvalobs::kvData> &dataList,
                     const std::list<kvalobs::kvTextData> &textDataList );

   DataUpdateTransaction( const miutil::miTime &obstime,
                          int stationid,
                          int typeid_,
                          int priority,
                          std::list<kvalobs::kvData> *newData,
                          std::list<kvalobs::kvTextData> *newTextData,
                          const std::string &logid );
   DataUpdateTransaction(const DataUpdateTransaction &dut );

   virtual ~DataUpdateTransaction();
   virtual bool operator()(dnmi::db::Connection *conection);
   virtual void onAbort( const std::string &driverid,
                         const std::string &errorMessage,
                         const std::string &errorCode );
   virtual void onSuccess();
   virtual void onRetry();
   virtual void onMaxRetry( const std::string &lastError );

   kvalobs::kvStationInfoList stationInfoList()const { return *stationInfoList_; }
   bool ok()const { return *ok_; }
};



}
}
#endif
