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
#include <times.h>
#include <kvalobs/kvDataOperations.h>
#include <algorithm>

CPPUNIT_TEST_SUITE_REGISTRATION( MinMaxTest );

using namespace kvalobs;
using namespace agregator;

MinMaxTest::MinMaxTest()
	: agregator( 0 )
{
}

MinMaxTest::~MinMaxTest()
{
}

void MinMaxTest::setUp()
{
	agregator = new MinMax( 1, 2, 12, sixAmSixPm, std::min<float> );
}

void MinMaxTest::tearDown()
{
	delete agregator;
	agregator = 0;
}

void MinMaxTest::testNormal()
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	data.push_back( dataFactory.getData( 15, 1, "2007-06-05 18:00:00" ) );
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
	
	AbstractAgregator::kvDataPtr d = agregator->process( data.back(), data );
	CPPUNIT_ASSERT( d.get() );
	
	CPPUNIT_ASSERT_EQUAL( 2, d->paramID() );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 3, d->corrected(), 0.0005 );	
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
