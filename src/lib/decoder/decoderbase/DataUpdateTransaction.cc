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
#include <miutil/miTimeParse.h>
#include "DataUpdateTransaction.h"

using namespace std;

namespace kvalobs{

namespace decoder{


DataUpdateTransaction::
DataUpdateTransaction( const miutil::miTime &obstime,
                       int stationid,
                       int typeid_,
                       std::list<kvalobs::kvData> *newData,
                       std::list<kvalobs::kvTextData> *newTextData )
   : newData( newData ), newTextData( newTextData ), obstime( obstime ),
     stationid( stationid ), typeid_( typeid_ ), ok_( new bool( false ) )
{
}

DataUpdateTransaction::
DataUpdateTransaction(const DataUpdateTransaction &dut )
   : newData( dut.newData ), newTextData( dut.newTextData ),
     obstime( dut.obstime ), stationid( dut.stationid ),
     typeid_( dut.typeid_ )
{
}

bool
DataUpdateTransaction::
hasDataWithTbtime( dnmi::db::Connection *con, const miutil::miTime &tbtime, int &msec )
{
   ostringstream q;

   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
     << "typeid=" << typeid_ << " AND "
     << "tbtime='" << tbtime << "." << msec <<"'";

   auto_ptr<dnmi::db::Result> res;

   dnmi::db::Result *dbRes = con->execQuery( q.str() );

   if( ! dbRes )
      throw logic_error("EXCEPTION: hasDataWithTimestamp: dbRes, NULL pointer.");

   res.reset( dbRes );

   return res->size()==0;
}


miutil::miTime
DataUpdateTransaction::
getTimestamp( dnmi::db::Connection *con, int &msec )
{
   msec=0;
   dnmi::db::Result *res=con->execQuery("SELECT now()");

   if( ! res )
      return miutil::miTime();

   if (res->hasNext()) {
      dnmi::db::DRow & row = res->next();
      return miutil::isoTimeWithMsec( row[0], msec );
   }

   return miutil::miTime();
}

miutil::miTime
DataUpdateTransaction::
getUniqTbtime( dnmi::db::Connection *con, int &msec )
{
   miutil::miTime t;
   while( true ) {
      t = getTimestamp(con, msec );

      if( t.undef() )
         continue;

      if( hasDataWithTbtime( con, t, msec ) )
         continue;

      return t;
   }
}


bool
DataUpdateTransaction::
isEqual( list<kvalobs::kvData> &oldData,
         list<kvalobs::kvTextData> &oldTextData )
{
   if( oldData.size() != newData->size() ||
       oldTextData.size() != newTextData->size() )
      return false;

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
getData( dnmi::db::Connection *con,
         int stationid,
         int typeid_,
         const miutil::miTime &obstime,
         list<kvalobs::kvData> &data,
         list<kvalobs::kvTextData> &textData )
{
   ostringstream q;
   data.clear();
   textData.clear();

   q << "SELECT * FROM data WHERE stationid=" << stationid << " AND "
     << "typeid=" << typeid_ << " AND "
     << "obstime='" << obstime << "' WHERE original<>-32767";


   auto_ptr<dnmi::db::Result> res;

   dnmi::db::Result *dbRes = con->execQuery( q.str() );


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
     << "obstime='" << obstime << "'";

   dbRes = con->execQuery( q.str() );

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

  conection->exec( ost.str() );
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
   }

   for( list<kvalobs::kvTextData>::iterator nit=newTextData->begin();
         nit != newTextData->end(); ++nit ) {
      nit->tbtime( tbtime, msec );
      insert( conection, *nit, "text_data" );
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

   conection->exec( q.str() );
   q.str("");

   q << "DELETE FROM text_data WHERE stationid=" << stationid << " AND abs(typeid)=" << typeid_
     << " AND (obstime='" << obstime << "' OR tbtime='" << tbtime << "." << msec << "')";

   conection->exec( q.str() );

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

   if( ! getData( conection, stationid, typeid_, obstime, dataList, textDataList ) )
      return false;

   if( dataList.empty() && textDataList.empty() ) {
      insertData( conection );
      return true;
   }

   if( isEqual( dataList, textDataList ) ) {
      return true;
   }

   replaceData( conection, dataList, textDataList );

   return true;
}
void
DataUpdateTransaction::
onSuccess()
{
   *ok_ = true;
}



}
}
