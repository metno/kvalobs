/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: times.h,v 1.1.2.3 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef __agregator__times_h__
#define __agregator__times_h__

namespace agregator {
  static const miutil::miClock hours[24] = {
    miutil::miClock( 0,0,0),  miutil::miClock(12,0,0),
    miutil::miClock( 1,0,0),  miutil::miClock(13,0,0),
    miutil::miClock( 2,0,0),  miutil::miClock(14,0,0),
    miutil::miClock( 3,0,0),  miutil::miClock(15,0,0),
    miutil::miClock( 4,0,0),  miutil::miClock(16,0,0),
    miutil::miClock( 5,0,0),  miutil::miClock(17,0,0),
    miutil::miClock( 6,0,0),  miutil::miClock(18,0,0),
    miutil::miClock( 7,0,0),  miutil::miClock(19,0,0),
    miutil::miClock( 8,0,0),  miutil::miClock(20,0,0),
    miutil::miClock( 9,0,0),  miutil::miClock(21,0,0),
    miutil::miClock(10,0,0),  miutil::miClock(22,0,0),
    miutil::miClock(11,0,0),  miutil::miClock(23,0,0)
  };
  const std::set<miutil::miClock> allHours( hours, &hours[24] );
  const std::set<miutil::miClock> sixAmSixPm( &hours[12], &hours[14] );
  const std::set<miutil::miClock> sixAm( &hours[12], &hours[13] );
}

#endif // __agregator__times_h__
