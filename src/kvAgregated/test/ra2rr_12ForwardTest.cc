/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ra2rr_12ForwardTest.cc,v 1.1.2.3 2007/09/27 09:02:16 paule Exp $                                                       

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
#include <ra2rr_12.h>
#include <kvalobs/kvDataOperations.h>

using namespace kvalobs;
using namespace agregator;


class ra2rr_12ForwardTest : public AbstractAgregatorTest
{
protected:
    enum { RR_12 = 109, RA = 104 };
	ra2rr_12_forward agregator;
};

// This class does not follow the default time span of most other tests
//INSTANTIATE_TEST_CASE_P(ra2rr_12ForwardTest, AbstractAgregatorTest, testing::Values(AgregatorPtr(new ra2rr_12_forward)));

TEST_F(ra2rr_12ForwardTest, testGetTimeSpanAtGenerationPoint)
{
	const kvalobs::kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	const AbstractAgregator::TimeSpan timeSpan = 
	agregator.getTimeSpan(dataFactory.getData( 15, 1 ));
	EXPECT_EQ(miutil::miTime("2007-06-05 18:00:00"), timeSpan.first );
	EXPECT_EQ(miutil::miTime("2007-06-06 18:00:00"), timeSpan.second );
}


TEST_F(ra2rr_12ForwardTest, testGetTimeSpan)
{
	const kvalobs::kvDataFactory dataFactory( 42, "2007-06-06 03:00:00", 302 );
	const AbstractAgregator::TimeSpan timeSpan = 
	agregator.getTimeSpan(dataFactory.getData( 15, 1 ));
	EXPECT_EQ(miutil::miTime("2007-06-05 18:00:00"), timeSpan.first );
	EXPECT_EQ(miutil::miTime("2007-06-06 18:00:00"), timeSpan.second );
}

TEST_F(ra2rr_12ForwardTest, testExpressedInterest)
{
    const miutil::miDate d = miutil::miDate::today();
    miutil::miTime t( d, miutil::miClock( 6, 0, 0 ) );

    const kvDataFactory dataFactory( 42, t, 302 );

    ASSERT_TRUE( agregator.isInterestedIn( dataFactory.getData( 0, RA ) ) );

    // TODO: complete this test:
    //	t = miutil::miTime::nowTime();
    //	t.addHour( 1 );
    //	ASSERT_TRUE( not agregator.isInterestedIn( dataFactory.getData( 0, RA, t ) ) );
}

TEST_F(ra2rr_12ForwardTest, testNotEnoughData)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 213.3, RA ) );

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( not d.get() );
}

TEST_F(ra2rr_12ForwardTest, testDataMarkedAsMissing)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
    data.push_back( dataFactory.getMissing( RA, "2007-06-05 18:00:00" ) );
    data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) );

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( d.get() );
    ASSERT_TRUE( missing( * d ) );
}

TEST_F(ra2rr_12ForwardTest, test12hZero)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
    data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 18:00:00" ) );
    data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) );
    data.push_back( dataFactory.getData( 209.0, RA, "2007-06-04 18:00:00" ) ); // note obstime earliest

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( d.get() );
    EXPECT_EQ( miutil::miTime("2007-06-05 18:00:00"), d->obstime() );
    EXPECT_FLOAT_EQ( 0, d->corrected() );
}


TEST_F(ra2rr_12ForwardTest, test12hNegative)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
    data.push_back( dataFactory.getData( 209.0, RA, "2007-06-05 18:00:00" ) );
    data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) );
    data.push_back( dataFactory.getData( 209.0, RA, "2007-06-04 18:00:00" ) ); // note obstime earliest

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( d.get() );
    EXPECT_EQ( miutil::miTime("2007-06-05 18:00:00"), d->obstime() );
    EXPECT_FLOAT_EQ( 0, d->corrected() );
}


TEST_F(ra2rr_12ForwardTest, test12hPositive24hNegative)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
    data.push_back( dataFactory.getData( 211.2, RA, "2007-06-05 18:00:00" ) );
    data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) ); //unused
    data.push_back( dataFactory.getData( 214.4, RA, "2007-06-04 18:00:00" ) ); // note obstime earliest

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( d.get() );
    EXPECT_EQ( miutil::miTime("2007-06-05 18:00:00"), d->obstime() );
    EXPECT_FLOAT_EQ( 0, d->corrected() );
}


TEST_F(ra2rr_12ForwardTest, test12hPositive24hZero)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
    data.push_back( dataFactory.getData( 211.2, RA, "2007-06-05 18:00:00" ) );
    data.push_back( dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00" ) ); //unused
    data.push_back( dataFactory.getData( 211.2, RA, "2007-06-04 18:00:00" ) ); // note obstime earliest

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( d.get() );
    EXPECT_EQ( miutil::miTime("2007-06-05 18:00:00"), d->obstime() );
    EXPECT_FLOAT_EQ( 0, d->corrected() );
}


TEST_F(ra2rr_12ForwardTest, test12hPositive24hPositivePrev12hNegative)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 209.0, RA, "2007-06-05 06:00:00" ) );
    data.push_back( dataFactory.getData( 211.2, RA, "2007-06-05 18:00:00" ) );
    data.push_back( dataFactory.getData( 210.1, RA, "2007-06-04 18:00:00" ) ); // note obstime earliest

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( d.get() );
    EXPECT_EQ( miutil::miTime("2007-06-05 18:00:00"), d->obstime() );
    EXPECT_NEAR( 1.1, d->corrected(), .00001 );
}
		

TEST_F(ra2rr_12ForwardTest, test12hPositive24hPositivePrev12hZero)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 209.0, RA, "2007-06-05 06:00:00" ) );
    data.push_back( dataFactory.getData( 211.2, RA, "2007-06-05 18:00:00" ) );
    data.push_back( dataFactory.getData( 211.0, RA, "2007-06-04 18:00:00" ) ); // note obstime earliest

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( d.get() );
    EXPECT_EQ( miutil::miTime("2007-06-05 18:00:00"), d->obstime() );
    EXPECT_NEAR( 0.2, d->corrected(), .00001 );
}


TEST_F(ra2rr_12ForwardTest, test12hPositive24hPositivePrev12hPositive)
{
    AbstractAgregator::kvDataList data;
    const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
    data.push_back( dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00" ) );
    data.push_back( dataFactory.getData( 211.2, RA, "2007-06-05 18:00:00" ) );
    data.push_back( dataFactory.getData( 209.0, RA, "2007-06-04 18:00:00" ) ); // note obstime earliest

    AbstractAgregator::kvDataPtr d = agregator.process( data.front(), data );
    ASSERT_TRUE( d.get() );
    EXPECT_EQ( miutil::miTime("2007-06-05 18:00:00"), d->obstime() );
    EXPECT_NEAR( 1.1, d->corrected(), .00001 );
}


