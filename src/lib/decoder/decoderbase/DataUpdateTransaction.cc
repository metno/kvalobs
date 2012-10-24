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
DataUpdateTransaction( const boost::posix_time::ptime &obstime,
                       int stationid,
                       int typeid_,
                       int priority,
                       std::list<kvalobs::kvData> *newData,
                       std::list<kvalobs::kvTextData> *newTextData,
                       const std::string &logid )
   : newData( newData ), newTextData( newTextData ), obstime( obstime ),
     stationid( stationid ), typeid_( typeid_ ), priority( priority ),
     stationInfoList_( new kvalobs::kvStationInfoList() ),
     ok_( new bool( false ) ), logid( logid ), nRetry( 0 )
{
}

DataUpdateTransaction::
DataUpdateTransaction(const DataUpdateTransaction &dut )
   : newData( dut.newData ), newTextData( dut.newTextData ),
     obstime( dut.obstime ), stationid( dut.stationid ),
     typeid_( dut.typeid_ ), priority( dut.priority ),
     stationInfoList_( dut.stationInfoList_ ),
     ok_( dut.ok_ ), logid( dut.logid ), nRetry( dut.nRetry )
{
}


void
DataUpdateTransaction::
addQuery( std::list<std::string> &qList, const std::string &query )
{
   for( std::list<std::string>::iterator it=qList.begin(); it != qList.end(); ++it ){
      if( query == *it )
         return;
   }

   qList.push_back( query );
}

void
DataUpdateTransaction::
addStationInfo( dnmi::db::Connection *con,
                long stationID,
                const boost::posix_time::ptime &obsTime,
                long typeID,
                const boost::posix_time::ptime &tbTime )
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
   boost::posix_time::ptime undefTime;

   q << "DELETE FROM workque WHERE stationid=" << stationID << " AND "
     << "typeid=" << typeID << " AND obstime='" << obsTime << "'";

   try {
      con->exec( q.str() );
   }
   catch( const dnmi::db::SQLException &ex ) {
      log << "addStationInfo (delete from workque): '" << q.str() << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
   }
   catch( const std::exception &ex ) {
      log << "addStationInfo: '" << q.str() << "' \nReason: " << ex.what() << endl;
      throw;
   }
   catch( ... ) {
      log << "addStationInfo: '" << q.str() << "' \nReason: Unknown." << endl;
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
   catch( const dnmi::db::SQLException &ex ) {
      log << "addStationInfo (add to workque): '" << q.str() << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
   }
   catch( const std::exception &ex ) {
      log << "addStationInfo: '" << q.str() << "' \nReason: " << ex.what() << endl;
      throw;
   }
   catch( ... ) {
      log << "addStationInfo: '" << q.str() << "' \nReason: Unknown." << endl;
      throw;
   }

   stationInfoList_->push_back( kvalobs::kvStationInfo( stationID, obsTime, typeID) );
}


bool
DataUpdateTransaction::
hasDataWithTbtime( dnmi::db::Connection *con, const boost::posix_time::ptime &tbtime )
{
   ostringstream q;
   dnmi::db::Result *dbRes;


//   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
//     << "typeid=" << typeid_ << " AND "
//     << "tbtime='" << tbtime << "." << msec <<"'";

   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
     << "tbtime='" << to_iso_extended_string(tbtime) <<"'";

   auto_ptr<dnmi::db::Result> res;

   try {
      dbRes = con->execQuery( q.str() );
   }
   catch( const dnmi::db::SQLException &ex ) {
      log << "hasDataWithTbtime: '" << q.str() << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
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


boost::posix_time::ptime
DataUpdateTransaction::
getTimestamp( dnmi::db::Connection *con )
{
    std::auto_ptr<dnmi::db::Result> res;
   string q("SELECT now()");

   try {
       res.reset(con->execQuery( q ));
   }
   catch( const dnmi::db::SQLException &ex ) {
      log << "getTimestamp: '" << q << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
   }
   catch( const std::exception &ex ) {
      log << "getTimestamp: '" << q << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "getTimestamp: '" << q << "' \nReason: Unknown \n";
      throw;
   }


   if( res.get() != 0 && res->hasNext() ) {
      dnmi::db::DRow & row = res->next();
      return boost::posix_time::time_from_string(row[0]);
   }

   return boost::posix_time::ptime();
}

boost::posix_time::ptime
DataUpdateTransaction::
getUniqTbtime( dnmi::db::Connection *con )
{
   boost::posix_time::ptime t;

   for( int i=0; i<10000; ++i ) {
      if( t.is_not_a_date_time() )
         t = getTimestamp(con);

      if( t.is_not_a_date_time() ) {
         continue;
      }

      if( hasDataWithTbtime( con, t ) ) {
         t += boost::posix_time::microseconds(1);
         continue;
      }

      return t;
   }
}

void
DataUpdateTransaction::
setTbtime( dnmi::db::Connection *conection )
{
   boost::posix_time::ptime tbtime;

   tbtime=getUniqTbtime( conection );

   for( list<kvalobs::kvData>::iterator nit=newData->begin();
        nit != newData->end(); ++nit ) {
      nit->tbtime( tbtime );
   }

   for( list<kvalobs::kvTextData>::iterator nit=newTextData->begin();
         nit != newTextData->end(); ++nit ) {
      nit->tbtime( tbtime );
   }
}

bool
DataUpdateTransaction::
isEqual( list<kvalobs::kvData> &oldData_,
         list<kvalobs::kvTextData> &oldTextData )
{

   list<kvalobs::kvData> oldData( oldData_ );

   for( list<kvalobs::kvData>::iterator it=oldData.begin(); it != oldData.end(); ++it ){
      if( it->original() == -32767 )
         it = oldData.erase( it );
   }

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

            if( static_cast<int>( (nv+0.005)*100 ) ==
                static_cast<int>( (ov+0.005)*100 )    ){
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

//   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
//     << "typeid=" << typeid_ << " AND "
//     << "tbtime='" << tbtime << "' AND original<>-32767";

//   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
//     << "tbtime='" << tbtime << "' AND original<>-32767";
   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
     << "tbtime='" << tbtime << "'";


   auto_ptr<dnmi::db::Result> res;

   try {
      dbRes = con->execQuery( q.str() );
   }
   catch( const dnmi::db::SQLException &ex ) {
      log << "getDataWithTbtime: '" << q.str() << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
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
//   q << "SELECT * FROM text_data WHERE stationid=" << stationid << " AND "
//     << "typeid=" << typeid_ << " AND "
//     << "tbtime='" << tbtime << "'";

   q << "SELECT * FROM text_data WHERE stationid=" << stationid << " AND "
     << "tbtime='" << tbtime << "'";

   try{
      dbRes = con->execQuery( q.str() );
   }
   catch( const dnmi::db::SQLException &ex ) {
      log << "getDataWithTbtime: '" << q.str() << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
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
         const boost::posix_time::ptime &obstime,
         list<kvalobs::kvData> &data,
         list<kvalobs::kvTextData> &textData )
{
   ostringstream q;
   dnmi::db::Result *dbRes;

   data.clear();
   textData.clear();

   /*
   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
     << "typeid=" << typeid_ << " AND "
     << "obstime='" << obstime << "' AND original<>-32767";
    */

   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
       << "typeid=" << typeid_ << " AND "
       << "obstime='" << obstime << "'";

   auto_ptr<dnmi::db::Result> res;

   try {
      dbRes = con->execQuery( q.str() );
   }
   catch( const dnmi::db::SQLException &ex ) {
      log << "getData: '" << q.str() << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
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
   catch( const dnmi::db::SQLException &ex ) {
      log << "getData: '" << q.str() << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
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
  catch( const dnmi::db::SQLException &ex ) {
     log << "insert: '" << ost.str() << "' \nSQLState: " << ex.errorCode()
         << "\nReason: " << ex.what() << endl;
     throw;
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
insertData(dnmi::db::Connection *conection,
           const std::list<kvalobs::kvData> &data,
           const std::list<kvalobs::kvTextData> &textData )
{

   boost::posix_time::ptime tbtime(boost::posix_time::microsec_clock::universal_time() );

   for( list<kvalobs::kvData>::const_iterator nit=data.begin();
        nit != data.end(); ++nit ) {
      insert( conection, *nit, "data" );
      addStationInfo( conection, nit->stationID(), nit->obstime(), nit->typeID(), tbtime );
   }

   for( list<kvalobs::kvTextData>::const_iterator nit=textData.begin();
         nit != textData.end(); ++nit ) {
      insert( conection, *nit, "text_data" );
      addStationInfo( conection, nit->stationID(), nit->obstime(), nit->typeID(), tbtime );
   }
}

bool
DataUpdateTransaction::
addDataToList( const kvalobs::kvData &data,
               std::list<kvalobs::kvData> &dataList, bool replaceOnly )
{
   for( std::list<kvalobs::kvData>::iterator it = dataList.begin();
        it != dataList.end(); ++it ) {
      if( data.obstime() == it->obstime() &&
          data.stationID() == it->stationID() &&
          data.typeID() == it->typeID() &&
          data.paramID() == it->paramID() &&
          data.sensor() == it->sensor() &&
          data.level() == it->level() ) {

         if( replaceOnly ) {
            *it = data;
            return true;
         } else {
            return false;
         }
      }
   }

   if( replaceOnly )
      return false;

   dataList.push_back( data );
   return true;
}

bool
DataUpdateTransaction::
addTextDataToList( const kvalobs::kvTextData &data,
                   std::list<kvalobs::kvTextData> &dataList,
                   bool replaceOnly )
{
   for( std::list<kvalobs::kvTextData>::iterator it = dataList.begin();
        it != dataList.end(); ++it ) {
      if( data.obstime() == it->obstime() &&
          data.stationID() == it->stationID() &&
          data.typeID() == it->typeID() &&
          data.paramID() == it->paramID() ) {
         if( replaceOnly ) {
            *it = data;
            return true;
         } else {
            return false;
         }
      }
   }
   if( replaceOnly )
      return false;

   dataList.push_back( data );
   return true;
}

void
DataUpdateTransaction::
getData( dnmi::db::Connection *conection,
         const std::list<std::string> &query,
         std::list<kvalobs::kvData> &data )
{
   dnmi::db::Result *dbRes;
   string q;
   data.clear();
   auto_ptr<dnmi::db::Result> res;

   log << "getData: # of queries: " << query.size() << endl;
   for( list<string>::const_iterator it=query.begin(); it!=query.end(); ++it )
      log << "\n  '" << *it << "'";
   log << endl;


   try {
      for( std::list<std::string>::const_iterator it=query.begin();
           it != query.end(); ++it ) {
         q = *it;
         dbRes = conection->execQuery( *it );

         if( ! dbRes )
            continue;

         res.reset( dbRes );

         while (res->hasNext()) {
            dnmi::db::DRow & row = res->next();
            addDataToList( kvalobs::kvData( row ), data );
         }
      }
    }
   catch( const dnmi::db::SQLException &ex ) {
      log << "getData: '" << q << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
   }
    catch( const std::exception &ex ) {
       log << "getData: '" << q << "' \nReason: " << ex.what() << "\n";
       throw;
    }
    catch( ... ) {
       log << "getData: '" << q << "' \nReason: Unknown \n";
       throw;
    }
}

void
DataUpdateTransaction::
getTextData( dnmi::db::Connection *conection,
             const std::list<std::string> &query,
             std::list<kvalobs::kvTextData> &data )
{
   dnmi::db::Result *dbRes;
   string q;
   data.clear();
   auto_ptr<dnmi::db::Result> res;

   log << "getTextData: # of queries: " << query.size() << endl;
   for( list<string>::const_iterator it=query.begin(); it!=query.end(); ++it )
      log << "\n  '" << *it << "'";
   log << endl;


   try {
      for( std::list<std::string>::const_iterator it=query.begin();
           it != query.end(); ++it ) {
         q = *it;
         dbRes = conection->execQuery( *it );

         if( ! dbRes )
            continue;

         res.reset( dbRes );

         while (res->hasNext()) {
            dnmi::db::DRow & row = res->next();
            addTextDataToList( kvalobs::kvTextData( row ), data );
         }
      }
   }
   catch( const dnmi::db::SQLException &ex ) {
      log << "getTextData: '" << q << "' \nSQLState: " << ex.errorCode()
          << "\nReason: " << ex.what() << endl;
      throw;
   }
   catch( const std::exception &ex ) {
      log << "getTextData: '" << q << "' \nReason: " << ex.what() << "\n";
      throw;
   }
   catch( ... ) {
      log << "getTextData: '" << q << "' \nReason: Unknown \n";
      throw;
   }
}


void
DataUpdateTransaction::
replaceData( dnmi::db::Connection *conection,
             const std::list<kvalobs::kvData> &dataList,
             const std::list<kvalobs::kvTextData> &textDataList )
{
   list<string> qDataList;
   list<string> qTextDataList;
   list<kvalobs::kvData> oldData;
   list<kvalobs::kvTextData> oldTextData;
   list<kvalobs::kvData> myNewData;
   list<kvalobs::kvTextData> myNewTextData;
   ostringstream q;
   boost::posix_time::ptime tbtime;
   bool onlyMissing=true;

   for( list<kvalobs::kvData>::const_iterator it=dataList.begin(); it != dataList.end();
         ++it ) {
      if( it->original() != -32767 ) {
         onlyMissing = false;
         break;
      }
   }

   if( onlyMissing && ! dataList.empty() ) {
      tbtime = dataList.begin()->tbtime();
   }else if( ! dataList.empty()) {
      for( list<kvalobs::kvData>::const_iterator it=dataList.begin(); it != dataList.end();
           ++it ) {
         if( it->original() != -32767 ) {
            tbtime = it->tbtime();
            break;
         }
      }
   }

   if( tbtime.is_not_a_date_time() && ! textDataList.empty() )
      tbtime = textDataList.begin()->tbtime();

   if( tbtime.is_not_a_date_time() ){
      insertData( conection, *newData, *newTextData );
      return;
   }

   for( std::list<kvalobs::kvData>::const_iterator it=dataList.begin();
         it != dataList.end(); ++it ) {
      q.str("");
      tbtime = it->tbtime();
      q << "SELECT * FROM data WHERE stationid=" << it->stationID()
        << " AND abs(typeid)=" << it->typeID()
        << " AND (obstime='" << obstime << "' OR tbtime='" << to_iso_extended_string(tbtime) << "')";

      addQuery( qDataList, q.str() );
   }

   for( std::list<kvalobs::kvTextData>::const_iterator it=textDataList.begin();
            it != textDataList.end(); ++it ) {
      q.str("");
      tbtime = it->tbtime();
      q << "SELECT * FROM text_data WHERE stationid=" << it->stationID()
        << " AND abs(typeid)=" << it->typeID()
        << " AND (obstime='" << obstime << "' OR tbtime='" << to_iso_extended_string(tbtime) << "')";

      addQuery( qTextDataList, q.str() );
   }

   getData( conection, qDataList, oldData );
   getTextData( conection, qTextDataList, oldTextData );

   //Mark the oldData as deleted
   int fmis;
   kvControlInfo cinfo;

   for( list<kvalobs::kvData>::iterator it = oldData.begin();
        it != oldData.end(); ++it ) {
      cinfo = it->controlinfo();
      fmis = cinfo.MissingFlag();

      //Do not mark missing and negative typeids as deleted in the
      //new message.
      if( ( fmis != 0 && fmis != 2 && fmis != 4 ) ||
          ( it->typeID() < 0 ) ) {
         //it = oldData.erase( it );
         continue;
      }

      it->corrected( -32766 );

      cinfo.setControlFlag( kvQCFlagTypes::f_fpre, 7 );
      cinfo.setControlFlag( kvQCFlagTypes::f_fmis, 2 );

      it->controlinfo( cinfo );
   }

   //Mark the oldtextData as deleted
   for( list<kvalobs::kvTextData>::iterator it = oldTextData.begin();
         it != oldTextData.end(); ++it ) {
      it->tbtime( it->tbtime() );
      it->set( it->stationID(), it->obstime(), "", it->paramID(), it->tbtime(), it->typeID() );
   }

   //We prepare the data in oldData for update and myNewData for insert.
   for( list<kvalobs::kvData>::iterator it = newData->begin();
        it != newData->end(); ++it ) {
      if( ! addDataToList( *it, oldData, true ) )
         myNewData.push_back( *it );
   }

   for( list<kvalobs::kvTextData>::iterator it = newTextData->begin();
        it != newTextData->end(); ++it ) {
      if( ! addTextDataToList( *it, oldTextData, true ) )
         myNewTextData.push_back( *it );
   }

   insertData( conection, myNewData, myNewTextData );
   update( conection, oldData, oldTextData );
}

void
DataUpdateTransaction::
update( dnmi::db::Connection *connection,
        const std::list<kvalobs::kvData> &data,
        const std::list<kvalobs::kvTextData> &textData )
{
   ostringstream ost;
   boost::posix_time::ptime tbtime( boost::posix_time::microsec_clock::universal_time() );

   for( std::list<kvalobs::kvData>::const_iterator it=data.begin();
        it != data.end(); ++it ) {
      ost.str("");

      ost << "UPDATE data SET "
          << "  corrected="    << it->corrected()
          << ", original=" << it->original()
          << ", controlinfo='"    << it->controlinfo().flagstring() << "'"
          << ", useinfo='"        << it->useinfo().flagstring() << "'"
          << ", cfailed='"        << it->cfailed() << "'"
          << ", tbtime='" << to_iso_extended_string(it->tbtime()) << "'"
          << " WHERE stationid=" << it->stationID() << " AND "
          << "       obstime='"   << to_iso_extended_string(it->obstime()) << "' AND "
          << "       paramid="   << it->paramID() << " AND "
          << "       typeid="    << it->typeID() << " AND "
          << "       sensor='"    << it->sensor() << "' AND "
          << "       level="     << it->level();

      connection->exec( ost.str() );
      addStationInfo( connection, it->stationID(), it->obstime(), it->typeID(), tbtime );
   }

   for( std::list<kvalobs::kvTextData>::const_iterator it=textData.begin();
          it != textData.end(); ++it ) {
      ost.str("");

      ost << "UPDATE text_data SET "
          << "  original='"    << it->original() << "'"
          << ", tbtime='" << to_iso_extended_string(it->tbtime()) <<"." << it->tbtimemsec() << "'"
          << " WHERE stationid=" << it->stationID() << " AND "
          << "       obstime='"   << to_iso_extended_string(it->obstime()) << "' AND "
          << "       paramid="   << it->paramID() << " AND "
          << "       typeid="    << it->typeID();

      connection->exec( ost.str() );
      addStationInfo( connection, it->stationID(), it->obstime(), it->typeID(), tbtime );
   }
}

bool
DataUpdateTransaction::
operator()( dnmi::db::Connection *conection )
{
   ostringstream mylog;
   list<kvalobs::kvData> dataList;
   list<kvalobs::kvTextData> textDataList;
   boost::posix_time::ptime tbtime;
   int msec;

   if( obstime.is_not_a_date_time() ) {
      LOGERROR("NewData: stationid: " << stationid << " typeid: " << typeid_
            << ". Invalid obstime.");
      return false;
   }

   if( !logid.empty() ) {
      bool err=false;
      for( std::list<kvalobs::kvData>::const_iterator it=newData->begin();
            it != newData->end(); ++it ) {
         if( it->obstime().is_not_a_date_time() ) {
            err = true;
            mylog << "Invalid obstime: " << it->stationID() << "," << it->typeID() <<","
                  << it->paramID() << "," << it->sensor() <<"," << it->level()
                  << "," << it->original() << endl;
         } else {
            mylog << it->obstime() << "," << it->stationID() << "," << it->typeID() <<","
                  << it->paramID() << "," << it->sensor() <<"," << it->level()
                  << "," << it->original() << endl;
         }
      }

      if( err ) {
         LOGERROR("NewData: INVALID OBSTIME stationid: " << stationid << " typeid: " << typeid_ << endl
                   << mylog.str() );
         return false;
      }

      log << "NewData: stationid: " << stationid << " typeid: " << typeid_
          << " obstime: " << obstime << endl << mylog.str()  << endl;
   }

   stationInfoList_->clear();

   if( ! getData( conection, stationid, typeid_, obstime, dataList, textDataList ) )
      return false;

   if( dataList.empty() && textDataList.empty() ) {
      log << "New data. stationid: " << stationid << " typeid: " << typeid_
          << " obstime: " << obstime << endl;
      setTbtime( conection );
      insertData( conection, *newData, *newTextData );
      insertType = "INSERT";
      return true;
   }

   if( isEqual( dataList, textDataList ) ) {
      log << "Data allready exist. stationid: " << stationid << " typeid: " << typeid_
          << " obstime: " << obstime << endl;
      IDLOGINFO("duplicates", "DUPLICATE: stationid: " << stationid << " typeid: " << typeid_
          << " obstime: " << obstime );
      insertType = "DUPLICATE";
      return true;
   }

   log << "Replace data.stationid: " << stationid << " typeid: " << typeid_
       << " obstime: " << obstime << endl;

   setTbtime( conection );
   replaceData( conection, dataList, textDataList );

   mylog.str("");
   IkvStationInfoList it=stationInfoList_->begin();

   if( it != stationInfoList_->end() ) {
      mylog << "UPDATED: stationid: " << it->stationID() << " typeid: " << it->typeID()
            << " obstime: " << it->obstime();
      ++it;
   }

   for(;it!=stationInfoList_->end(); it++){
      mylog << "\n         stationid: " << it->stationID() << " typeid: " << it->typeID()
            << " obstime: " << it->obstime();
   }

   IDLOGINFO( "updated", mylog.str() );
   insertType = "UPDATE";
   return true;
}

void
DataUpdateTransaction::
onSuccess()
{
   ostringstream mylog;
   string prefix( insertType.length(), ' ' );
   IkvStationInfoList it=stationInfoList_->begin();

   if( it != stationInfoList_->end() ) {
      mylog << insertType << ": stationid: " << it->stationID() << " typeid: " << it->typeID()
            << " obstime: " << it->obstime();
      ++it;
   } else {
      mylog << insertType << ": stationid: " << stationid << " typeid: " << typeid_
                  << " obstime: " << obstime;
   }

   for(;it!=stationInfoList_->end(); it++){
      mylog << "\n" << prefix
            << "  stationid: " << it->stationID() << " typeid: " << it->typeID()
            << " obstime: " << it->obstime();
   }

   IDLOGINFO( logid, log.str() );
   IDLOGINFO( "transaction", mylog.str() );
   *ok_ = true;
}

void
DataUpdateTransaction::
onRetry()
{

   if( ! logid.empty() ) {
      IDLOGDEBUG( logid, "Retry transaction.\n" << log.str() );
   }

   nRetry++;
   IDLOGDEBUG("retry", "RETRY: " << nRetry << " stationid: " << stationid << " Typeid: "
              << typeid_ << " obstime: " << obstime << "\nMessage: " << log.str() );
   log.str("");
   stationInfoList_->clear();
}

void
DataUpdateTransaction::
onAbort( const std::string &driverid,
         const std::string &errorMessage,
         const std::string &errorCode )
{
   if( !logid.empty() ) {
      IDLOGINFO( logid, "Transaction aborted: Driver: '" << driverid << "' ErrorCode: '"
                 << errorCode << "'.\nReason: " << errorMessage );
   }
}


void
DataUpdateTransaction::
onMaxRetry( const std::string &lastError )
{
   ostringstream mylog;

   if( ! log.str().empty() ) {
      mylog << endl << "Log: " << log.str();
   }

   IDLOGERROR( logid, "Transaction Failed.\n" << lastError << "\n" << log.str() );
   IDLOGERROR( "failed", "Transaction Failed. Stationid: " << stationid << " Typeid: "
               << typeid_ << " obstime: " << obstime  << "\nLast error: " << lastError
               << mylog.str() );
   IDLOGERROR( "transaction", "   FAILED: Stationid: " << stationid << " Typeid: "
               << typeid_ << " obstime: " << obstime );
}

}
}
