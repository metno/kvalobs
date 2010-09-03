/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvalobsdataserializertest.cc,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $

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

#include <gtest/gtest.h>
#include "kvalobsdata.h"
#include <kvalobs/kvDataOperations.h>
#include <set>
#include <boost/shared_ptr.hpp>
#include "kvalobsdataserializer.h"
#include "kvalobsdataparser.h"
#include <string>

using namespace std;
using namespace kvalobs;
using namespace kvalobs::serialize;

class KvalobsDataSerializerTest: public testing::Test
{
protected:
	typedef std::set<kvalobs::kvData, kvalobs::compare::lt_kvData> DSet;
	DSet indata;
	kvalobs::serialize::KvalobsData in;

	typedef boost::shared_ptr<kvalobs::serialize::KvalobsData> KvalobsDataPtr;

	/**
	 * Create string representation, and the turn it back into kvData objects
	 */
	KvalobsDataPtr loop()
	{
		string xml = KvalobsDataSerializer::serialize(in);
		KvalobsDataPtr out(new KvalobsData);
		KvalobsDataParser::parse(xml, *out.get());
		return out;
	}

	KvalobsDataSerializerTest()
	{
		kvalobs::kvDataFactory f(42, "2006-04-26 06:00:00", 302);
		indata.insert(f.getData(1.0, 110));
		indata.insert(f.getData(4, 112));
		indata.insert(f.getData(3, 18));
		indata.insert(f.getData(3, 34));
		indata.insert(f.getData(3, 34, "2006-04-25 18:00:00"));

		in.insert(indata.begin(), indata.end());
	}
};

TEST_F(KvalobsDataSerializerTest, testPreserveKvData)
{
	KvalobsDataPtr out = loop();
	list<kvData> outdata;
	out->getData(outdata);
	DSet outdata_set(outdata.begin(), outdata.end());

	ASSERT_EQ(in.size(), out->size());
	EXPECT_TRUE( equal( indata.begin(), indata.end(), outdata_set.begin(), compare::exactly_equal_ex_tbtime() ) );
}

TEST_F(KvalobsDataSerializerTest, testPreserveOverwrite)
{
	KvalobsDataPtr out = loop();
	EXPECT_TRUE( not out->overwrite() );
	in.overwrite( true );
	out = loop();
	EXPECT_TRUE( out->overwrite() );
}

TEST_F(KvalobsDataSerializerTest, testPreserveInvalidate)
{
	in.invalidate(true, 42, 302, "2006-04-26 06:00:00");
	in.invalidate(true, 42, 302, "2006-04-25 18:00:00");
	in.invalidate(true, 42, 302, "2006-04-25 12:00:00");

	KvalobsDataPtr out = loop();

	EXPECT_TRUE( out->isInvalidate( 42, 302, "2006-04-26 06:00:00" ) );
	EXPECT_TRUE( out->isInvalidate( 42, 302, "2006-04-25 18:00:00" ) );
	EXPECT_TRUE( out->isInvalidate( 42, 302, "2006-04-25 12:00:00" ) );
	EXPECT_TRUE( not out->isInvalidate( 42, 302, "2006-04-26 12:00:00" ) );
}

TEST_F(KvalobsDataSerializerTest, testPreserveFixedRejectedList)
{
	kvalobs::kvRejectdecode rejected("hallo?", "2010-08-11 12:00:00",
			"somedecoder", "unknown message");
	in.setMessageCorrectsThisRejection(rejected);

	KvalobsDataPtr out = loop();

	KvalobsData::RejectList fixedReject;
	out->getRejectedCorrections(fixedReject);
	ASSERT_EQ(1u, fixedReject.size());
	EXPECT_EQ(rejected, fixedReject[0]);
}

TEST_F(KvalobsDataSerializerTest, testPreserveMultipleFixedRejectedList)
{
	kvalobs::kvRejectdecode r1("hallo?", "2010-08-11 12:00:00", "somedecoder", "unknown message");
	kvalobs::kvRejectdecode r2("wrong msg", "2010-08-12 12:12:41", "some_other_decoder", "");
	kvalobs::kvRejectdecode r3("wrong msg", "2010-08-12 12:12:45", "somedecoder", "");

	in.setMessageCorrectsThisRejection(r1);
	in.setMessageCorrectsThisRejection(r2);
	in.setMessageCorrectsThisRejection(r3);

	KvalobsDataPtr out = loop();

	KvalobsData::RejectList fixedReject;
	out->getRejectedCorrections(fixedReject);
	EXPECT_EQ(3u, fixedReject.size());

	EXPECT_TRUE(std::find(fixedReject.begin(), fixedReject.end(), r1) != fixedReject.end());
	EXPECT_TRUE(std::find(fixedReject.begin(), fixedReject.end(), r2) != fixedReject.end());
	EXPECT_TRUE(std::find(fixedReject.begin(), fixedReject.end(), r3) != fixedReject.end());
}
