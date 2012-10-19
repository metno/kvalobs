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

#ifndef __BUFRDECODEKVRESULT_H__
#define __BUFRDECODEKVRESULT_H__

#include <list>
#include <boost/shared_ptr.hpp>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStation.h>
#include <puTools/miTime.h>
#include "BufrDecodeBase.h"

namespace kvalobs {
namespace decoder {
namespace bufr {

typedef boost::shared_ptr<std::list<kvalobs::kvStation> > KvStationList_ptr;

class BufrDecoder;

class BufrDecodeKvResult : public BufrDecodeResultBase
{
protected:
   struct Data {
      int paramid;
      float value;
      miutil::miTime obstime;

      Data( float val, int paramid, const miutil::miTime &obst )
         : paramid( paramid ), value( val ), obstime( obst ) {}
   };

   KvStationList_ptr stationList_;
   std::list<Data> data_;
   std::map<int, miutil::miTime> stationidTbTimeList;
   miutil::miTime obstime_;
   int wmono_;
   int stationid_;
   int typeid_;
   float latitude_;
   float longitude_;

   int findStationid( int wmono );

public:
   BufrDecodeKvResult():
      wmono_(INT_MAX), stationid_( INT_MAX ), typeid_( 7 ),
      latitude_( FLT_MAX ), longitude_( FLT_MAX ) {}

   void clear();
   void setKvStationList( KvStationList_ptr stationlist ) { stationList_ = stationlist; }
   virtual void setObstime( const miutil::miTime &obstime ){ obstime_ = obstime; }
   virtual miutil::miTime getObstime()const { return obstime_; }
   virtual void setStationid( int wmono );
   virtual void setLatLong( double latitude, double longitude ) { latitude_ = latitude; longitude_ = longitude; }
   virtual void add( float value, int kvparamid, const miutil::miTime &obstime );


};

}
}
}


#endif
