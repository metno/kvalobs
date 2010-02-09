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

#include "AbstractAgregatorTest.h"
#include <ra2rr_12.h>
#include <kvalobs/kvDataOperations.h>
#include <iterator>

using namespace kvalobs;
using namespace agregator;


class ra2rr_12Test : public AbstractAgregatorTest
{
protected:
    enum { RR_12 = 109, RA = 104 };
    ra2rr_12 agregator;
};

INSTANTIATE_TEST_CASE_P(ra2rr_12Test, AbstractAgregatorTest, testing::Values(AgregatorPtr(new ra2rr_12)));

TEST_F(ra2rr_12Test, testExpressedInterest)
{
	const miutil::miDate d = miutil::miDate::today();
	miutil::miTime t(d, miutil::miClock( 6, 0, 0) );

	const kvDataFactory dataFactory( 42, t, 302);

	ASSERT_TRUE(agregator.isInterestedIn(dataFactory.getData( 0, RA) ) );

	// TODO: complete this test:
	//	t = miutil::miTime::nowTime();
	//	t.addHour( 1 );
	//	ASSERT_TRUE( not agregator.isInterestedIn( dataFactory.getData( 0, RA, t ) ) );
}

TEST_F(ra2rr_12Test, testNotEnoughData)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 213.3, RA) );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE( not d.get() );
}

TEST_F(ra2rr_12Test, testCompleteDataObservationInMiddle)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);

	miutil::miTime t("2007-06-05 06:00:00");
	for (int i = 0; i < 13; ++i)
	{
		data.push_back(dataFactory.getData(i, RA, t) );
		t.addHour();
	}

	AbstractAgregator::kvDataList::const_iterator randomElement = data.begin();
	advance(randomElement, 4);

	AbstractAgregator::kvDataPtr d = agregator.process( *randomElement, data);

	ASSERT_TRUE( !d.get() );
}

TEST_F(ra2rr_12Test, testNonStandardDataSet)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	for (miutil::miTime t = "2007-06-06 07:00:00"; t < "2007-06-06 19:00:00"; t.addHour() )
		data.push_back(dataFactory.getData( 100, RA, t) );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE( !d.get() );
}

TEST_F(ra2rr_12Test, testDataMarkedAsMissing)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getMissing(RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	ASSERT_TRUE(missing( *d) );
}

TEST_F(ra2rr_12Test, test12hZero)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	EXPECT_EQ(miutil::miTime("2007-06-06 06:00:00"), d->obstime() );
	EXPECT_NEAR( 0, d->corrected(), .00001);
}

TEST_F(ra2rr_12Test, test12hNegative)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 210.1, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 212.2, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	EXPECT_EQ(miutil::miTime("2007-06-06 06:00:00"), d->obstime() );
	EXPECT_NEAR( 0, d->corrected(), .00001);
}

TEST_F(ra2rr_12Test, test12hPositive24hNegative)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 214.4, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getData( 212.2, RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	EXPECT_EQ(miutil::miTime("2007-06-06 06:00:00"), d->obstime() );
	EXPECT_NEAR( 0, d->corrected(), .00001);
}

TEST_F(ra2rr_12Test, test12hPositive24hZero)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getData( 212.2, RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	EXPECT_EQ(miutil::miTime("2007-06-06 06:00:00"), d->obstime() );
	EXPECT_NEAR( 0, d->corrected(), .00001);
}

TEST_F(ra2rr_12Test, test12hPositive24hPositivePrev12hNegative)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 212.1, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getData( 211.1, RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	EXPECT_EQ(miutil::miTime("2007-06-06 06:00:00"), d->obstime() );
	EXPECT_NEAR( 1.2, d->corrected(), .00001);
}

TEST_F(ra2rr_12Test, test12hPositive24hPositivePrev12hZero)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 211.1, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getData( 211.1, RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	EXPECT_EQ(miutil::miTime("2007-06-06 06:00:00"), d->obstime() );
	EXPECT_NEAR( 2.2, d->corrected(), .00001);
}

TEST_F(ra2rr_12Test, test12hPositive24hPositivePrev12hPositive)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 211.0, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getData( 212.2, RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 213.3, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	EXPECT_EQ(miutil::miTime("2007-06-06 06:00:00"), d->obstime() );
	EXPECT_NEAR( 1.1, d->corrected(), .00001);
}

TEST_F(ra2rr_12Test, testEmptyBucket)
{
	AbstractAgregator::kvDataList data;
	const kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302);
	data.push_back(dataFactory.getData( 411.0, RA, "2007-06-05 06:00:00") );
	data.push_back(dataFactory.getData( 112.2, RA, "2007-06-05 18:00:00") );
	data.push_back(dataFactory.getData( 113.3, RA, "2007-06-06 06:00:00") );

	AbstractAgregator::kvDataPtr d = agregator.process(data.back(), data);
	ASSERT_TRUE(d.get() );
	EXPECT_EQ(miutil::miTime("2007-06-06 06:00:00"), d->obstime() );
	EXPECT_NEAR( 1.1, d->corrected(), .00001);
}
