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
#include "lib/kvalobs/test/sqlesctest.h"

using namespace std;
//using namespace decodeutility;
//using namespace kvalobs;

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

TEST_F(Tests, paramlist) {
  string paramfile(testdir + "/stinfosys_params.csv");
  ParamList params;
  Param param;

  ASSERT_TRUE( readParamsFromFile( paramfile, params ) )<< "Cant read parameter file '" << paramfile << "'.";

  ASSERT_EQ( params.size(), 397);

  ASSERT_FALSE(findParamInList(params, "NotAParam", param));

  //Check that a paramid that is NOT defined return a empty string
  ASSERT_TRUE(findParamIdInList(params, 100).empty());

  //Check that a paramid that is defined return a non empty string.
  ASSERT_EQ(findParamIdInList(params, 211), "TA");

  ASSERT_TRUE(findParamInList(params, "TA", param));
  ASSERT_EQ(param.id(), 211);
  ASSERT_TRUE(param.isScalar());

  //Test that a param with 'scalar=f' return isScalar() == false.
  ASSERT_TRUE(findParamInList(params, "KLFG", param));
  ASSERT_EQ(param.id(), 1025);
  ASSERT_FALSE(param.isScalar());
}

TEST_F(Tests, SqlEsc) {
  using kvalobs::test::escapeSql_;
  using std::string;
  string val = escapeSql_("Hei");

  ASSERT_EQ(val, "\'Hei\'");
  val = escapeSql_("\'frank\';  DROP TABLE people;");
  ASSERT_EQ(val, "\'\'\'frank\'\';  DROP TABLE people;\'");

  pt::ptime dt = pt::time_from_string_nothrow("2016-02-26 15:00:00");
  ASSERT_TRUE(!dt.is_special());
  val = kvalobs::kvDbBase::quoted(dt);
  ASSERT_EQ(val, "\'2016-02-26 15:00:00\'");
  ASSERT_EQ(kvalobs::kvDbBase::quoted(10), "\'10\'");
}

int main(int argc, char* argv[]) {
  milog::Logger::logger().logLevel(milog::FATAL);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

