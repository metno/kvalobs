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

#include <string.h>
#include <string>
#include <memory>
#include "boost/date_time/posix_time/ptime.hpp"
#include "gtest/gtest.h"
#include "lib/miutil/timeconvert.h"
#include "lib/kvalobs/paramlist.h"
#include "lib/kvalobs/kvDbBase.h"
#include "lib/milog/milog.h"
#include "lib/kvalobs/kvTextData.h"
#include "lib/decoder/kltext/kltext.h"
#include "lib/decoder/kltext/test/testheader.h"

using std::string;
using kvalobs::kvTextData;

namespace pt = boost::posix_time;

class Tests : public testing::Test {
 public:
  Tests() {
  }

 protected:
  string testdir;
  virtual void SetUp() {
    testdir = TESTDIR;
  }
};

int fakeGetStationId(const std::string &key, const std::string &val) {
  return std::stoi(val);
}


TEST_F(Tests, DecodeTest) {
  kvalobs::decoder::kltextdecoder::test::setTbTime(pt::time_from_string_nothrow("2016-02-26 10:58:00"));
  kvalobs::decoder::kltextdecoder::KlText decode(fakeGetStationId);

  string msg;
  kvTextData textData;
  string obsType = "kltext/stationid=100/type=313";
  string obs = "Dette er en obs";

  decode.execute(obsType, obs, msg, &textData);
  ASSERT_EQ(textData.paramID(), 1001);
  ASSERT_TRUE(!textData.obstime().is_special());
  ASSERT_EQ(textData.stationID(), 100);
  ASSERT_EQ(textData.typeID(), 313);
  ASSERT_EQ(textData.original(), obs);

  obsType = "kltext/stationid=100/type=313/received_time=2016-02-26 10:38:00";
  obs = "Dette er en obs \' med appostrof";
  pt::ptime obst = pt::time_from_string_nothrow("2016-02-26 10:38:00");

  decode.execute(obsType, obs, msg, &textData);
  ASSERT_EQ(textData.paramID(), 1001);
  ASSERT_EQ(textData.obstime(), obst);
  ASSERT_EQ(textData.stationID(), 100);
  ASSERT_EQ(textData.typeID(), 313);
  ASSERT_EQ(textData.original(), obs);
  ASSERT_EQ(textData.toSend(), "(100,'2016-02-26 10:38:00','Dette er en obs '' med appostrof',1001,'2016-02-26 10:58:00',313)");
}


int main(int argc, char* argv[]) {
  milog::Logger::logger().logLevel(milog::FATAL);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
