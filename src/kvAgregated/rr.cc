/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: rr.cc,v 1.1.2.6 2007/09/27 09:02:16 paule Exp $                                                       

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
#include "rr.h"
#include "times.h"
#include "GenerateZero.h"
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvObsPgm.h>
#include <milog/milog.h>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <vector>

#include <boost/thread/thread.hpp>

using namespace std;
using namespace miutil;
using namespace kvalobs;
using namespace kvservice;
using namespace boost;

namespace agregator
{
  // rr_1:

  rr_1::rr_1()
    : AbstractAgregator( RR_01, RR_1, 1, allHours )
    , threadStopping( false )
  {
    GenerateZero g0( *this );
    thread = new boost::thread( g0 );
  }

  rr_1::~rr_1()
  {
    LOGDEBUG("Stopping RR_1 autogeneration");
    threadStopping = true;
    thread->join();
    delete thread;
  }

  bool rr_1::shouldProcess( const kvData &trigger,
			    const kvDataList &observations )
  {
    //return true;
    // Will only generate when receiving values from GenerateZero thread
    bool generate = ( trigger.original() == GenerateZero::obsVal() );
    if ( not generate ) {
        const miTime & obstime = trigger.obstime();
        generate = obstime.date() != miDate::today() or
            ( obstime.clock() <= miClock( 6,0,0 ) and miClock::oclock() > GenerateZero::genClock );
    }
    return generate;
  }


  float rr_1::generateKvData( const kvDataList &data, 
			      const kvData &trigger )
  {
    float sum = 0;
    for ( kvDataList::const_iterator it = data.begin();
	  it != data.end();  it++ ) {
      if ( not valid( * it ) )
	return invalidParam;
      if ( it->corrected() > 0 )
	sum += it->corrected();
    }
    return sum;
  }


  // rr:

  rr::rr( int readParam, int writeParam, int interestingHours, 
	 const set<miClock> &generateWhen )
    : AbstractAgregator( readParam, writeParam, interestingHours, generateWhen )
  {
  }

  float rr::generateKvData( const kvDataList &data, 
			    const kvData &trigger )
  {
    bool nothing = true;
    float sum = 0;
    for ( kvDataList::const_iterator it = data.begin();
	  it != data.end();  it++ ) {
      if ( not valid( * it ) )
	return invalidParam;
      if ( it->corrected() >= 0 ) {
	sum += it->corrected();
	nothing = false;
      }
    }    
    if ( nothing )
      return -1;
    return sum;
  }


  // rr_12:

  rr_12::rr_12()
    : rr( RR_1, RR_12, 12, sixAmSixPm)
  {}


  // rr_24:

  rr_24::rr_24()
    : rr( RR_12, RR_24, 24, sixAm)
  {
  }


  bool rr_24::shouldProcess( const kvData &trigger,
			     const kvDataList &observations )
  {
    // These are the times from which we will generate data:
    const set<miClock> &when = sixAmSixPm;

    const miTime &time = trigger.obstime();
    if ( when.find( time.clock() ) == when.end() )
      return false;


    for ( set<miClock>::const_iterator genTime = when.begin();
	  genTime != when.end();  genTime++) {
      kvDataList::const_iterator search = observations.begin();
      while ( search != observations.end() ) {
	const miTime &t = search->obstime();
	if ( t.clock() == *genTime ) 
	  break;
	search++;
      }
      if ( search == observations.end() )
	return false;
    }
    return true;
  }

  
  float rr_24::generateKvData( const kvDataList &data, 
			       const kvData &trigger )
  {
    if ( not valid( trigger ) )
      return invalidParam;

    const set<miClock> &when = sixAmSixPm;
    kvDataList relevant;

    for ( set<miClock>::const_iterator it = when.begin();
	  it != when.end();  it++) {
      for ( kvDataList::const_iterator dataIt = data.begin();
	    dataIt != data.end();  dataIt++ ) {
	miTime t = dataIt->obstime();
	if ( t.clock() == *it )
	  relevant.push_back( *dataIt );
      }
    }
    return rr::generateKvData( relevant, trigger );
  }
}
