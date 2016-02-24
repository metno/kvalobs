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

#include <memory>
#include <string>
#include "gtest/gtest.h"
#include "lib/milog/milog.h"
#include "lib/miconfparser/confsection.h"
#include "lib/kvalobs/kvPath.h"
#include "service-libs/kvcpp/test/configuration.h"

using std::string;
using boost::filesystem::path;
using miutil::conf::ConfSection;

class Tests : public testing::Test {
 public:
  Tests() {
  }

 protected:
  string testdir;
  virtual void SetUp() {
    testdir = TESTDIR;
    setKvPathPrefix(testdir);
  }
};

TEST_F(Tests, getConfig) {
  using kvservice::test::getConfiguration;
  using kvservice::test::readConf;
  using boost::filesystem::path;
  using std::shared_ptr;

  shared_ptr<ConfSection> conf = getConfiguration(shared_ptr<ConfSection>(), "gtest");
  ASSERT_TRUE(conf.get());

  //  A new call to getConfiguration should return the same configuration as the first call
  //  as it is cached. Independent of the input parameters.
  shared_ptr<ConfSection> conf2 = getConfiguration(shared_ptr<ConfSection>(), "kvalobs");
  ASSERT_TRUE(conf == conf2);

  //  Reset the cache. There is no configuration for 'kvalobs' so this should throw.
  EXPECT_THROW(kvservice::test::getConfiguration(shared_ptr<ConfSection>(), "kvalobs", true), std::runtime_error);

  //  Read the configuration file manually.
  path confFile = path(testdir) / "etc" / "kvalobs" / "gtest.conf";
  conf.reset(readConf(confFile));

  // This should use the configuration we supply. Regardless of the application name.
  conf2 = getConfiguration(conf, "kvalobs");
  ASSERT_TRUE(conf == conf2);

  //  And it should have been cached.
  conf2 = getConfiguration(shared_ptr<ConfSection>(), "kvalobs");
  ASSERT_TRUE(conf == conf2);
}

int main(int argc, char* argv[]) {
  milog::Logger::logger().logLevel(milog::FATAL);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
