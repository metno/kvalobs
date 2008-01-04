/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: MinMaxTest.cc,v 1.1.2.2 2007/09/27 09:02:16 paule Exp $                                                       

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
#include "MinMaxTest.h"
#include <minmax.h>
#include <times.h>
#include <kvalobs/kvDataOperations.h>
#include <algorithm>
#include <iterator>

CPPUNIT_TEST_SUITE_REGISTRATION( MinMaxTest );

using namespace kvalobs;
using namespace agregator;

MinMaxTest::MinMaxTest()
{
}

MinMaxTest::~MinMaxTest()
{
}

void MinMaxTest::setUp()
{
	agregator = new MinMax( 1, 2, 12, sixAmSixPm, std::min<float> );
}

void MinMaxTest::testNormal()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
//	data.push_back( dataFactory.getData( 2, 1, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-05 19:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-05 20:00:00" ) );
	data.push_back( dataFactory.getData( 3, 1, "2007-06-05 21:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-05 22:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-05 23:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-06 00:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-06 01:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-06 02:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-06 03:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-06 06:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-06 05:00:00" ) );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-06 06:00:00" ) );
	
	AbstractAgregator::kvDataList::const_iterator p = data.begin();
	++p;
	
	AbstractAgregator::kvDataPtr d = agregator->process( *p, data );
	CPPUNIT_ASSERT( d.get() );
	
	CPPUNIT_ASSERT_EQUAL( 2, d->paramID() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 3, d->corrected(), 0.0005 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 3, d->original(), 0.0005 );
}

void MinMaxTest::testIncompleteData()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
//	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 19:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 20:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 21:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 22:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 23:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 00:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 01:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 02:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 03:00:00" ) );
	data.push_back( dataFactory.getMissing( 1, "2007-06-06 04:00:00" ) ); // <- Here
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 05:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 06:00:00" ) );

	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	
	CPPUNIT_ASSERT_EQUAL( 2, d->paramID() );
	CPPUNIT_ASSERT( not valid( * d ) );	
}

void MinMaxTest::testMissingRow()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
//	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 3, 1, "2007-06-05 19:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 20:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 21:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 22:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 23:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 00:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 01:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 02:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 03:00:00" ) );
//	data.push_back( dataFactory.getMissing( 1, "2007-06-06 04:00:00" ) ); // <- Here
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 05:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 06:00:00" ) );

	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( ! d.get() );
}


void MinMaxTest::testWrongInputDates()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 1, 1, "2007-06-02 18:00:00" ) ); // <- this should be ignored
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 19:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 20:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 21:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 22:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-05 23:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 00:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 01:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 02:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 03:00:00" ) );
//	data.push_back( dataFactory.getMissing( 1, "2007-06-06 04:00:00" ) ); // <- Here one is missing
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 05:00:00" ) );
	data.push_back( dataFactory.getData( 4, 1, "2007-06-06 06:00:00" ) );

	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( ! d.get() );
//	CPPUNIT_ASSERT_DOUBLES_EQUAL(4, d->original(), 0.0005);
//	CPPUNIT_ASSERT_DOUBLES_EQUAL(4, d->corrected(), 0.0005);
}


void MinMaxTest::testCompleteDataObservationInMiddle()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
//	data.push_back( dataFactory.getData( 12, 1, "2007-06-05 18:00:00" ) );
	data.push_back( dataFactory.getData( 11, 1, "2007-06-05 19:00:00" ) );
	data.push_back( dataFactory.getData( 10, 1, "2007-06-05 20:00:00" ) );
	data.push_back( dataFactory.getData( 9, 1, "2007-06-05 21:00:00" ) );
	data.push_back( dataFactory.getData( 8, 1, "2007-06-05 22:00:00" ) );
	data.push_back( dataFactory.getData( 7, 1, "2007-06-05 23:00:00" ) );
	data.push_back( dataFactory.getData( 6, 1, "2007-06-06 00:00:00" ) );
	data.push_back( dataFactory.getData( 5, 1, "2007-06-06 01:00:00" ) );
	data.push_back( dataFactory.getData( 4, 1, "2007-06-06 02:00:00" ) );
	data.push_back( dataFactory.getData( 3, 1, "2007-06-06 03:00:00" ) );
	data.push_back( dataFactory.getData( 2, 1, "2007-06-06 06:00:00" ) );
	data.push_back( dataFactory.getData( 1, 1, "2007-06-06 05:00:00" ) );
	data.push_back( dataFactory.getData( 0, 1, "2007-06-06 06:00:00" ) );
	
	AbstractAgregator::kvDataList::const_iterator randomElement = data.begin();
	std::advance( randomElement, 4 );
	
	AbstractAgregator::kvDataPtr d = agregator->process( * randomElement, data );
	CPPUNIT_ASSERT( d.get() );
	
	CPPUNIT_ASSERT_EQUAL( 2, d->paramID() );
	
	// This is a previous bug we are checking for:
	CPPUNIT_ASSERT( d->corrected() != randomElement->corrected() );
	
	// This is the correct answer:
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, d->corrected(), 0.0005 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, d->original(), 0.0005 );
}
