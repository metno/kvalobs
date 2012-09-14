/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: AbstractAgregator.cc,v 1.1.2.9 2007/09/27 09:02:15 paule Exp $                                                       

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
#include "AbstractAgregator.h"
//#include "AgregatorHandler.h"
#include <kvalobs/kvDataOperations.h>
#include <puTools/miTime>
#include <puTools/miString>
#include <milog/milog.h>
#include <decodeutility/kvDataFormatter.h>
#include <sstream>
#include <vector>
#include <list>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <boost/lexical_cast.hpp>

//#define NDEBUG
#include <cassert>

using namespace std;
using namespace kvservice;
using namespace kvalobs;
using namespace miutil;
using namespace dnmi::db;
using namespace milog;
using namespace boost;

namespace agregator
{
  AbstractAgregator::AbstractAgregator( int readParam, int writeParam,
                                        int interestingHours,
                                        const set<miClock> &generateWhen )
        : name( "Agregator(" + lexical_cast<string>( readParam )
                + ", " + lexical_cast<string>( writeParam ) + ")" )
        , read_param( readParam )
        , write_param( writeParam )
        , interesting_hours( interestingHours )
        , generate_when( generateWhen )
  {}


  AbstractAgregator::~AbstractAgregator()
  {}

  const AbstractAgregator::TimeSpan
  AbstractAgregator::getTimeSpan( const kvData & data ) const
  {
    // Find out what times of day we are interested in:
    miTime time = data.obstime();
    miDate date = time.date();
    set<miClock>::const_iterator it =
        generate_when.lower_bound( time.clock() );
    if ( it == generate_when.end() )
    {
      it = generate_when.begin();
      date.addDay();
    }
    miTime genTime( date, *it );
    miTime startTime = genTime;
    startTime.addHour( - interesting_hours );

    const TimeSpan ret( startTime, genTime );

    return ret;
  }


  bool AbstractAgregator::shouldProcess( const kvalobs::kvData &trigger,
                                         const kvDataList &observations )
  {
    if ( ( int ) observations.size() < interesting_hours )
      return false;
    return true;
  }


  kvalobs::kvData
  AbstractAgregator::getDataObject( const kvData &trigger,
                                    const miTime &obsTime,
                                    float agregateValue )
  {
    int typeID = trigger.typeID();
    if ( typeID > 0 )
      typeID *= -1;

    kvDataFactory f( trigger.stationID(), obsTime, typeID, trigger.sensor(), trigger.level() );

    if ( agregateValue == invalidParam )
      return f.getMissing( write_param );

    return f.getData( agregateValue, write_param );
  }

  
  	bool AbstractAgregator::isInterestedIn( const kvalobs::kvData &data ) const
  	{
		// Are we still supposed to run?
		if ( KvApp::kvApp )
		  if ( KvApp::kvApp->shutdown() )
			return false;
		
		LogContext context( name + " Station=" + lexical_cast<string>( data.stationID() ) );
		
		// What time range should we use as base data?
		TimeSpan times = getTimeSpan( data );
		
		// Immediatly return if we obviously are supposed to agregate
		if ( data.obstime().hour() != 6 and data.obstime().hour() != 18 )
		{
		  miTime t = miTime::nowTime();
		  t.addMin( 30 );
		  if ( data.obstime() < t )
		  {
		    t.addHour( 2 );
		    if ( times.second > t )
		    {
		      return false;
		    }
		  }
		}
		return true;
  	}
  	
	std::auto_ptr<kvalobs::kvData> AbstractAgregator::process( const kvalobs::kvData & data, const std::list<kvalobs::kvData> & observations )
	{
	  typedef std::auto_ptr<kvalobs::kvData> return_type;
		
	    if ( not shouldProcess( data, observations ) )
	    {
	      LOGDEBUG( "Will not process" );
	      return return_type( 0 );
	    }
	    
	    LOGINFO( "Agregating with base " << data );
	
	    // Call abstract method to get agregate value:
	    float agregateValue;
	    try
	    {
	      agregateValue = generateKvData( observations, data );
	    }
	    catch ( std::exception & err )
	    {
	      if ( err.what() [ 0 ] != '\0' )
	      {
	        LOGERROR( err.what() );
	      }
	      return return_type( 0 );
	    }
	    catch ( ... )
	    {
	      LOGERROR( "Unrecognized error" );
	      return return_type( 0 );
	    }
	
	    TimeSpan times = getTimeSpan( data );
	    
	    // Create a data object for saving
	    miTime t = miTime( times.second.date(), miClock( times.second.hour(), 0, 0 ) );

		return return_type( new kvData( getDataObject( data, t, agregateValue ) ) ); 
	}

}
