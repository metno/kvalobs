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

#include "AbstractAgregatorTest.h"
#include <minmax.h>
#include <times.h>
#include <kvalobs/kvDataOperations.h>
#include <algorithm>
#include <iterator>

using namespace kvalobs;
using namespace agregator;

class MinMaxTest : public AbstractAgregatorTest
{
protected:
	MinMax agregatorToTest;
	MinMaxTest() : agregatorToTest(1, 2, 12, sixAmSixPm, std::min<float>) {}
};

INSTANTIATE_TEST_CASE_P(MinMaxTest, AbstractAgregatorTest, testing::Values(AgregatorPtr(new MinMax(1, 2, 12, sixAmSixPm, std::min<float>))));

TEST_F(MinMaxTest, testNormal)
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
	
	AbstractAgregator::kvDataPtr d = agregatorToTest.process( *p, data );
	ASSERT_TRUE( d.get() );
	
	EXPECT_EQ( 2, d->paramID() );
	EXPECT_FLOAT_EQ( 3, d->corrected() );
	EXPECT_FLOAT_EQ( 3, d->original() );
}

TEST_F(MinMaxTest, testIncompleteData)
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

	AbstractAgregator::kvDataPtr d = agregatorToTest.process( data.back(), data );
	ASSERT_TRUE(d.get());
	
	EXPECT_EQ( 2, d->paramID() );
	ASSERT_TRUE( not valid( * d ) );
}

TEST_F(MinMaxTest, testMissingRow)
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

	AbstractAgregator::kvDataPtr d = agregatorToTest.process( data.back(), data );
	ASSERT_TRUE( ! d.get() );
}


TEST_F(MinMaxTest, testWrongInputDates)
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

	AbstractAgregator::kvDataPtr d = agregatorToTest.process( data.back(), data );
	ASSERT_TRUE( ! d.get() );
//	EXPECT_FLOAT_EQ(4, d->original());
//	EXPECT_FLOAT_EQ(4, d->corrected());
}


TEST_F(MinMaxTest, testCompleteDataObservationInMiddle)
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
	
	AbstractAgregator::kvDataPtr d = agregatorToTest.process( * randomElement, data );
	ASSERT_TRUE(d.get());
	
	EXPECT_EQ( 2, d->paramID() );
	
	// This is a previous bug we are checking for:
	ASSERT_TRUE( d->corrected() != randomElement->corrected() );
	
	// This is the correct answer:
	EXPECT_FLOAT_EQ( 0, d->corrected());
	EXPECT_FLOAT_EQ( 0, d->original());
}
