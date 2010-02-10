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
#include <configuration/AgregatorConfiguration.h>
#include <sstream>

class AgregatorConfigurationTest : public testing::Test
{
protected:
	AgregatorConfiguration config;
	std::ostringstream msg;
	std::ostringstream err;

	AgregatorConfigurationTest() :
		config(msg, err)
	{}

	AgregatorConfiguration::ParseResult parse(int & argc, const char ** argv)
	{
		return config.parse(argc, const_cast<char **>(argv));
	}
};

TEST_F(AgregatorConfigurationTest, helpString)
{
	int argc = 2;
	const char * argv[] = {"test", "--help"};

	EXPECT_EQ(AgregatorConfiguration::Exit_Success, parse(argc, argv));

	EXPECT_TRUE(not msg.str().empty());
	EXPECT_TRUE(err.str().empty());
}

TEST_F(AgregatorConfigurationTest, daemonModeLongOption)
{
	int argc = 2;
	const char * argv[] = {"test", "--daemon-mode"};

	EXPECT_EQ(AgregatorConfiguration::No_Action, parse(argc, argv));

	ASSERT_TRUE(config.daemonMode());
}

TEST_F(AgregatorConfigurationTest, daemonModeShortOption)
{
	int argc = 2;
	const char * argv[] = {"test", "-d"};

	EXPECT_EQ(AgregatorConfiguration::No_Action, parse(argc, argv));

	ASSERT_TRUE(config.daemonMode());
}

TEST_F(AgregatorConfigurationTest, stationList)
{
	int argc = 2;
	const char * argv[] = {"test", "-s1,2,3"};

	EXPECT_EQ(AgregatorConfiguration::No_Action, parse(argc, argv));

	const std::vector<int> & stations = config.stations();
	ASSERT_FALSE(stations.empty());

	EXPECT_EQ(3u, stations.size()) << "(last value is " << stations.back() << ")";

	for (int i = 0; i < 3; ++ i )
		EXPECT_EQ(i +1, stations[i]);
}

TEST_F(AgregatorConfigurationTest, stationListManyTimes)
{
	int argc = 3;
	const char * argv[] = {"test", "-s1,2,3", "-s4"};

	EXPECT_EQ(AgregatorConfiguration::No_Action, parse(argc, argv));

	const std::vector<int> & stations = config.stations();
	ASSERT_FALSE(stations.empty());

	EXPECT_EQ(4u, stations.size()) << "(last value is " << stations.back() << ")";

	for (int i = 0; i < 4; ++ i )
		EXPECT_EQ(i +1, stations[i]);
}
