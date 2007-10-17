/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ra2rr_12Test.cc,v 1.1.2.3 2007/09/27 09:02:16 paule Exp $                                                       

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
#include "ra2rr_12Test.h"
#include <ra2rr_12.h>
#include <kvalobs/kvDataOperations.h>
#include <iterator>

CPPUNIT_TEST_SUITE_REGISTRATION( ra2rr_12Test );

using namespace kvalobs;
using namespace agregator;

ra2rr_12Test::ra2rr_12Test()
{}

ra2rr_12Test::~ra2rr_12Test()
{}

void ra2rr_12Test::setUp()
{
	agregator = new agregator::ra2rr_12;
}

void ra2rr_12Test::tearDown()
{
	delete agregator;
	agregator = 0;
}

void ra2rr_12Test::testExpressedInterest()
{
	const miutil::miDate d = miutil::miDate::today();
	miutil::miTime t( d, miutil::miClock( 6,0,0 ) );
	
	const kvDataFactory dataFactory( 42, t, 302 );
	
	CPPUNIT_ASSERT( agregator->isInterestedIn( dataFactory.getData( 0, RA ) ) );

	// TODO: complete this test:
//	t = miutil::miTime::nowTime();
//	t.addHour( 1 );
//	CPPUNIT_ASSERT( not agregator->isInterestedIn( dataFactory.getData( 0, RA, t ) ) );
}

void ra2rr_12Test::testNotEnoughData()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 213.3, RA ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( not d.get() );
}

void ra2rr_12Test::testCompleteDataObservationInMiddle()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	
	miutil::miTime t("2007-06-05 06:00:00");
	for ( int i = 0; i < 13; ++ i )
	{
		data.push_back( dataFactory.getData( i, RA, t ) );
		t.addHour();
	}
	
	AbstractAgregator::kvDataList::const_iterator randomElement = data.begin();
	advance(randomElement, 4);

	AbstractAgregator::kvDataPtr d = agregator->process( * randomElement, data );

	CPPUNIT_ASSERT( ! d.get() );
}

void ra2rr_12Test::testNonStandardDataSet()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	for ( miutil::miTime t = "2007-06-06 07:00:00"; t < "2007-06-06 19:00:00"; t.addHour() )
		data.push_back( dataFactory.getData( 100, RA, t ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( ! d.get() );
}


void ra2rr_12Test::testDataMarkedAsMissing()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
	data.push_back( dataFactory.getMissing( RA, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	CPPUNIT_ASSERT( missing( * d ) );
}



void ra2rr_12Test::testStandardAgregation()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
	data.push_back( dataFactory.getData( 211.2, RA, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 2.1, d->corrected(), .00001 );
}

void ra2rr_12Test::testResultIsZero()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
	data.push_back( dataFactory.getData( 213.3, RA, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, d->corrected(), .00001 );
}


void ra2rr_12Test::testResultIsBelowZero()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
	data.push_back( dataFactory.getData( 213.5, RA, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, d->corrected(), .00001 );
}

void ra2rr_12Test::testResultIsMuchBelowZero()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
	data.push_back( dataFactory.getData( 213.5, RA, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 13.3, RA, "2007-06-06 06:00:00" ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, d->corrected(), .00001 );
}

void ra2rr_12Test::test12hPositive24hZeroAt06()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 211.2, RA, "2007-06-05 06:00:00" ) );
	data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 211.2, RA, "2007-06-06 06:00:00" ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, d->corrected(), .00001 );
}

void ra2rr_12Test::test12hPositive24hZeroAt18()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 18:00:00", 302 );
	data.push_back( dataFactory.getData( 211.2, RA, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 210.1, RA, "2007-06-06 06:00:00" ) );
	data.push_back( dataFactory.getData( 211.2, RA, "2007-06-06 18:00:00" ) );
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.1, d->corrected(), .00001 );	
}

