/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  Copyright (C) 2010 met.no

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
#include <kvDataFormatter.h>
#include <kvalobs/kvDataOperations.h>

using namespace decodeutility::kvdataformatter;

class kvDataFormatterTest : public testing::Test
{
public:
	kvDataFormatterTest() :
		f(10, "2010-02-25 00:00:00", 1)
	{}

protected:
	kvalobs::kvDataFactory f;
};

TEST_F(kvDataFormatterTest, noDecimal)
{
	kvalobs::kvData in = f.getData(1, 100);

	miutil::miString s = createString(in);

	kvDataList out = getKvData(s);
	ASSERT_EQ(1u, out.size());

	EXPECT_FLOAT_EQ(in.corrected(), out.front().corrected());
}

TEST_F(kvDataFormatterTest, oneDecimal)
{
	kvalobs::kvDataFactory f(10, "2010-02-25 00:00:00", 1);
	kvalobs::kvData in = f.getData(1.3, 100);

	miutil::miString s = createString(in);

	kvDataList out = getKvData(s);
	ASSERT_EQ(1u, out.size());

	EXPECT_FLOAT_EQ(in.corrected(), out.front().corrected());
}

TEST_F(kvDataFormatterTest, multipleDecimals)
{
	kvalobs::kvDataFactory f(10, "2010-02-25 00:00:00", 1);
	kvalobs::kvData in = f.getData(1.12345, 100);

	miutil::miString s = createString(in);

	kvDataList out = getKvData(s);
	ASSERT_EQ(1u, out.size());

	EXPECT_FLOAT_EQ(in.corrected(), out.front().corrected());
}
