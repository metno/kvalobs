/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GenerateZero.cc,v 1.1.2.5 2007/09/27 09:02:15 paule Exp $                                                       

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
#include "GenerateZero.h"
#include "AgregatorHandler.h"
#include "rr.h"
#include <algorithm>
#include <sstream>
#include <kvalobs/kvObsPgm.h>
#include <kvcpp/KvApp.h>
#include <milog/milog.h>
#include <kvalobs/kvDataOperations.h>
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>


using namespace std;
using namespace kvalobs;
using namespace kvservice;
using namespace miutil;
using namespace milog;
using namespace boost;

namespace agregator
{
  GenerateZero::GenerateZero( rr_1 &ag ) 
    : ag(ag)
    , nextGen( miDate::today(), genClock )
    , nextGetObsPgm( miDate::today(), getObsPgmClock )
  {
  }


  GenerateZero::~GenerateZero()
  {
  }


  void GenerateZero::operator()()
  {
    milog::LogContext context( "RR_1 autogenerate" );
    LOGINFO("Started RR_1 autogeneration");

    try {
    while ( not ag.threadIsStopping() ) {
      miTime now = miTime::nowTime();
      if ( now > nextGetObsPgm ) {
    	try {
    	  fillObsPgm();
    	}
    	catch ( runtime_error & e ) {
    	  LOGERROR( e.what() );
    	}
    	nextGetObsPgm.addDay();
      }
      if ( now > nextGen ) 
      {
		generate();
    	nextGen.addDay();
      }
      xtime xt;
      xtime_get( &xt, TIME_UTC );
      xt.sec += 5;
      ::boost::thread::sleep( xt );
    }
    LOGDEBUG("RR_1 autogeneration stopped");
    
    }
    catch( std::exception & e ) {
        LOGFATAL( e.what() );
        assert( 0 );
    }
  }

  
  void GenerateZero::generate()
  {
	LOGINFO("Generating new RR_1");
	
	miTime to( nextGen.date(), miClock( 7,0,0 ) );
    miTime from( nextGen.date(), miClock( 6,0,0 ) );
    
	for ( from.addDay( -1 ); from < to; from.addHour() ) 
	{
	  for ( vector<StData>::const_iterator it = stations.begin(); it != stations.end();  it++ ) 
	  {
	    LOGDEBUG( "Station " << it->station );
	    
	    AgregatorHandler * ag = AgregatorHandler::agHandler;
	    if ( ! ag )
	    	return;
	    	
	    kvDataFactory dataFactory( it->station, from, it->type, it->sensor, it->lvl );
		ag->process( dataFactory.getData( obsVal(), RR_01 ) );
	  }
	}
  }


  void GenerateZero::fillObsPgm()
  {
    LOGDEBUG( "Fetching obs_pgm from kvalobs" );

    if ( ! KvApp::kvApp )
      throw runtime_error( "Unable to access KvApp - cannot get obs_pgm." );

    list<kvObsPgm> obspgm;
    list<long> unused;

    bool result = KvApp::kvApp->getKvObsPgm( obspgm, unused, false );
    if ( ! result )
      throw runtime_error( "Could not fetch obs_pgm from kvalobs!" );

    StDataContainer newData;
    
    const miutil::miTime now = miutil::miTime::nowTime();
    
    for ( list<kvObsPgm>::const_iterator it = obspgm.begin(); it != obspgm.end(); it++ )
      if ( it->paramID() == RR_01 && it->fromtime() <  now && now < it->totime() )
        newData.push_back( *it );

    if ( stations != newData )
      stations = newData;

    ostringstream ss;
    ss << "Stasjoner med RR_01";
    for ( StDataContainer::const_iterator it = stations.begin();
	  it != stations.end();  ++it )
      ss << ": " << it->station;
    LOGDEBUG( ss.str() );
  }
  

  const miClock GenerateZero::genClock( 7, 20, 0 );
  const miClock GenerateZero::getObsPgmClock( 6, 50, 0 );


  GenerateZero::StData::StData( const kvObsPgm & p )
    : station( p.stationID() )
    , type( p.typeID() )
    , sensor( '0' ) // p.nr_sensor()
    , lvl( p.level() )
  {}
  
  GenerateZero::StData::StData( int station, int type, int sensor, int level )
    : station( station )
    , type( type )
    , sensor( sensor )
    , lvl( level )
  {}


  bool GenerateZero::StData::operator<( const StData &d ) const {
    return station < d.station
      or type < d.type
      or sensor < d.sensor
      or lvl < d.lvl;
  }


  bool GenerateZero::StData::operator==( const StData &d ) const {
    return station == d.station
      and type == d.type
	  and sensor == d.sensor
      and lvl == d.lvl;
  }
}
