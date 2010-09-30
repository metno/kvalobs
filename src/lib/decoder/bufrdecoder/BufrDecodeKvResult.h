/*
 * BufrDecodeKvResult.h
 *
 *  Created on: Sep 29, 2010
 *      Author: borgem
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

class BufrDecodeKvResult : public BufrDecodeResultBase
{
   KvStationList_ptr stationList_;
   std::list<kvalobs::kvData> data_;
   miutil::miTime obstime_;
   miutil::miTime tbtime_;
   int wmono_;
   int stationid_;
   int typeid_;
   double latitude_;
   double longitude_;

   int findStationid( int wmono );

public:
   BufrDecodeKvResult():
      tbtime_( miutil::miTime::nowTime() ), wmono_(INT_MAX), stationid_( INT_MAX ), typeid_( 7 ),
      latitude_( DBL_MAX ), longitude_( DBL_MAX ) {}

   void setKvStationList( KvStationList_ptr stationlist ) { stationList_ = stationlist; }
   virtual void setObstime( const miutil::miTime &obstime ){ obstime_ = obstime; }
   virtual miutil::miTime getObstime()const { return obstime_; }
   virtual void setStationid( int wmono );
   virtual void setLatLong( double latitude, double longitude ) { latitude_ = latitude; longitude_ = longitude; }
   void add( float value, int kvparamid, int sensor=0, int level=0 );
};

}
}
}


#endif
