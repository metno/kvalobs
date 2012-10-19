/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: comobsentry.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $

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

#include <iostream>
#include <milog/milog.h>
#include "BufrDecodeKvResult.h"

namespace kvalobs {
namespace decoder {
namespace bufr {

using namespace std;

int
BufrDecodeKvResult::
findStationid( int wmono )
{
   if( wmono != wmono_ || stationid_ == INT_MAX ) {
      if( stationList_ ) {
         std::list<kvalobs::kvStation>::iterator it = stationList_->begin();

         for( ;it != stationList_->end(); ++it ) {
            if( it->wmonr() == wmono )
               break;
         }

         wmono_ = wmono;

         if( it == stationList_->end() )
            stationid_ = INT_MAX;
         else
            stationid_ = it->stationID();
      }
   }

   return stationid_;
}

void
BufrDecodeKvResult::
clear()
{
   obstime_ = miutil::miTime(); //Undef
   wmono_ = INT_MAX;
   stationid_ = INT_MAX;
   latitude_ = FLT_MAX;
   longitude_ = FLT_MAX;
   data_.clear();
}

void
BufrDecodeKvResult::
setStationid( int wmono )
{
   stationid_ = findStationid( wmono );
}

void
BufrDecodeKvResult::
add( float value, int kvparamid, const miutil::miTime &obstime )
{
   if( value == FLT_MAX )
      return;

   if( obstime_.undef() )
      obstime_ = obstime;


   data_.push_back( Data( value , kvparamid, obstime ) );
}


}
}
}

