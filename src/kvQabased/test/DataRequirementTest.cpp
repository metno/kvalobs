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
#include <db/returntypes/DataRequirement.h>
#include <algorithm>

using qabase::DataRequirement;

TEST(DataRequirementTest, manyParameters) {
  DataRequirement req("obs;FX_1,FG_1,FG_010;;", 10, true);

  EXPECT_FALSE(req.empty());
  EXPECT_EQ("obs", req.requirementType());

  EXPECT_EQ(3u, req.parameter().size());
  EXPECT_TRUE(req.haveParameter("FX_1"));
  EXPECT_TRUE(req.haveParameter("FG_1"));
  EXPECT_TRUE(req.haveParameter("FG_010"));

  EXPECT_EQ(1u, req.station().size());
  EXPECT_TRUE(req.haveStation(10));

  EXPECT_EQ(0, req.firstTime());
  EXPECT_EQ(0, req.lastTime());
}

TEST(DataRequirementTest, implicitZeroTime) {
  DataRequirement req("obs;A;;-60", 42, true);

  EXPECT_FALSE(req.empty());
  EXPECT_EQ("obs", req.requirementType());

  EXPECT_EQ(1u, req.parameter().size());
  EXPECT_TRUE(req.haveParameter("A"));

  EXPECT_EQ(1u, req.station().size());
  EXPECT_TRUE(req.haveStation(42));

  EXPECT_EQ(-60, req.firstTime());
  EXPECT_EQ(0, req.lastTime());
}

TEST(DataRequirementTest, explicitZeroTime) {
  DataRequirement req("obs;B;;0,-120", 42, true);

  EXPECT_FALSE(req.empty());
  EXPECT_EQ("obs", req.requirementType());

  EXPECT_EQ(1u, req.parameter().size());
  EXPECT_TRUE(req.haveParameter("B"));

  EXPECT_EQ(1u, req.station().size());
  EXPECT_TRUE(req.haveStation(42));

  EXPECT_EQ(-120, req.firstTime());
  EXPECT_EQ(0, req.lastTime());
}

TEST(DataRequirementTest, swapUnorderedTime) {
  DataRequirement req("obs;C;;-180,0", 42, false);

  EXPECT_FALSE(req.empty());
  EXPECT_EQ("obs", req.requirementType());

  EXPECT_EQ(1u, req.parameter().size());
  EXPECT_TRUE(req.haveParameter("C"));

  EXPECT_EQ(1u, req.station().size());
  EXPECT_TRUE(req.haveStation(42));

  EXPECT_EQ(-180, req.firstTime());
  EXPECT_EQ(0, req.lastTime());
}

TEST(DataRequirementTest, ampersandInParameter) {
  DataRequirement req("obs;P&&&;;", 42, true);

  EXPECT_EQ(1u, req.parameter().size());
  ASSERT_TRUE(req.haveParameter("P"));

  const DataRequirement::ParameterList & plist = req.parameter();
  ASSERT_EQ(1u, plist.size());

  const DataRequirement::Parameter & p = plist.front();
  EXPECT_FALSE(p.haveSensor());
  EXPECT_FALSE(p.haveLevel());
  EXPECT_FALSE(p.haveType());
}

TEST(DataRequirementTest, levelInParameter) {
  DataRequirement req("obs;P&10&&;;", 42, true);

  EXPECT_EQ(1u, req.parameter().size());
  EXPECT_TRUE(req.haveParameter("P"));  // Hva brukes denne til?

  const DataRequirement::ParameterList & plist = req.parameter();
  ASSERT_EQ(1u, plist.size());

  const DataRequirement::Parameter & p = plist.front();
  EXPECT_TRUE(p.haveLevel());
  EXPECT_EQ(10, p.level());
  EXPECT_FALSE(p.haveSensor());
  EXPECT_FALSE(p.haveType());
}

TEST(DataRequirementTest, sensorInParameter) {
  DataRequirement req("obs;P&&1&;;", 42, true);

  EXPECT_EQ(1u, req.parameter().size());
  EXPECT_TRUE(req.haveParameter("P"));

  const DataRequirement::ParameterList & plist = req.parameter();
  ASSERT_EQ(1u, plist.size());

  const DataRequirement::Parameter & p = plist.front();
  EXPECT_FALSE(p.haveLevel());
  EXPECT_TRUE(p.haveSensor());
  EXPECT_EQ(1, p.sensor());
  EXPECT_FALSE(p.haveType());
}

TEST(DataRequirementTest, typeidInParameter) {
  DataRequirement req("obs;P&&&302;;", 42, true);

  EXPECT_EQ(1u, req.parameter().size());
  EXPECT_TRUE(req.haveParameter("P"));

  const DataRequirement::ParameterList & plist = req.parameter();
  ASSERT_EQ(1u, plist.size());

  const DataRequirement::Parameter & p = plist.front();
  EXPECT_FALSE(p.haveLevel());
  EXPECT_FALSE(p.haveSensor());
  EXPECT_TRUE(p.haveType());
  EXPECT_EQ(302, p.type());
}
