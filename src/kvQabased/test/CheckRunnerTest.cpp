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
#include "FakeDatabaseAccess.h"
#include "MockDatabaseAccess.h"
#include <CheckRunner.h>
#include <scriptcreate/DataStore.h>
#include <scriptrunner/Script.h>
#include <decodeutility/kvalobsdata.h>
#include <kvalobs/kvDataOperations.h>
#include <boost/scoped_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include <algorithm>
#include <list>

using qabase::CheckRunner;

class CheckRunnerTest : public testing::Test {
 public:
  CheckRunnerTest()
      : observation(10,
                    boost::posix_time::time_from_string("2010-05-12 06:00:00"),
                    302),
        factory(10, boost::posix_time::time_from_string("2010-05-12 06:00:00"),
                302),
        runner(nullptr) {
  }
 protected:
  virtual void SetUp() {
    auto db = std::make_shared<MockDatabaseAccess>();
    db->setDefaultActions();
    database = db;
    runner = new CheckRunner(database);
  }

  virtual void TearDown() {
    delete runner;
    database.reset();
  }

  std::shared_ptr<MockDatabaseAccess> database;
  CheckRunner * runner;
  kvalobs::kvStationInfo observation;
  kvalobs::kvDataFactory factory;
};

namespace {
  bool isSimilar(const CheckRunner::DataList & a, const CheckRunner::DataList & b) {
    if (a.size() != b.size())
      return false;
    for (const kvalobs::kvData & d : a) {
      auto func = std::bind1st(kvalobs::compare::exactly_equal_ex_tbtime(), d);
      auto other = std::find_if(b.begin(), b.end(), func);
      if (other == b.end())
        return false;
    }
    return true;
  }
}  // namespace

TEST_F(CheckRunnerTest, test) {
  runner->newObservation(observation);
}

TEST_F(CheckRunnerTest, resetsFlagsBeforeCheck) {
  using namespace testing;

  db::DatabaseAccess::DataList dataFromDatabase = boost::assign::list_of(
      factory.getData(6.0, 110));
  // Data has lots of rubbish flags, but fagg=1 fmis=3 fd=2 and fpre=7
  // fagg and fmis flags should be preserved, and fmis should also be preserved if it equals 7
  dataFromDatabase.front().controlinfo(
      kvalobs::kvControlInfo("1999993999992790"));
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(
      SetArgumentPointee<0>(dataFromDatabase));

  // TAM_24 have got another typeid, and will not be used
  kvalobs::kvDataFactory factory2(factory.stationID(), factory.obstime(),
                                  factory.typeID() + 1);
  db::DatabaseAccess::DataList taData = boost::assign::list_of(
      factory2.getData(23.0, 211));
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("TAM_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(taData));

  db::DatabaseAccess::DataList expectedScriptReturn = boost::assign::list_of(
      factory.getData(6.0, 110));
  expectedScriptReturn.front().controlinfo(
      kvalobs::kvControlInfo("1040003000002700"));
  expectedScriptReturn.front().cfailed("QC1-2-101");  // QC1-2-101 is the default qcx return from fake database
  expectedScriptReturn.front().tbtime(dataFromDatabase.front().tbtime());

  // Typical error condition:
  // checkrunner fails to reset flags before check, causing no update (since
  // new flag < old flag). Since data have not been changed, it will not be
  // written to database.

  EXPECT_CALL(*database, write(expectedScriptReturn)).Times(1);

  runner->newObservation(observation);

  ASSERT_EQ(1u, database->savedData().size());
  const kvalobs::kvData & returnFromScript = *database->savedData().begin();

  kvalobs::kvData expectedData = expectedScriptReturn.front();
  kvalobs::kvUseInfo ui = expectedData.useinfo();
  ui.setUseFlags(expectedData.controlinfo());
  expectedData.useinfo(ui);

  EXPECT_TRUE(
      kvalobs::compare::exactly_equal()(expectedData, returnFromScript));

  EXPECT_EQ(expectedData.controlinfo(), returnFromScript.controlinfo());
}

TEST_F(CheckRunnerTest, uncheckedObservationDataUpdatesUseinfo) {
  using namespace testing;

  db::DatabaseAccess::CheckList checks;  // empty check list
  EXPECT_CALL(*database, getChecks(_, observation)).Times(AtLeast(1))
      .WillRepeatedly(SetArgumentPointee<0>(checks));

  db::DatabaseAccess::ParameterList expectedParameters = boost::assign::list_of(
      "RR_24");
  EXPECT_CALL(*database, getParametersToCheck(_, observation)).Times(AtLeast(1))
      .WillRepeatedly(SetArgumentPointee<0>(expectedParameters));

  kvalobs::kvData expected = factory.getMissing(110);
  kvalobs::kvControlInfo ci = expected.controlinfo();
  ci.set(kvQCFlagTypes::f_fpre, 7);
  expected.controlinfo(ci);

  //getData(DataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const;
  db::DatabaseAccess::DataList databaseReturn = boost::assign::list_of(
      expected);
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(databaseReturn));

  kvalobs::kvUseInfo ui;
  ui.setUseFlags(expected.controlinfo());
  expected.useinfo(ui);

  db::DatabaseAccess::DataList expectedScriptReturn = boost::assign::list_of(
      expected);

  EXPECT_CALL(*database, write(expectedScriptReturn)).Times(AtLeast(1));

  runner->newObservation(observation);

  ASSERT_EQ(1u, database->savedData().size());
  const kvalobs::kvData & returnFromScript = *database->savedData().begin();

  kvalobs::kvData expectedData = expectedScriptReturn.front();
  expectedData.useinfo(ui);

  EXPECT_TRUE(
      kvalobs::compare::exactly_equal()(expectedData, returnFromScript));
  EXPECT_EQ(expectedData.controlinfo(), returnFromScript.controlinfo());
  EXPECT_EQ(expectedData.useinfo(), returnFromScript.useinfo());
}

TEST_F(CheckRunnerTest, forwardsAggregatedData) {
  using namespace testing;
  kvalobs::kvStationInfo stInfo(10, factory.obstime(), -308);
  kvalobs::kvDataFactory factory2(stInfo);


  db::DatabaseAccess::DataList dataFromDatabase = {
      factory2.getData(6.0, 110)
  };
  auto databaseData = std::make_shared<kvalobs::serialize::KvalobsData>(dataFromDatabase);

  db::DatabaseAccess::DataList emptyDataList;
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(0);
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("TAM_24"), 0))
      .Times(0);
  EXPECT_CALL(*database, write(_)).Times(0);
  EXPECT_CALL(*database, complete(stInfo, _, _))
      .Times(AtLeast(1)).WillRepeatedly(Return(databaseData));

  CheckRunner::KvalobsDataPtr modifiedList = runner->newObservation(stInfo);

  std::list<kvalobs::kvData> data;
  modifiedList->getData(data);
  ASSERT_EQ(1u, data.size());
  EXPECT_EQ(data.front(), dataFromDatabase.front());
}

TEST_F(CheckRunnerTest, skipsHqcCorrectedData) {
  using namespace testing;

  db::DatabaseAccess::DataList dataFromDatabase = boost::assign::list_of(
      factory.getData(6.0, 110));
  dataFromDatabase.front().controlinfo(
      kvalobs::kvControlInfo("0000000000000001"));
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(
      SetArgumentPointee<0>(dataFromDatabase));
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("TAM_24"), 0))
      .Times(AtLeast(1));
  EXPECT_CALL(*database, write(_)).Times(0);

  CheckRunner::KvalobsDataPtr modifiedList = runner->newObservation(observation);
  std::list<kvalobs::kvData> data;
  modifiedList->getData(data);
  EXPECT_TRUE(isSimilar(dataFromDatabase, data));
}

TEST_F(CheckRunnerTest, reusesResultsFromOtherChecks) {
  using namespace testing;

  db::DatabaseAccess::CheckList checks;
  checks.push_back(
      kvalobs::kvChecks(
          10, "QC1-2-101", "QC1-2", 1, "foo", "obs;RR_24;;", "* * * * *",
          boost::posix_time::time_from_string("2010-01-01 00:00:00")));
  checks.push_back(
      kvalobs::kvChecks(
          10, "QC1-3-101", "QC1-3", 1, "foo", "obs;RR_24;;", "* * * * *",
          boost::posix_time::time_from_string("2010-01-01 00:00:00")));
  EXPECT_CALL(*database, getChecks(_, observation)).Times(AtLeast(1))
      .WillRepeatedly(SetArgumentPointee<0>(checks));

  db::DatabaseAccess::DataList expectedScriptReturn = boost::assign::list_of(
      factory.getData(6.0, 110));
  expectedScriptReturn.front().controlinfo(
      kvalobs::kvControlInfo("0044000000000000"));
  expectedScriptReturn.front().cfailed("QC1-2-101,QC1-3-101");
  EXPECT_CALL(*database, write(expectedScriptReturn)).Times(AtLeast(1));

  runner->newObservation(observation);

  ASSERT_EQ(1u, database->savedData().size());
  const kvalobs::kvData & returnFromScript = *database->savedData().begin();

  kvalobs::kvData expectedData = expectedScriptReturn.front();
  kvalobs::kvUseInfo ui = expectedData.useinfo();
  ui.setUseFlags(expectedData.controlinfo());
  expectedData.useinfo(ui);

  EXPECT_TRUE(
      kvalobs::compare::exactly_equal_ex_tbtime()(expectedData,
                                                  returnFromScript));

  EXPECT_EQ(expectedData.controlinfo(), returnFromScript.controlinfo());
}

TEST_F(CheckRunnerTest, runCheckWithOneParameterAtOddLevel) {
  using namespace testing;

  kvalobs::kvStationInfo observation(
      20, boost::posix_time::time_from_string("2010-05-12 06:00:00"), 302);
  kvalobs::kvDataFactory factory(
      20, boost::posix_time::time_from_string("2010-05-12 06:00:00"), 302);

  db::DatabaseAccess::DataList rrFromDatabase;
  kvalobs::kvData rr = factory.getData(6.0, 110);
  rrFromDatabase.push_back(rr);

  // Placing tam data at level 10:
  kvalobs::kvDataFactory f2(factory.stationID(), factory.obstime(),
                            factory.typeID(), 0, 10);
  db::DatabaseAccess::DataList tamFromDatabase;
  kvalobs::kvData tam = f2.getData(19, 109);
  tamFromDatabase.push_back(tam);

  // NOTE: The controlinfo flags are not checked in the EXPECT_CALL(.., write(..)) call, below.
  rr.controlinfo(kvalobs::kvControlInfo("0010000000000000"));
  tam.controlinfo(kvalobs::kvControlInfo("0020000000000000"));
  db::DatabaseAccess::DataList expectedWrite;
  expectedWrite.push_back(rr);
  expectedWrite.push_back(tam);

  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(rrFromDatabase));
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("TAM_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(tamFromDatabase));
  EXPECT_CALL(*database, write(expectedWrite)).Times(1);

  runner->newObservation(observation);
}

TEST_F(CheckRunnerTest, runDecision) {
  db::DatabaseAccess::ParameterList expectedParameters;
  database->getParametersToCheck(&expectedParameters, observation);

  ASSERT_TRUE(expectedParameters.find("RR_24") != expectedParameters.end())<< "Check precondition error";

  EXPECT_TRUE(
      runner->shouldRunCheck(
          observation,
          kvalobs::kvChecks(
              10, "QC1-2-101", "QC1-2", 1, "foo", "obs;RR_24;;", "* * * * *",
              boost::posix_time::time_from_string("2010-01-01 00:00:00")),
          expectedParameters));
}

TEST_F(CheckRunnerTest, willNotRunChecksWithoutAnyExpectedParametersForTypeid) {
  db::DatabaseAccess::ParameterList expectedParameters;
  database->getParametersToCheck(&expectedParameters, observation);

  ASSERT_TRUE(expectedParameters.find("TAM") == expectedParameters.end())<< "Check precondition error";

  EXPECT_FALSE(
      runner->shouldRunCheck(
          observation,
          kvalobs::kvChecks(
              10, "QC1-2-101", "QC1-2", 1, "foo", "obs;TAM;;", "* * * * *",
              boost::posix_time::time_from_string("2010-01-01 00:00:00")),
          expectedParameters));
}

TEST_F(CheckRunnerTest, runCheckWithOneButNotAllParametersExpectedForTypeid) {
  db::DatabaseAccess::ParameterList expectedParameters;
  database->getParametersToCheck(&expectedParameters, observation);

  ASSERT_TRUE(expectedParameters.find("RR_24") != expectedParameters.end())<< "Check precondition error";
  ASSERT_TRUE(expectedParameters.find("TAM") == expectedParameters.end())<< "Check precondition error";

  EXPECT_TRUE(
      runner->shouldRunCheck(
          observation,
          kvalobs::kvChecks(
              10, "QC1-2-101", "QC1-2", 1, "foo", "obs;RR_24,TAM;;",
              "* * * * *",
              boost::posix_time::time_from_string("2010-01-01 00:00:00")),
          expectedParameters));
}

TEST_F(CheckRunnerTest, respectsChecksActiveColumn) {
  db::DatabaseAccess::ParameterList expectedParameters;
  database->getParametersToCheck(&expectedParameters, observation);

  ASSERT_TRUE(expectedParameters.find("RR_24") != expectedParameters.end())<< "Check precondition error";

  EXPECT_FALSE(
      runner->shouldRunCheck(
          observation,
          kvalobs::kvChecks(
              10, "QC1-2-101", "QC1-2", 1, "foo", "obs;RR_24;;", "* 13 * * *",
              boost::posix_time::time_from_string("2010-01-01 00:00:00")),
          expectedParameters));
}

TEST_F(CheckRunnerTest, skipChecksForMissingParameters) {
  db::DatabaseAccess::ParameterList expectedParameters;
  database->getParametersToCheck(&expectedParameters, observation);

  ASSERT_TRUE(expectedParameters.find("TA_24") == expectedParameters.end())<< "Check precondition error";

  EXPECT_FALSE(
      runner->shouldRunCheck(
          observation,
          kvalobs::kvChecks(
              10, "QC1-2-101", "QC1-2", 1, "foo", "obs;TA_24;;", "* * * * *",
              boost::posix_time::time_from_string("2010-01-01 00:00:00")),
          expectedParameters));
}

TEST_F(CheckRunnerTest, checkShipsEvenIfParameterMissingInObsPgm) {
  db::DatabaseAccess::ParameterList expectedParameters;
  database->getParametersToCheck(&expectedParameters, observation);

  ASSERT_TRUE(expectedParameters.find("TA_24") == expectedParameters.end())<< "Check precondition error";

  // high station info is a ship.. :/

  EXPECT_TRUE(
      runner->shouldRunCheck(
          kvalobs::kvStationInfo(10000001, observation.obstime(),
                                 observation.typeID()),
          kvalobs::kvChecks(
              10000001, "QC1-2-101", "QC1-2", 1, "foo", "obs;TA_24;;",
              "* * * * *",
              boost::posix_time::time_from_string("2010-01-01 00:00:00")),
          expectedParameters));
}

TEST_F(CheckRunnerTest, skipsChecksWhereAllParametersAreFromOtherTypeId) {
  using namespace testing;

  kvalobs::kvDataFactory factory(
      10, boost::posix_time::time_from_string("2010-05-12 06:00:00"), 304);
  db::DatabaseAccess::DataList dataFromDatabase = boost::assign::list_of(
      factory.getData(6.0, 110));

  // Returned data will have typeid 304
  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(
      SetArgumentPointee<0>(dataFromDatabase));

  EXPECT_CALL(*database, getData(_, observation, qabase::DataRequirement::Parameter("TAM_24"), 0))
      .Times(AtLeast(1));

  db::DatabaseAccess::DataList expectedScriptReturn = boost::assign::list_of(
      factory.getData(6.0, 110));
  expectedScriptReturn.front().controlinfo(
      kvalobs::kvControlInfo("1040003000000000"));
  expectedScriptReturn.front().cfailed("QC1-2-101");  // QC1-2-101 is the default qcx return from fake database

  EXPECT_CALL(*database, write(expectedScriptReturn)).Times(0);

  runner->newObservation(observation);
}
