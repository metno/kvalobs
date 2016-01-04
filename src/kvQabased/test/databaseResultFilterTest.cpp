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
#include <db/databaseResultFilter.h>
#include <kvalobs/kvDataOperations.h>

/**
 * All tests here are related to filtering results from a database query in
 * those cases where using the query itself to get exactly the data we need
 * is not practical.
 */

TEST(databaseResultFilterTest, getStationParamTest) {
  const std::string metadata =
      "max;highest;high;low;lowest;min\n98;33.4;30.1;0.0;-1;-2.2";

  using db::resultfilter::parseStationParam;

  EXPECT_FLOAT_EQ(98, parseStationParam(metadata, "max"));
  EXPECT_FLOAT_EQ(33.4, parseStationParam(metadata, "highest"));
  EXPECT_FLOAT_EQ(30.1, parseStationParam(metadata, "high"));
  EXPECT_FLOAT_EQ(0, parseStationParam(metadata, "low"));
  EXPECT_FLOAT_EQ(-1, parseStationParam(metadata, "lowest"));
  EXPECT_FLOAT_EQ(-2.2, parseStationParam(metadata, "min"));
}

TEST(databaseResultFilterTest, getStationParamOneEntryTest) {
  const std::string metadata = "c\n-2.2";

  using db::resultfilter::parseStationParam;

  EXPECT_FLOAT_EQ(-2.2, parseStationParam(metadata, "C"));
}

TEST(databaseResultFilterTest, caseInsensitiveMetadataTest) {
  const std::string metadata = "C\n-2.2";

  using db::resultfilter::parseStationParam;

  EXPECT_FLOAT_EQ(-2.2, parseStationParam(metadata, "c"));
}

TEST(databaseResultFilterTest, getStationParamWrongKeyThrows) {
  const std::string metadata =
      "max;highest;high;low;lowest;min\n98;33.4;30.1;0.0;-1;-2.2";
  EXPECT_THROW(db::resultfilter::parseStationParam(metadata, "foo"),
               std::exception);
}

TEST(databaseResultFilterTest, getDataSimpleCase) {
  kvalobs::kvDataFactory f(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 302);
  db::DatabaseAccess::DataList data;
  for (boost::posix_time::ptime t = boost::posix_time::time_from_string(
      "2010-06-11 06:00:00");
      t > boost::posix_time::time_from_string("2010-06-05 06:00:00"); t +=
          boost::posix_time::hours(-12))
    data.push_back(f.getData(1.0, 1, t));

  db::DatabaseAccess::DataList expectedResult = data;

  db::resultfilter::filter(data, 4);

  EXPECT_TRUE(expectedResult == data);
}

TEST(databaseResultFilterTest, getDataSimpleCaseNonstandardLevelAndSensor) {
  kvalobs::kvDataFactory f(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 302, 1,
      10);
  db::DatabaseAccess::DataList data;
  for (boost::posix_time::ptime t = boost::posix_time::time_from_string(
      "2010-06-11 06:00:00");
      t > boost::posix_time::time_from_string("2010-06-05 06:00:00"); t +=
          boost::posix_time::hours(-12))
    data.push_back(f.getData(1.0, 1, t));

  db::DatabaseAccess::DataList expectedResult = data;

  db::resultfilter::filter(data, 4);

  EXPECT_TRUE(expectedResult == data);
}

TEST(databaseResultFilterTest, getDataFiltersOutExtraLevels) {
  kvalobs::kvDataFactory f0(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 302, 0,
      0);
  kvalobs::kvDataFactory f10(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 302, 0,
      10);

  db::DatabaseAccess::DataList data;
  db::DatabaseAccess::DataList expectedResult;

  for (boost::posix_time::ptime t = boost::posix_time::time_from_string(
      "2010-06-11 06:00:00");
      t > boost::posix_time::time_from_string("2010-06-05 06:00:00"); t +=
          boost::posix_time::hours(-12)) {
    data.push_back(f0.getData(1.0, 1, t));
    data.push_back(f10.getData(1.0, 1, t));
    expectedResult.push_back(f0.getData(1.0, 1, t));
  }

  db::resultfilter::filter(data, 4);

  EXPECT_TRUE(data == expectedResult);
}

TEST(databaseResultFilterTest, getDataFiltersOutExtraSensors) {
  kvalobs::kvDataFactory f0(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 302, 0,
      0);
  kvalobs::kvDataFactory f1(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 302, 1,
      0);

  db::DatabaseAccess::DataList data;
  db::DatabaseAccess::DataList expectedResult;

  for (boost::posix_time::ptime t = boost::posix_time::time_from_string(
      "2010-06-11 06:00:00");
      t > boost::posix_time::time_from_string("2010-06-05 06:00:00"); t +=
          boost::posix_time::hours(-12)) {
    data.push_back(f0.getData(1.0, 1, t));
    data.push_back(f1.getData(1.0, 1, t));
    expectedResult.push_back(f0.getData(1.0, 1, t));
  }

  db::resultfilter::filter(data, 4);

  EXPECT_TRUE(data == expectedResult);
}

TEST(databaseResultFilterTest, getDataChoosesPreferredTypeId) {
  kvalobs::kvDataFactory f0(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 302);
  kvalobs::kvDataFactory f1(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 311);

  db::DatabaseAccess::DataList data;
  db::DatabaseAccess::DataList expectedResult;

  for (boost::posix_time::ptime t = boost::posix_time::time_from_string(
      "2010-06-11 06:00:00");
      t > boost::posix_time::time_from_string("2010-06-05 06:00:00"); t +=
          boost::posix_time::hours(-12)) {
    data.push_back(f0.getData(1.0, 1, t));
    data.push_back(f1.getData(1.0, 1, t));
    expectedResult.push_back(f0.getData(1.0, 1, t));
  }

  db::resultfilter::filter(data, 302);

  EXPECT_TRUE(data == expectedResult);
}

TEST(databaseResultFilterTest, getDataFiltersOutExtraTypes) {
  kvalobs::kvDataFactory f0(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 302);
  kvalobs::kvDataFactory f1(
      10, boost::posix_time::time_from_string("2010-06-11 06:00:00"), 311);

  db::DatabaseAccess::DataList data;
  db::DatabaseAccess::DataList expectedResult;

  for (boost::posix_time::ptime t = boost::posix_time::time_from_string(
      "2010-06-11 06:00:00");
      t > boost::posix_time::time_from_string("2010-06-05 06:00:00"); t +=
          boost::posix_time::hours(-12)) {
    data.push_back(f0.getData(1.0, 1, t));
    data.push_back(f1.getData(1.0, 1, t));
    expectedResult.push_back(f1.getData(1.0, 1, t));
  }

  db::resultfilter::filter(data, 4);

  EXPECT_TRUE(data == expectedResult);
}
