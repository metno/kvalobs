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
#include <scriptcreate/DataStore.h>
#include <kvalobs/kvDataOperations.h>
#include <boost/assign/list_of.hpp>

using qabase::DataStore;

TEST(DataStore_ParameterSortedDataListTest, fillMissing_nothingToFill)
{
	kvalobs::kvDataFactory factory(10, "2010-06-09 06:00:00", 302);

	DataStore::ParameterSortedDataList dl;
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-07 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-07 06:00:00"));

	DataStore::ParameterSortedDataList expectedResult = dl;

	qabase::fillMissing(dl);

	EXPECT_TRUE(dl == expectedResult);
}

TEST(DataStore_ParameterSortedDataListTest, fillMissing_missingLastEntry)
{
	kvalobs::kvDataFactory factory(10, "2010-06-09 06:00:00", 302);

	DataStore::ParameterSortedDataList dl;
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-07 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));

	DataStore::ParameterSortedDataList expectedResult = dl;
	expectedResult["B"].push_back(factory.getMissing(0, "2010-06-07 06:00:00"));

	qabase::fillMissing(dl);

	ASSERT_EQ(3u, dl["B"].size());
	EXPECT_EQ("2010-06-07 06:00:00", dl["B"][2].obstime());
	EXPECT_TRUE(kvalobs::missing(dl["B"][2]));
}

TEST(DataStore_ParameterSortedDataListTest, fillMissing_missingLastEntry2)
{
	kvalobs::kvDataFactory factory(10, "2010-06-09 06:00:00", 302);

	DataStore::ParameterSortedDataList dl;
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-07 06:00:00"));

	DataStore::ParameterSortedDataList expectedResult = dl;
	expectedResult["A"].push_back(factory.getMissing(0, "2010-06-07 06:00:00"));

	qabase::fillMissing(dl);

	ASSERT_EQ(3u, dl["A"].size());
	EXPECT_EQ("2010-06-07 06:00:00", dl["A"][2].obstime());
	EXPECT_TRUE(kvalobs::missing(dl["A"][2]));
}

TEST(DataStore_ParameterSortedDataListTest, fillMissing_manyMissingLastEntries)
{
	kvalobs::kvDataFactory factory(10, "2010-06-09 06:00:00", 302);

	DataStore::ParameterSortedDataList dl;
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-07 06:00:00"));

	DataStore::ParameterSortedDataList expectedResult = dl;
	expectedResult["A"].push_back(factory.getMissing(0, "2010-06-07 06:00:00"));

	qabase::fillMissing(dl);

	ASSERT_EQ(3u, dl["A"].size());
	EXPECT_EQ("2010-06-08 06:00:00", dl["A"][1].obstime());
	EXPECT_TRUE(kvalobs::missing(dl["A"][1]));
	EXPECT_EQ("2010-06-07 06:00:00", dl["A"][2].obstime());
	EXPECT_TRUE(kvalobs::missing(dl["A"][2]));
}


TEST(DataStore_ParameterSortedDataListTest, fillMissing_missingInAllSets)
{
	kvalobs::kvDataFactory factory(10, "2010-06-09 06:00:00", 302);

	DataStore::ParameterSortedDataList dl;
	//dl["A"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));
	dl["A"].push_back(factory.getData(10, 1.1, "2010-06-07 06:00:00"));

	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-09 06:00:00"));
	//dl["B"].push_back(factory.getData(10, 1.1, "2010-06-08 06:00:00"));
	dl["B"].push_back(factory.getData(10, 1.1, "2010-06-07 06:00:00"));

	DataStore::ParameterSortedDataList expectedResult = dl;
	expectedResult["A"].push_back(factory.getMissing(0, "2010-06-07 06:00:00"));

	qabase::fillMissing(dl);

	ASSERT_EQ(3u, dl["A"].size());
	EXPECT_EQ("2010-06-09 06:00:00", dl["A"][0].obstime());
	EXPECT_TRUE(kvalobs::missing(dl["A"][0]));

	ASSERT_EQ(3u, dl["B"].size());
	EXPECT_EQ("2010-06-08 06:00:00", dl["B"][1].obstime());
	EXPECT_TRUE(kvalobs::missing(dl["B"][1]));
}
