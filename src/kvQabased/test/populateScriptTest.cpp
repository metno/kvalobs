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
#include <scriptcreate/DataStore.h>
#include <scriptcreate/populateScript.h>
#include <db/returntypes/CheckSignature.h>
#include <scriptrunner/Script.h>
#include <boost/scoped_ptr.hpp>
#include <algorithm>

class populateScriptTest : public testing::Test {
 public:
  populateScriptTest()
      : observation(1, 10, 302,
                    boost::posix_time::time_from_string("2010-05-12 06:00:00"),
                    boost::posix_time::time_from_string("2010-05-12 06:00:00")),
        script("sub check() { print \"ok\\n\"; }",
               scriptrunner::language::Interpreter::get("perl")) {
  }
 protected:
  FakeDatabaseAccess database;
  qabase::Observation observation;
  scriptrunner::Script script;
};

TEST_F(populateScriptTest, scriptGenerate) {
  qabase::CheckSignature abstractRequirement("obs;X,Y;;|meta;M;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement(
      "obs;RR_24,RR_12;10;0,-1440|meta;RR_24_MAX;10;0",
      observation.stationID());
  qabase::DataStore store(database, observation, "QC1-1-1", abstractRequirement,
                          concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();

  EXPECT_EQ(3u, scriptInput.size());  // obs, meta and general

  {
    ScriptInputList::const_iterator obs;
    for (obs = scriptInput.begin(); obs != scriptInput.end(); ++obs)
      if (obs->name() == "obs")
        break;
    ASSERT_TRUE(obs != scriptInput.end());

    EXPECT_EQ(7u, obs->numericList().size());
    typedef scriptrunner::ScriptInput::NumericListParameters NumList;
    const NumList & numericList = obs->numericList();
    EXPECT_TRUE(numericList.find("X") != numericList.end());
    EXPECT_TRUE(numericList.find("X_missing") != numericList.end());
    EXPECT_TRUE(numericList.find("X_controlinfo") != numericList.end());
    EXPECT_TRUE(numericList.find("Y") != numericList.end());
    EXPECT_TRUE(numericList.find("Y_missing") != numericList.end());
    EXPECT_TRUE(numericList.find("Y_controlinfo") != numericList.end());
    EXPECT_TRUE(numericList.find("obs_timeoffset") != numericList.end());
  }
  {
    ScriptInputList::const_iterator meta;
    for (meta = scriptInput.begin(); meta != scriptInput.end(); ++meta)
      if (meta->name() == "meta")
        break;
    ASSERT_TRUE(meta != scriptInput.end());

    scriptrunner::ScriptInput::NumericParameters numeric = meta->numeric();  // non-const copy
    EXPECT_EQ(1, numeric["meta_numtimes"]);
    EXPECT_EQ(0, numeric["meta_missing"]);
    EXPECT_EQ(2u, numeric.size());

    EXPECT_TRUE(meta->strings().empty());

    EXPECT_TRUE(meta->strings().empty());

    EXPECT_EQ(2u, meta->numericList().size());
    typedef scriptrunner::ScriptInput::NumericListParameters NumList;
    const NumList & numericList = meta->numericList();
    EXPECT_TRUE(numericList.find("M") != numericList.end());

    NumList::const_iterator meta_timeoffset = numericList.find(
        "meta_timeoffset");
    ASSERT_TRUE(meta_timeoffset != numericList.end());
    ASSERT_EQ(1u, meta_timeoffset->second.size());
    EXPECT_FLOAT_EQ(0, meta_timeoffset->second.front());
  }
}

TEST_F(populateScriptTest, minimalScript) {
  qabase::CheckSignature abstractRequirement("obs;X;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement("obs;RR_12;;",
                                             observation.stationID());
  qabase::DataStore store(database, observation, "QC1-1-1", abstractRequirement,
                          concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();
  ScriptInputList::const_iterator obs;
  for (obs = scriptInput.begin(); obs != scriptInput.end(); ++obs)
    if (obs->name() == "general")
      break;
  ASSERT_TRUE(obs != scriptInput.end());

  typedef scriptrunner::ScriptInput::NumericListParameters NumList;
  NumList numericList = obs->numericList();
  const scriptrunner::ScriptInput::ValueList & v = numericList["obstime"];
  ASSERT_EQ(6u, v.size());
  boost::posix_time::ptime t(
      boost::gregorian::date(v[0], v[1], v[2]),
      boost::posix_time::time_duration(v[3], v[4], v[5]));
  EXPECT_EQ(observation.obstime(), t);

  scriptrunner::ScriptInput::NumericParameters numeric = obs->numeric();
  EXPECT_FLOAT_EQ(1.15, numeric["station_latitude"]);
  EXPECT_FLOAT_EQ(12.5, numeric["station_longitude"]);
}

TEST_F(populateScriptTest, usesOriginalValueForObs) {
  MockDatabaseAccess mockDatabase;
  mockDatabase.setDefaultActions();
  using namespace testing;

  db::DatabaseAccess::DataList data;
  kvalobs::kvDataFactory factory(observation.stationID(), observation.obstime(),
                                 observation.typeID());
  kvalobs::kvData d = factory.getData(10, 110);
  kvalobs::correct(d, 42);
  data.push_back(d);

  EXPECT_CALL(mockDatabase, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(data));

  qabase::CheckSignature abstractRequirement("obs;R;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement("obs;RR_24;;",
                                             observation.stationID());
  qabase::DataStore store(mockDatabase, observation, "QC1-1-1",
                          abstractRequirement, concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();

  ScriptInputList::const_iterator obs;
  for (obs = scriptInput.begin(); obs != scriptInput.end(); ++obs)
    if (obs->name() == "obs")
      break;
  ASSERT_TRUE(obs != scriptInput.end());

  typedef scriptrunner::ScriptInput::NumericListParameters NumericListParam;
  NumericListParam numList = obs->numericList();

  const scriptrunner::ScriptInput::ValueList & rr24Missing =
      numList["R_missing"];
  ASSERT_EQ(1u, rr24Missing.size());
  EXPECT_EQ(4, rr24Missing.front());

  const scriptrunner::ScriptInput::ValueList & rr24 = numList["R"];
  ASSERT_EQ(1u, rr24.size());
  EXPECT_EQ(10, rr24.front());
}

TEST_F(populateScriptTest, flagsMissingValuesAsMissing) {
  MockDatabaseAccess mockDatabase;
  mockDatabase.setDefaultActions();
  using namespace testing;

  db::DatabaseAccess::DataList data;
  kvalobs::kvDataFactory factory(observation.stationID(), observation.obstime(),
                                 observation.typeID());
  kvalobs::kvData d = factory.getMissing(110);
  kvalobs::correct(d, 42);
  data.push_back(d);

  EXPECT_CALL(mockDatabase, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(data));

  qabase::CheckSignature abstractRequirement("obs;R;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement("obs;RR_24;;",
                                             observation.stationID());
  qabase::DataStore store(mockDatabase, observation, "QC1-1-1",
                          abstractRequirement, concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();

  ScriptInputList::const_iterator obs;
  for (obs = scriptInput.begin(); obs != scriptInput.end(); ++obs)
    if (obs->name() == "obs")
      break;
  ASSERT_TRUE(obs != scriptInput.end());

  typedef scriptrunner::ScriptInput::NumericListParameters NumericListParam;
  NumericListParam numList = obs->numericList();

  const scriptrunner::ScriptInput::ValueList & rr24Missing =
      numList["R_missing"];
  ASSERT_EQ(1u, rr24Missing.size());
  EXPECT_EQ(1, rr24Missing.front());

  const scriptrunner::ScriptInput::ValueList & rr24 = numList["R"];
  ASSERT_EQ(1u, rr24.size());
  EXPECT_EQ(-32767, rr24.front());

  typedef scriptrunner::ScriptInput::NumericParameters NumericParam;
  NumericParam num = obs->numeric();

  NumericParam::const_iterator obs_missing = num.find("obs_missing");
  ASSERT_TRUE(obs_missing != num.end());
  ASSERT_EQ(1, obs_missing->second);
}

TEST_F(populateScriptTest, flagsRejectedValuesAsNotMissing) {
  MockDatabaseAccess mockDatabase;
  mockDatabase.setDefaultActions();
  using namespace testing;

  db::DatabaseAccess::DataList data;
  kvalobs::kvDataFactory factory(observation.stationID(), observation.obstime(),
                                 observation.typeID());
  kvalobs::kvData d = factory.getData(10, 110);
  kvalobs::reject(d);
  data.push_back(d);

  EXPECT_CALL(mockDatabase, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(data));

  qabase::CheckSignature abstractRequirement("obs;R;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement("obs;RR_24;;",
                                             observation.stationID());
  qabase::DataStore store(mockDatabase, observation, "QC1-1-1",
                          abstractRequirement, concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();

  ScriptInputList::const_iterator obs;
  for (obs = scriptInput.begin(); obs != scriptInput.end(); ++obs)
    if (obs->name() == "obs")
      break;
  ASSERT_TRUE(obs != scriptInput.end());

  typedef scriptrunner::ScriptInput::NumericListParameters NumericListParam;
  NumericListParam numList = obs->numericList();

  const scriptrunner::ScriptInput::ValueList & rr24Missing =
      numList["R_missing"];
  ASSERT_EQ(1u, rr24Missing.size());
  EXPECT_EQ(2, rr24Missing.front());

  const scriptrunner::ScriptInput::ValueList & rr24 = numList["R"];
  ASSERT_EQ(1u, rr24.size());
  EXPECT_EQ(10, rr24.front());

  typedef scriptrunner::ScriptInput::NumericParameters NumericParam;
  NumericParam num = obs->numeric();

  NumericParam::const_iterator obs_missing = num.find("obs_missing");
  ASSERT_TRUE(obs_missing != num.end());
  ASSERT_EQ(0, obs_missing->second);
}

TEST_F(populateScriptTest, missingRefobs) {
  qabase::CheckSignature abstractRequirement("obs;R;;|refobs;Rstart,Robs;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement(
      "obs;RR_X;;|refobs;KLSTART,KLOBS;;", observation.stationID());
  qabase::DataStore store(database, observation, "QC1-1-1", abstractRequirement,
                          concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();

  ScriptInputList::const_iterator refobs;
  for (refobs = scriptInput.begin(); refobs != scriptInput.end(); ++refobs)
    if (refobs->name() == "refobs")
      break;
  ASSERT_TRUE(refobs != scriptInput.end());

  typedef scriptrunner::ScriptInput::StringListParameters StringListParam;
  StringListParam stringList = refobs->stringList();
  const scriptrunner::ScriptInput::StringList & rstart = stringList["Rstart"];
  ASSERT_EQ(1u, rstart.size());
  EXPECT_EQ("", rstart.front());

  const scriptrunner::ScriptInput::StringList & robs = stringList["Robs"];
  ASSERT_EQ(1u, robs.size());
  EXPECT_EQ("", robs.front());

  typedef scriptrunner::ScriptInput::NumericListParameters NumericListParam;
  NumericListParam numList = refobs->numericList();
  const scriptrunner::ScriptInput::ValueList & rstartMissing =
      numList["Rstart_missing"];
  ASSERT_EQ(1u, rstartMissing.size());
  EXPECT_EQ(3, rstartMissing.front());

  const scriptrunner::ScriptInput::ValueList & robsMissing =
      numList["Robs_missing"];
  ASSERT_EQ(1u, robsMissing.size());
  EXPECT_EQ(3, robsMissing.front());
}

TEST_F(populateScriptTest, existingRefObs) {
  MockDatabaseAccess mockDatabase;
  mockDatabase.setDefaultActions();
  using namespace testing;

  db::DatabaseAccess::TextDataList klstart;
  klstart.push_back(
      kvalobs::kvTextData(observation.stationID(), observation.obstime(),
                          "2010010106", 1021, boost::posix_time::ptime(),
                          observation.typeID()));
  EXPECT_CALL(mockDatabase, getTextData(_, observation.stationInfo(), qabase::DataRequirement::Parameter("KLSTART"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(klstart));

  db::DatabaseAccess::TextDataList klobs;
  klobs.push_back(
      kvalobs::kvTextData(observation.stationID(), observation.obstime(),
                          "2010050106", 1022, boost::posix_time::ptime(),
                          observation.typeID()));
  EXPECT_CALL(mockDatabase, getTextData(_, observation.stationInfo(), qabase::DataRequirement::Parameter("KLOBS"), 0))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(klobs));

  qabase::CheckSignature abstractRequirement("obs;R;;|refobs;Rstart,Robs;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement(
      "obs;RR_X;;|refobs;KLSTART,KLOBS;;", observation.stationID());
  qabase::DataStore store(mockDatabase, observation, "QC1-1-1",
                          abstractRequirement, concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();

  ScriptInputList::const_iterator refobs;
  for (refobs = scriptInput.begin(); refobs != scriptInput.end(); ++refobs)
    if (refobs->name() == "refobs")
      break;
  ASSERT_TRUE(refobs != scriptInput.end());

  typedef scriptrunner::ScriptInput::StringListParameters StringListParam;
  StringListParam stringList = refobs->stringList();
  const scriptrunner::ScriptInput::StringList & rstart = stringList["Rstart"];
  ASSERT_EQ(1u, rstart.size());
  EXPECT_EQ("2010010106", rstart.front());

  const scriptrunner::ScriptInput::StringList & robs = stringList["Robs"];
  ASSERT_EQ(1u, robs.size());
  EXPECT_EQ("2010050106", robs.front());

  typedef scriptrunner::ScriptInput::NumericListParameters NumericListParam;
  NumericListParam numList = refobs->numericList();
  const scriptrunner::ScriptInput::ValueList & rstartMissing =
      numList["Rstart_missing"];
  ASSERT_EQ(1u, rstartMissing.size());
  EXPECT_EQ(0, rstartMissing.front());

  const scriptrunner::ScriptInput::ValueList & robsMissing =
      numList["Robs_missing"];
  ASSERT_EQ(1u, robsMissing.size());
  EXPECT_EQ(0, robsMissing.front());
}

TEST_F(populateScriptTest, oneRefObsNonexisting) {
  MockDatabaseAccess mockDatabase;
  mockDatabase.setDefaultActions();
  using namespace testing;

  db::DatabaseAccess::TextDataList klstart;
  boost::posix_time::ptime t = observation.obstime();
  klstart.push_back(
      kvalobs::kvTextData(observation.stationID(), t, "2010010206", 1021,
                          boost::posix_time::ptime(), observation.typeID()));
  t -= boost::posix_time::hours(1);
  klstart.push_back(
      kvalobs::kvTextData(observation.stationID(), t, "2010010106", 1021,
                          boost::posix_time::ptime(), observation.typeID()));
  EXPECT_CALL(mockDatabase, getTextData(_, observation.stationInfo(), qabase::DataRequirement::Parameter("KLSTART"), -60))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(klstart));

  db::DatabaseAccess::TextDataList klobs;
  klobs.push_back(
      kvalobs::kvTextData(observation.stationID(), observation.obstime(),
                          "2010050106", 1022, boost::posix_time::ptime(),
                          observation.typeID()));
  EXPECT_CALL(mockDatabase, getTextData(_, observation.stationInfo(), qabase::DataRequirement::Parameter("KLOBS"), -60))
      .Times(AtLeast(1)).WillRepeatedly(SetArgumentPointee<0>(klobs));

  qabase::CheckSignature abstractRequirement("obs;R;;|refobs;Rstart,Robs;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement(
      "obs;RR_X;;|refobs;KLSTART,KLOBS;;0,-60", observation.stationID());
  qabase::DataStore store(mockDatabase, observation, "QC1-1-1",
                          abstractRequirement, concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();

  ScriptInputList::const_iterator refobs;
  for (refobs = scriptInput.begin(); refobs != scriptInput.end(); ++refobs)
    if (refobs->name() == "refobs")
      break;
  ASSERT_TRUE(refobs != scriptInput.end());

  typedef scriptrunner::ScriptInput::StringListParameters StringListParam;
  StringListParam stringList = refobs->stringList();
  const scriptrunner::ScriptInput::StringList & rstart = stringList["Rstart"];
  ASSERT_EQ(2u, rstart.size());
  EXPECT_EQ("2010010206", rstart.front());
  EXPECT_EQ("2010010106", rstart.back());

  const scriptrunner::ScriptInput::StringList & robs = stringList["Robs"];
  ASSERT_EQ(2u, robs.size());
  EXPECT_EQ("2010050106", robs.front());
  EXPECT_EQ("-32767", robs.back());

  typedef scriptrunner::ScriptInput::NumericListParameters NumericListParam;
  NumericListParam numList = refobs->numericList();
  const scriptrunner::ScriptInput::ValueList & rstartMissing =
      numList["Rstart_missing"];
  ASSERT_EQ(2u, rstartMissing.size());
  EXPECT_EQ(0, rstartMissing.front());
  EXPECT_EQ(0, rstartMissing.back());

  const scriptrunner::ScriptInput::ValueList & robsMissing =
      numList["Robs_missing"];
  ASSERT_EQ(2u, robsMissing.size());
  EXPECT_EQ(0, robsMissing.front());
  EXPECT_EQ(3, robsMissing.back());
}

TEST_F(populateScriptTest, metaData) {
  qabase::CheckSignature abstractRequirement("obs;X;;|meta;M,N;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement(
      "obs;RR_24;;|meta;RR_24_MIN,RR_24_MAX;;", observation.stationID());
  qabase::DataStore store(database, observation, "QC1-1-1", abstractRequirement,
                          concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();

  ScriptInputList::const_iterator meta;
  for (meta = scriptInput.begin(); meta != scriptInput.end(); ++meta)
    if (meta->name() == "meta")
      break;
  ASSERT_TRUE(meta != scriptInput.end());

  scriptrunner::ScriptInput::NumericParameters numeric = meta->numeric();  // non-const copy
  EXPECT_EQ(1, numeric["meta_numtimes"]);
  EXPECT_EQ(0, numeric["meta_missing"]);
  EXPECT_EQ(2u, numeric.size());

  EXPECT_TRUE(meta->strings().empty());

  EXPECT_EQ(3u, meta->numericList().size());
  typedef scriptrunner::ScriptInput::NumericListParameters NumList;
  const NumList & numericList = meta->numericList();
  NumList::const_iterator m = numericList.find("M");
  ASSERT_TRUE(m != numericList.end());
  ASSERT_EQ(1u, m->second.size());
  EXPECT_FLOAT_EQ(1.1, m->second.front());

  NumList::const_iterator n = numericList.find("N");
  ASSERT_TRUE(n != numericList.end());
  ASSERT_EQ(1u, n->second.size());
  EXPECT_FLOAT_EQ(2.2, n->second.front());

  NumList::const_iterator meta_timeoffset = numericList.find("meta_timeoffset");
  ASSERT_TRUE(meta_timeoffset != numericList.end());
  ASSERT_EQ(1u, meta_timeoffset->second.size());
  EXPECT_FLOAT_EQ(0, meta_timeoffset->second.front());
}

TEST_F(populateScriptTest, getsValuesFromDb) {
  qabase::CheckSignature abstractRequirement("obs;X;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement("obs;RR_12;10;0,-1440",
                                             observation.stationID());
  qabase::DataStore store(database, observation, "QC1-1-1", abstractRequirement,
                          concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();
  ScriptInputList::const_iterator obs;
  for (obs = scriptInput.begin(); obs != scriptInput.end(); ++obs)
    if (obs->name() == "obs")
      break;
  ASSERT_TRUE(obs != scriptInput.end());

  typedef scriptrunner::ScriptInput::NumericListParameters NumList;
  NumList numericList = obs->numericList();
  EXPECT_EQ(3u, numericList["X"].size());
  EXPECT_EQ(3u, numericList["X_missing"].size());
  EXPECT_EQ(3u * 16u, numericList["X_controlinfo"].size());
}

TEST_F(populateScriptTest, modelData) {
  qabase::CheckSignature abstractRequirement("obs;X;;|model;mX;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement(
      "obs;RR_12&0&&;;|model;RR_12&0&&;;", observation.stationID());
  qabase::DataStore store(database, observation, "QC1-1-1", abstractRequirement,
                          concreteRequirement);
  qabase::addDataToScript(script, store);

  typedef std::vector<scriptrunner::Script::Input> ScriptInputList;
  const ScriptInputList & scriptInput = script.getInput();
  ScriptInputList::const_iterator model;
  for (model = scriptInput.begin(); model != scriptInput.end(); ++model)
    if (model->name() == "model")
      break;
  ASSERT_TRUE(model != scriptInput.end());

  scriptrunner::ScriptInput::NumericParameters numeric = model->numeric();  // non-const copy
  EXPECT_EQ(1, numeric["model_numtimes"]);
  EXPECT_EQ(0, numeric["model_missing"]);
  EXPECT_EQ(2u, numeric.size());

  typedef scriptrunner::ScriptInput::NumericListParameters NumList;
  NumList numericList = model->numericList();
  EXPECT_EQ(3u, numericList.size());
  ASSERT_EQ(1u, numericList["mX"].size());
  EXPECT_FLOAT_EQ(42.1, numericList["mX"].front());
  ASSERT_EQ(1u, numericList["mX_missing"].size());
  EXPECT_EQ(0, numericList["mX_missing"].front());
  ASSERT_EQ(1u, numericList["model_timeoffset"].size());
  EXPECT_EQ(0, numericList["mX_missing"].front());
}

TEST_F(populateScriptTest, nonmatchingSignatures) {
  qabase::CheckSignature abstractRequirement("obs;X,Y;;|meta;M;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement(
      "obs;RR_24;10;0,-1440|meta;RR_24_MAX;10;0", observation.stationID());

  EXPECT_THROW(
      qabase::DataStore store(database, observation, "QC1-1-1", abstractRequirement, concreteRequirement),
      qabase::NonmatchingDataRequirements);
}

TEST_F(populateScriptTest, missingSignatureInChecks) {
  qabase::CheckSignature abstractRequirement("obs;X,Y;;|meta;M;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement("meta;RR_24_MAX;10;0",
                                             observation.stationID());

  EXPECT_THROW(
      qabase::DataStore store(database, observation, "QC1-1-1",abstractRequirement, concreteRequirement),
      qabase::DataStore::UnableToGetData);
}

TEST_F(populateScriptTest, missingSignatureInAlgorithm) {
  qabase::CheckSignature abstractRequirement("meta;M;;",
                                             observation.stationID());
  qabase::CheckSignature concreteRequirement(
      "obs;RR_24;10;0,-1440|meta;RR_24_MAX;10;0", observation.stationID());

  EXPECT_THROW(
      qabase::DataStore store(database, observation, "QC1-1-1",abstractRequirement, concreteRequirement),
      qabase::DataStore::UnableToGetData);
}

