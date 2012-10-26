/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: RejectdecodeIterator.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include "WorkstatistikIterator.h"
#include <typeinfo>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <milog/milog.h>
#include <ctime>

#define NDEBUG
#include <cassert>

using namespace std;
using namespace kvalobs;
using namespace CKvalObs::CService;
using namespace milog;

namespace{
  FLogStream   *fs=0;
  StdErrStream *trace=0;
  LogLevel getLogLevel(const char *str);
  bool     setLoglevel(const string &ll, const string &tl);
}

namespace kvservice
{
WorkstatistikIterator::
WorkstatistikIterator()
   : iter( CKvalObs::CService::WorkstatistikIterator::_nil() )
    , dataList( new CKvalObs::CService::WorkstatistikElemList() )
    , index( 0 )
  {
  }


WorkstatistikIterator::
~WorkstatistikIterator()
{
   cleanup();
}


bool
WorkstatistikIterator::
next( kvalobs::kvWorkelement &ws )
{
   LogContext context( "WorkstatistikIterator::next" );

   if ( index == dataList->length() ) {
      LOGDEBUG( "Fetching data from kvalobs" );
      for ( int i = 0; ; i++ ) {
         try {
            bool ok = iter->next( dataList );
            index = 0;
            if ( dataList->length() == 0 or not ok ) {
               LOGDEBUG( "No more data available" );
               return false;
            }
            break;
         }
         catch( CORBA::TRANSIENT &e ) {
            if ( i < 2 ) {
               LOGWARN( "CORBA TRANSIENT exception - retrying..." );
               timespec ts;
               ts.tv_sec = 0;
               ts.tv_nsec = 500000000;
               nanosleep( &ts, NULL );
               continue;
            }
            else {
               LOGERROR( "CORBA TRANSIENT exception - giving up" );
               return false;
            }
         }
         catch( CORBA::Exception &e ) {
            LOGERROR( "Unhandled CORBA exception: " << typeid( e ).name() );
            return false;
         }
      }
   }
   assert( index < dataList->length() );

   const  WorkstatistikElem &rd = dataList[ index++ ];
   ws.set( rd.stationID,
           boost::posix_time::time_from_string( std::string(rd.obstime) ),
           rd.typeID_,
           boost::posix_time::time_from_string( std::string(rd.tbtime) ),
           rd.priority,
           boost::posix_time::time_from_string(std::string(rd.processStart)),
           boost::posix_time::time_from_string(std::string(rd.qaStart)),
           boost::posix_time::time_from_string(std::string(rd.qaStop)),
           boost::posix_time::time_from_string(std::string(rd.serviceStart)),
           boost::posix_time::time_from_string(std::string(rd.serviceStop)) );
   return true;
}


WorkstatistikIterator_var&
WorkstatistikIterator::
getCorbaObjPtr()
{
    return iter;
}

void
WorkstatistikIterator
::cleanup()
{
   LogContext context( "WorkstatistikIterator::cleanup" );

   dataList->length( 0 );
   index = 0;

   if ( not CORBA::is_nil( iter ) ) {
      LOGDEBUG( "Destroying CORBA iterator object" );
      try {
         iter->destroy();
         iter = CKvalObs::CService::WorkstatistikIterator::_nil();
      }
      catch(...) {
         LOGERROR( "Unable to destroy iterator object on server." );
      }
    }
  }
}

