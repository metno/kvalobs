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
#include <milog/milog.h>
#include <kvdb/dbdrivermgr.h>

using namespace std;

class kvdbTest : public testing::Test {
 public:
  kvdbTest() {
  }
};

TEST_F( kvdbTest, fixeDriverName ) {
  using namespace dnmi::db;
  ostringstream s;
  string dir(PKGLIB_DBDIR);
  string soVersion(KVALOBSLIBS_SO_VERSION);
  size_t i = soVersion.find(":");

  if (i != string::npos)
    soVersion.erase(i);

  DriverManager mgr;

  string driver("pgdriver.so");
  string res = mgr.fixDriverName(driver);

  s << dir << "/pgdriver.so." << soVersion;

  //cerr << "1: '" << s.str() << "'" << endl;
  //cerr << "2: '" << res << "'" << endl;

  EXPECT_TRUE( s.str() == res ) << "test1";

  driver = "pgdriver";
  res = mgr.fixDriverName(driver);

  EXPECT_TRUE( s.str() == res ) << "test2";

  driver = "pgdriver.";
  res = mgr.fixDriverName(driver);

  EXPECT_FALSE( s.str() == res ) << "test3";

  s.str("");
  driver = "/a/dir/pgdriver.so";
  res = mgr.fixDriverName(driver);
  s << "/a/dir/pgdriver.so." << soVersion;
  EXPECT_TRUE( s.str() == res ) << "test4";

  s.str("");
  s << "a/dir/pgdriver.so." << soVersion;
  driver = "a/dir/pgdriver.so";
  res = mgr.fixDriverName(driver);
  EXPECT_TRUE( s.str() == res ) << "test5";

  s.str("");
  s << "a/dir/pgdriver.so." << soVersion;
  driver = "a/dir/pgdriver.so.1";
  res = mgr.fixDriverName(driver);
  EXPECT_TRUE( s.str() == res ) << "test6";

  s.str("");
  s << "a/dir/pgdriver.so." << soVersion;
  driver = "a/dir/pgdriver.so.1.2";
  res = mgr.fixDriverName(driver);
  EXPECT_TRUE( s.str() == res ) << "test7";

  s.str("");
  s << dir << "/pgdriver.so." << soVersion;
  driver = "pgdriver.so.1.2";
  res = mgr.fixDriverName(driver);
  EXPECT_TRUE( s.str() == res ) << "test7";
}

int main(int argc, char* argv[]) {
  milog::Logger::logger().logLevel(milog::FATAL);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

