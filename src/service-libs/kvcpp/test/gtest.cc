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
#include <iostream>
#include "boost/date_time.hpp"
#include "boost/algorithm/string.hpp"
#include "gtest/gtest.h"
#include "lib/milog/milog.h"
#include "lib/miconfparser/confsection.h"
#include "lib/kvalobs/kvPath.h"
#include "lib/decodeutility/kvalobsdata.h"
#include "lib/dnmithread/CommandQue.h"
#include "service-libs/kvcpp/test/configuration.h"
#include "service-libs/kvcpp/test/testKafkaSubcriber.h"
#include "service-libs/kvcpp/kvDataSubscribeInfoHelper.h"
#include "service-libs/kvcpp/kvevents.h"

using std::string;
using boost::filesystem::path;
using miutil::conf::ConfSection;
namespace pt = boost::posix_time;
namespace b = boost;

namespace {
kvalobs::kvData createKvData(const std::string &obstime, int sid, int tid, int pid, float val) {
  pt::ptime obst = pt::from_iso_string(b::erase_all_copy(b::erase_all_copy(obstime, ":"), "-"));
  return kvalobs::kvData(sid, obst, val, pid, obst, tid, 0, 0, val, kvalobs::kvControlInfo(), kvalobs::kvUseInfo(), "");
}
}  // namespace


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

TEST_F(Tests, broadcast) {
  dnmi::thread::CommandQue queue;
  kvservice::KvDataSubscribeInfoHelper info;
  std::list<kvalobs::kvData> kvData;
  kvalobs::serialize::KvalobsData data;
  kvservice::DataEvent *event;

  info.addStationId(18700);  // Listen to data for station 18700

  // Create data for stationid 99999, we are not listening for this.
  kvData.push_back(createKvData("2016-03-08T06:00:00", 99999, 316, 10, 2.4));
  data.insert(createKvData("2016-03-08T06:00:00", 99999, 316, 10, 2.4));
  kvservice::kafka::test::broadcast(data, info, queue);
  event = dynamic_cast<kvservice::DataEvent*>(queue.get(1));
  ASSERT_TRUE(!event);  // We expect no event from stationid 99999

  data.clear();
  kvData.clear();
  // Create data for stationid 18700
  data.insert(createKvData("2016-03-08T06:00:00", 18700, 316, 10, 2.4));
  kvservice::kafka::test::broadcast(data, info, queue);
  event =  dynamic_cast<kvservice::DataEvent*>(queue.get(1));
  ASSERT_TRUE(event);  // We expect data from stationid 18700

  kvservice::KvObsDataListPtr theData = event->data();
  ASSERT_FALSE(!theData);
  ASSERT_EQ(theData->size(), 1);
  ASSERT_EQ(theData->front().dataList().size(), 1);
  ASSERT_EQ(theData->front().stationid(), 18700);
  ASSERT_EQ(theData->front().dataList().front().stationID(), 18700);
}

int main(int argc, char* argv[]) {
  milog::Logger::logger().logLevel(milog::FATAL);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
