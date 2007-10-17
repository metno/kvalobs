/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ra2rr_12.cc,v 1.1.2.9 2007/09/27 09:02:16 paule Exp $                                                       

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
#include "ra2rr_12.h"
#include <kvalobs/kvDataOperations.h>
#include <milog/milog.h>
#include "times.h"

using namespace std;
using namespace miutil;
using namespace kvalobs;

namespace agregator
{
  ra2rr_12::ra2rr_12( )
  	: AbstractAgregator( RA, RR_12, 24, sixAmSixPm)
  {}

  namespace
  {
  	struct has_obstime
  	{
  		const miutil::miTime obstime;
  		has_obstime( const miutil::miTime & t ) : obstime( t ) {}
  		bool operator () ( const kvData & d ) const
  		{
  			return d.obstime() == obstime;
  		}
  	};
  	
  	template<int clock>
  	bool hasObsHour( const kvData & d )
  	{
  		return clock == d.obstime().hour();
  	}
  	
  	bool hasObsHourSix( const kvData & d )
  	{
  		int hour = d.obstime().hour();
  		return 6 == hour or 18 == hour; 
  	}
  	
  	struct lt_obstime
  	{
  		bool operator () ( const kvData & a, const kvData & b ) const
  		{
  			return a.obstime() < b.obstime();
  		}
  	};
  	
  }


  bool ra2rr_12::shouldProcess( const kvData & trigger, const kvDataList & observations )
  {
	  const set<miClock> & gw = generateWhen();
    if ( observations.size() > 1 and
         gw.find( trigger.obstime().clock() ) != gw.end() and
         find_if( observations.begin(), observations.end(), hasObsHour<6> ) != observations.end() and
         find_if( observations.begin(), observations.end(), hasObsHour<18> ) != observations.end() )
      return true;
    return false;
  }
    

  float ra2rr_12::generateKvData( const kvDataList &data, const kvData & trigger )
  {
    if ( ! valid( trigger ) )
      return invalidParam;

	const kvData * to = & trigger; 
      
    miTime t = to->obstime();
    t.addHour( timeOffset() );
    
	kvDataList::const_iterator find = find_if( data.begin(), data.end(), has_obstime( t ) );
	if ( find == data.end() )
		throw runtime_error( "Could not find other relevant RA observation" );
	const kvData * from = &* find;
		
	if ( from->obstime() > to->obstime() )
		swap( from, to );
		
    t = to->obstime();
	t.addDay( -1 );
	kvDataList::const_iterator oneDayAgo = find_if( data.begin(), data.end(), has_obstime( t ) );
		
	
	return agregate( * from, * to, oneDayAgo == data.end() ? 0 : &* oneDayAgo );  
  }

  float ra2rr_12::agregate( const kvData & from, const kvData & to, const kvData * oneDayAgo ) const
  {
    if ( not ( valid( from ) and valid( to ) ) )
      return invalidParam;
    
    // Do calculation
    float result = to.corrected() - from.corrected();

	// Correct false precipitation 
	// - if  rain for 18:00 - 06:00 > 0
	//   and rain for 06:00 - 06:00 <= 0
	//   then return -1
	if ( oneDayAgo and result > 0 and to.obstime().clock().hour() == 6)
	{
		float otherResult = to.corrected() - oneDayAgo->corrected();
		if ( otherResult < 0.0001 )
			result = otherResult;
	}

    LOGDEBUG( "Calculation sum: " << result );
    
    // Make neccessary adjustments (values <= 0 is set to -1):
    if ( result < 0.0001 )
      return 0;
      
    return result;  	
  }



  // ra2rr_12_forward only:

  ra2rr_12_forward::ra2rr_12_forward()
  {
    name += "-Forward";
  }

  const pair<miTime, miTime> ra2rr_12_forward::getTimeSpan( const kvData &data )
  {
    miDate date = data.obstime().date();
    set<miClock>::const_iterator it =
        generateWhen().lower_bound( data.obstime().clock() );
    if ( it == generateWhen().end() ) {
      it = generateWhen().begin();
      date.addDay();
    }
    miTime startTime( date, *it );
    startTime.addSec( -1 );
    miTime genTime = startTime;
    genTime.addHour( interestingHours() );

    const pair<miTime, miTime> ret(startTime, genTime);
    
    LOGDEBUG( "Time span: " << ret.first << " - " << ret.second );

    return ret;
  }
}
