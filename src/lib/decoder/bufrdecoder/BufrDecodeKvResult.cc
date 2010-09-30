/*
 * BufrDecodeKvResult.h
 *
 *  Created on: Sep 29, 2010
 *      Author: borgem
 */

#include <iostream>
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
setStationid( int wmono )
{
   stationid_ = findStationid( wmono );
}

void
BufrDecodeKvResult::
add( float value, int kvparamid, int sensor, int level )
{
   cerr << "KvData: " << kvparamid << " value: " << value << " s: " << sensor << " l: " << level << endl;
   if( stationid_ == INT_MAX )
      return;

   if( obstime_.undef() )
      return;

   if( value == FLT_MAX )
      return;

   data_.push_back( kvalobs::kvData( stationid_, obstime_, value, kvparamid, tbtime_, typeid_, sensor, level, value,
                                    kvalobs::kvControlInfo(), kvalobs::kvUseInfo(), "" ) );

}

}
}
}

