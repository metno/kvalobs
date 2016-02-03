/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvalobsdatatest.cc,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#include <kvalobs/kvTextDataOperations.h>

using namespace std;
using namespace kvalobs;
using namespace kvalobs::serialize;

class KvalobsDataTest : public testing::Test {
 protected:
  KvalobsData data;
};

TEST_F(KvalobsDataTest, testConstructor) {
  EXPECT_TRUE(not data.overwrite());
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(size_t(0), data.size());

  list<kvData> d;
  list<kvTextData> td;
  data.getData(d, td);
  EXPECT_TRUE(d.empty());
  EXPECT_TRUE(td.empty());

  list<KvalobsData::InvalidateSpec> inv;
  data.getInvalidate(inv);
  EXPECT_TRUE(inv.empty());
}

TEST_F(KvalobsDataTest, testInsertKvData) {
  kvalobs::kvDataFactory f(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302);
  kvData d = f.getData(0.1, 110);
  data.insert(d);
  list<kvData> out;
  data.getData(out);

  ASSERT_TRUE(not data.empty());
  EXPECT_EQ(size_t(1), data.size());
  EXPECT_EQ(size_t(1), out.size());
  EXPECT_TRUE(compare::exactly_equal_ex_tbtime()(d, out.front()));
}

TEST_F(KvalobsDataTest, testInsertKvDataMultipleSensors_a) {
  kvalobs::kvDataFactory f0(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302);
  kvData d0 = f0.getData(0.1, 110);
  data.insert(d0);

  kvalobs::kvDataFactory f1(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302, '1');
  kvData d1 = f1.getData(0.2, 110);
  data.insert(d1);

  list<kvData> out;
  data.getData(out);

  ASSERT_TRUE(not data.empty());
  EXPECT_EQ(size_t(2), data.size());  // 1 or 2?
  EXPECT_EQ(size_t(2), out.size());

  EXPECT_TRUE(
      compare::exactly_equal_ex_tbtime()(d0, out.front())
          or compare::exactly_equal_ex_tbtime()(d0, out.back()));
  EXPECT_TRUE(
      compare::exactly_equal_ex_tbtime()(d1, out.front())
          or compare::exactly_equal_ex_tbtime()(d1, out.back()));
}

TEST_F(KvalobsDataTest, testInsertKvDataMultipleSensors_b) {
  kvalobs::kvDataFactory f0(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302);
  kvData d0 = f0.getData(0.1, 110);
  data.insert(d0);

  kvalobs::kvDataFactory f1(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302, 1);
  kvData d1 = f1.getData(0.2, 110);
  data.insert(d1);

  list<kvData> out;
  data.getData(out);

  ASSERT_TRUE(not data.empty());
  EXPECT_EQ(size_t(2), data.size());  // 1 or 2?
  EXPECT_EQ(size_t(2), out.size());

  EXPECT_TRUE(
      compare::exactly_equal_ex_tbtime()(d0, out.front())
          or compare::exactly_equal_ex_tbtime()(d0, out.back()));
  EXPECT_TRUE(
      compare::exactly_equal_ex_tbtime()(d1, out.front())
          or compare::exactly_equal_ex_tbtime()(d1, out.back()));
}

TEST_F(KvalobsDataTest, testInsertKvTextData) {
  kvTextDataFactory f(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302);
  kvTextData d = f.getData("FOO", 1021);
  data.insert(d);
  list<kvTextData> out;
  data.getData(out);

  ASSERT_TRUE(not data.empty());
  EXPECT_EQ(size_t(1), data.size());
  EXPECT_EQ(size_t(1), out.size());
  EXPECT_TRUE(compare::kvTextData_exactly_equal_ex_tbtime()(d, out.front()));
}

TEST_F(KvalobsDataTest, testInvalidate) {
  int st = 4;
  int tp = 5;
  boost::posix_time::ptime ot = boost::posix_time::time_from_string(
      "2006-04-26 06:00:00");
  data.invalidate(true, st, tp, ot);

  EXPECT_TRUE(data.isInvalidate(st, tp, ot));

  list<KvalobsData::InvalidateSpec> inv;
  data.getInvalidate(inv);

  EXPECT_EQ(size_t(1), inv.size());

  const KvalobsData::InvalidateSpec & i = inv.front();
  EXPECT_EQ(st, i.station);
  EXPECT_EQ(tp, i.typeID);
  EXPECT_EQ(ot, i.obstime);
}

TEST_F(KvalobsDataTest, testSummary) {
  kvalobs::kvDataFactory f(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302);
  kvData d = f.getData(0.1, 110);
  data.insert(d);

  const std::set<kvalobs::kvStationInfo> & si = data.summary();

  ASSERT_EQ(1u, si.size());

  const kvalobs::kvStationInfo & s = *si.begin();

  EXPECT_EQ(42, s.stationID());
  EXPECT_EQ(boost::posix_time::time_from_string("2006-04-26 06:00:00"),
            s.obstime());
  EXPECT_EQ(302, s.typeID());
}

TEST_F(KvalobsDataTest, testMultipleTbTime) {
  kvalobs::kvDataFactory f(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302);
  kvData d = f.getData(0.1, 110);
  data.insert(f.getData(0.1, 110));
  data.insert(f.getData(0.1, 110));
  EXPECT_EQ(1u, data.size());
}

TEST_F(KvalobsDataTest, testMultipleTbTimeAndData) {
  kvalobs::kvDataFactory f(
      42, boost::posix_time::time_from_string("2006-04-26 06:00:00"), 302);
  kvData d = f.getData(0.1, 110);
  data.insert(f.getData(0.1, 110));
  data.insert(f.getData(0.1, 110));
  data.insert(f.getData(0.1, 109));
  EXPECT_EQ(2u, data.size());
}
