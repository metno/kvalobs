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
#include <memory>
#include <stdexcept>
#include <sstream>
#include <kvdb/kvdb.h>
#include <milog/milog.h>
#include <kvalobs/kvWorkelement.h>
#include <miutil/miTimeParse.h>
#include "DataUpdateTransaction.h"

using namespace std;

namespace kvalobs{

namespace decoder{


DataUpdateTransaction::
DataUpdateTransaction( const miutil::miTime &obstime,
                       int stationid,
                       int typeid_,
                       int priority,
                       std::list<kvalobs::kvData> *newData,
                       std::list<kvalobs::kvTextData> *newTextData,
                       const std::string &logid )
   : newData( newData ), newTextData( newTextData ), obstime( obstime ),
     stationid( stationid ), typeid_( typeid_ ), priority( priority ),
     stationInfoList_( new kvalobs::kvStationInfoList() ),
     ok_( new bool( false ) ), logid( logid )
{
}

DataUpdateTransaction::
DataUpdateTransaction(const DataUpdateTransaction &dut )
   : newData( dut.newData ), newTextData( dut.newTextData ),
     obstime( dut.obstime ), stationid( dut.stationid ),
     typeid_( dut.typeid_ ), priority( dut.priority ),
     stationInfoList_( dut.stationInfoList_ ),
     ok_( dut.ok_ ), logid( dut.logid )
{
}

void
DataUpdateTransaction::
addStationInfo( dnmi::db::Connection *con,
                long stationID,
                const miutil::miTime &obsTime,
                long typeID,
                const miutil::miTime &tbTime )
{
   IkvStationInfoList it=stationInfoList_->begin();

   for(;it!=stationInfoList_->end(); it++){
     if(it->stationID()==stationID &&
        it->obstime()==obsTime     &&
        it->typeID()==typeID){

       return;
     }
   }

   ostringstream q;
   miutil::miTime undefTime;

   q << "DELETE FROM workque WHERE stationid=" << stationID << " AND "
     << "typeid=" << typeID << "AND obstime='" << obsTime << "'";

   try {
      con->exec( q.str() );
   }
   catch( const std::exception &ex ) {
      cerr << "addStationInfo: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      cerr << "addStationInfo: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }

   kvalobs::kvWorkelement workque( stationID,
                                   obsTime,
                                   typeID,
                                   tbTime,
                                   priority,
                                   undefTime,
                                   undefTime,
                                   undefTime,
                                   undefTime,
                                   undefTime );

   q.str("");

   q << "INSERT INTO " << workque.tableName()
     << " VALUES" << workque.toSend() << ";";

   try {
      con->exec( q.str() );
   }
   catch( const std::exception &ex ) {
      cerr << "addStationInfo: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      cerr << "addStationInfo: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }

   stationInfoList_->push_back( kvalobs::kvStationInfo( stationID, obsTime, typeID) );
}


bool
DataUpdateTransaction::
hasDataWithTbtime( dnmi::db::Connection *con, const miutil::miTime &tbtime, int &msec )
{
   ostringstream q;
   dnmi::db::Result *dbRes;

   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
     << "typeid=" << typeid_ << " AND "
     << "tbtime='" << tbtime << "." << msec <<"'";

   auto_ptr<dnmi::db::Result> res;

   try {
      dbRes = con->execQuery( q.str() );
   }
   catch( const std::exception &ex ) {
      log << "hasDataWithTbtime: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "hasDataWithTbtime: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }

   if( ! dbRes )
      throw logic_error("EXCEPTION: hasDataWithTimestamp: dbRes, NULL pointer.");

   res.reset( dbRes );

   return res->size() != 0;
}


miutil::miTime
DataUpdateTransaction::
getTimestamp( dnmi::db::Connection *con, int &msec )
{
   dnmi::db::Result *res;
   string q("SELECT now()");

   msec=0;

   try {
      res=con->execQuery( q );
   }
   catch( const std::exception &ex ) {
      log << "getTimestamp: '" << q << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "getTimestamp: '" << q << "' \nReason: Unknown \n";
      throw;
   }


   if( ! res )
      return miutil::miTime();

   if (res->hasNext()) {
      dnmi::db::DRow & row = res->next();
      cerr << "Now: " << row[0] << endl;
      return miutil::isoTimeWithMsec( row[0], msec );
   }

   return miutil::miTime();
}

miutil::miTime
DataUpdateTransaction::
getUniqTbtime( dnmi::db::Connection *con, int &msec )
{
   miutil::miTime t;

   for( int i=0; i<10000; ++i ) {
      if( t.undef() )
         t = getTimestamp(con, msec );

      if( t.undef() ) {
         cerr << "getUniqTbtime: undef.\n";
         continue;
      }

      msec = msec % 1000000;

      if( hasDataWithTbtime( con, t, msec ) ) {
         msec++;
         continue;
      }

      return t;
   }
}


bool
DataUpdateTransaction::
isEqual( list<kvalobs::kvData> &oldData,
         list<kvalobs::kvTextData> &oldTextData )
{

   if( oldData.size() != newData->size() ||
       oldTextData.size() != newTextData->size() ) {
      log << "isEqual: size differ: " << oldData.size() << " (" << newData->size() << ") "
           << "- " << oldTextData.size() << " (" << newTextData->size() << ")\n";

      return false;
   }

   bool found;

   for( list<kvalobs::kvData>::const_iterator oit=oldData.begin();
        oit != oldData.end();  ++oit ) {
      found = false;
      for( list<kvalobs::kvData>::const_iterator nit=newData->begin();
            nit != newData->end();  ++nit ) {
         if( oit->obstime() == nit->obstime() &&
             oit->stationID() == nit->stationID() &&
             oit->typeID() == nit->typeID() &&
             oit->paramID() == nit->paramID() &&
             oit->sensor() == nit->sensor() &&
             oit->level() == nit->level() ) {
            float nv = nit->original();
            float ov = oit->original();

            if( static_cast<int>( (nv+0.05)*10 ) ==
                static_cast<int>( (ov+0.05)*10 )    ){
               found=true;
               break;
            }
         }
      }

      if( ! found )
         return false;
   }

   for( list<kvalobs::kvTextData>::const_iterator oit=oldTextData.begin();
           oit != oldTextData.end();  ++oit ) {
         found = false;
         for( list<kvalobs::kvTextData>::const_iterator nit=newTextData->begin();
               nit != newTextData->end();  ++nit ) {
            if( oit->obstime() == nit->obstime() &&
                oit->stationID() == nit->stationID() &&
                oit->typeID() == nit->typeID() &&
                oit->paramID() == nit->paramID() &&
                oit->original() == nit->original() ) {
               found=true;
               break;
            }
         }

         if( ! found )
            return false;
   }

   return true;
}
bool
DataUpdateTransaction::
getDataWithTbtime( dnmi::db::Connection *con,
                   int stationid,
                   int typeid_,
                   const std::string &tbtime,
                   list<kvalobs::kvData> &data,
                   list<kvalobs::kvTextData> &textData )
{
   ostringstream q;
   dnmi::db::Result *dbRes;

   data.clear();
   textData.clear();

   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
     << "typeid=" << typeid_ << " AND "
     << "tbtime='" << tbtime << "' AND original<>-32767";


   auto_ptr<dnmi::db::Result> res;

   try {
      dbRes = con->execQuery( q.str() );
   }
   catch( const std::exception &ex ) {
      log << "getDataWithTbtime: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "getDataWithTbtime: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }


   if( ! dbRes )
      return false;

   res.reset( dbRes );


   while (res->hasNext()) {
      dnmi::db::DRow & row = res->next();
      data.push_back( kvalobs::kvData( row ) );
   }

   q.str("");
   q << "SELECT * FROM text_data WHERE stationid=" << stationid << " AND "
     << "typeid=" << typeid_ << " AND "
     << "tbtime='" << tbtime << "'";

   try{
      dbRes = con->execQuery( q.str() );
   }
   catch( const std::exception &ex ) {
      log << "getDataWithTbtime: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "getDataWithTbtime: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }

   if( ! dbRes )
      return false;

   res.reset( dbRes );

   while (res->hasNext()) {
      dnmi::db::DRow & row = res->next();
      textData.push_back( kvalobs::kvTextData( row ) );
   }

   return true;
}


bool
DataUpdateTransaction::
getData( dnmi::db::Connection *con,
         int stationid,
         int typeid_,
         const miutil::miTime &obstime,
         list<kvalobs::kvData> &data,
         list<kvalobs::kvTextData> &textData )
{
   ostringstream q;
   dnmi::db::Result *dbRes;

   data.clear();
   textData.clear();

   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
     << "typeid=" << typeid_ << " AND "
     << "obstime='" << obstime << "' AND original<>-32767";


   auto_ptr<dnmi::db::Result> res;

   try {
      dbRes = con->execQuery( q.str() );
   }
   catch( const std::exception &ex ) {
      log << "getData: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "getData: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }


   if( ! dbRes )
      return false;

   res.reset( dbRes );

   std::string myTbtime;
   bool eqTbtime=true;

   while (res->hasNext()) {
      dnmi::db::DRow & row = res->next();
      data.push_back( kvalobs::kvData( row ) );

      if( myTbtime.empty() )
         myTbtime = row["tbtime"];
      else if( myTbtime != row["tbtime"] )
         eqTbtime = false;
   }

   if( eqTbtime && !myTbtime.empty() ) {
      log << "getData: all data has the same tbtime '" << myTbtime << "' as expected.\n";
      return getDataWithTbtime( con, stationid, typeid_, myTbtime, data, textData );
   }

   q.str("");
   q << "SELECT * FROM text_data WHERE stationid=" << stationid << " AND "
     << "typeid=" << typeid_ << " AND "
     << "obstime='" << obstime << "'";

   try{
      dbRes = con->execQuery( q.str() );
   }
   catch( const std::exception &ex ) {
      log << "getData: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "getData: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }

   if( ! dbRes )
      return false;

   res.reset( dbRes );

   while (res->hasNext()) {
      dnmi::db::DRow & row = res->next();
      textData.push_back( kvalobs::kvTextData( row ) );
   }

   return true;
}

void
DataUpdateTransaction::
insert( dnmi::db::Connection *conection,
        const kvalobs::kvDbBase &elem,
        const std::string &tblName )
{
  ostringstream ost;

  ost << "INSERT INTO " << tblName
      << " VALUES" << elem.toSend() << ";";

  try {
     conection->exec( ost.str() );
  }
  catch( const std::exception &ex ) {
     log << "insert: '" << ost.str() << "' \nReason: " << ex.what() << "\n";
     throw;
  }
  catch( ... ) {
     log << "insert: '" << ost.str() << "' \nReason: Unknown \n";
     throw;
  }

}


void
DataUpdateTransaction::
insertData(dnmi::db::Connection *conection)
{
   int msec;
   miutil::miTime tbtime;

   tbtime=getUniqTbtime( conection, msec );

   for( list<kvalobs::kvData>::iterator nit=newData->begin();
        nit != newData->end(); ++nit ) {
      nit->tbtime( tbtime, msec );
      insert( conection, *nit, "data" );
      addStationInfo( conection, nit->stationID(), nit->obstime(), nit->typeID(), tbtime );
   }

   for( list<kvalobs::kvTextData>::iterator nit=newTextData->begin();
         nit != newTextData->end(); ++nit ) {
      nit->tbtime( tbtime, msec );
      insert( conection, *nit, "text_data" );
      addStationInfo( conection, nit->stationID(), nit->obstime(), nit->typeID(), tbtime );
   }
}

void
DataUpdateTransaction::
replaceData( dnmi::db::Connection *conection,
             const std::list<kvalobs::kvData> &dataList,
             const std::list<kvalobs::kvTextData> &textDataList )
{
   ostringstream q;
   miutil::miTime tbtime;
   int msec;

   if( ! dataList.empty()) {
      tbtime = dataList.begin()->tbtime();
      msec = dataList.begin()->tbtimemsec();
   } else {
      tbtime = textDataList.begin()->tbtime();
      msec = textDataList.begin()->tbtimemsec();
   }

   q << "DELETE FROM data WHERE stationid=" << stationid << " AND abs(typeid)=" << typeid_
     << " AND (obstime='" << obstime << "' OR tbtime='" << tbtime << "." << msec << "')";

   try {
      conection->exec( q.str() );
   }
   catch( const std::exception &ex ) {
      log << "replaceData: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "replaceData: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }

   q.str("");

   q << "DELETE FROM text_data WHERE stationid=" << stationid << " AND abs(typeid)=" << typeid_
     << " AND (obstime='" << obstime << "' OR tbtime='" << tbtime << "." << msec << "')";

   try {
      conection->exec( q.str() );
   }
   catch( const std::exception &ex ) {
      log << "replaceData: '" << q.str() << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "replaceData: '" << q.str() << "' \nReason: Unknown \n";
      throw;
   }

   insertData( conection );
}


bool
DataUpdateTransaction::
operator()( dnmi::db::Connection *conection )
{
   list<kvalobs::kvData> dataList;
   list<kvalobs::kvTextData> textDataList;
   miutil::miTime tbtime;
   int msec;

   stationInfoList_->clear();

   if( ! getData( conection, stationid, typeid_, obstime, dataList, textDataList ) )
      return false;

   if( dataList.empty() && textDataList.empty() ) {
      log << "New new data. stationid: " << stationid << " typeid: " << typeid_
          << " obstime: " << obstime << endl;
      insertData( conection );
      return true;
   }

   if( isEqual( dataList, textDataList ) ) {
      log << "Data allready exist. stationid: " << stationid << " typeid: " << typeid_
          << " obstime: " << obstime << endl;
      return true;
   }

   log << "Replace data.stationid: " << stationid << " typeid: " << typeid_
       << " obstime: " << obstime << endl;
   replaceData( conection, dataList, textDataList );

   return true;
}

void
DataUpdateTransaction::
onSuccess()
{
  LOGINFO( log.str() );

  if( ! logid.empty() ){
     IDLOGINFO( logid, log.str() );
  }

  *ok_ = true;
}

void
DataUpdateTransaction::
onRetry()
{
   LOGDEBUG( "Retry transaction.\n" << log.str() );

   if( ! logid.empty() ) {
      IDLOGDEBUG( logid, "Retry transaction.\n" << log.str() );
   }

   log.str("");
   stationInfoList_->clear();
}

void
DataUpdateTransaction::
onMaxRetry( const std::string &lastError )
{
   LOGERROR("Transaction Failed.\n" << lastError << "\n" << log.str() );

   if( ! logid.empty() ) {
      IDLOGERROR( logid, "Transaction Failed.\n" << lastError << "\n" << log.str() );
   }
}

}
}