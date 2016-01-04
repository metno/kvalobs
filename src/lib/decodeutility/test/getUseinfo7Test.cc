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

#include <boost/date_time/posix_time/ptime.hpp>
#include <gtest/gtest.h>
#include <miutil/timeconvert.h>
#include <kvalobs/kvTypes.h>
#include <decodeutility/getUseInfo7.h>

using namespace std;
using namespace decodeutility;
using namespace kvalobs;

namespace pt = boost::posix_time;

class GetUseInfoTest : public testing::Test {
 public:
  GetUseInfoTest() {
  }

};

TEST_F(GetUseInfoTest, getUseInfo7) {
  list<kvTypes> types;

  types.push_back(kvTypes(1, "", 60, 60, "I", "h", "For test"));

  pt::ptime obstime(pt::time_from_string("2010-09-13 06:00:00"));
  pt::ptime oktime(pt::time_from_string("2010-09-13 06:59:59"));
  pt::ptime oktime1(pt::time_from_string("2010-09-13 05:00:01"));
  pt::ptime toearly(pt::time_from_string("2010-09-13 04:59:00"));
  pt::ptime tolate(pt::time_from_string("2010-09-13 07:01:00"));

  ASSERT_TRUE(getUseinfo7Code(1, oktime, obstime, types) == 0);
  ASSERT_TRUE(getUseinfo7Code(1, oktime1, obstime, types) == 0);
  ASSERT_TRUE(getUseinfo7Code(1, toearly, obstime, types) == 3);
  ASSERT_TRUE(getUseinfo7Code(1, tolate, obstime, types) == 4);

}

