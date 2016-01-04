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
#include <db/DelayedSaveDatabaseAccess.h>

class DelayedSaveDatabaseAccessTest : public testing::Test {
 public:
  DelayedSaveDatabaseAccessTest()
      : observation(10,
                    boost::posix_time::time_from_string("2010-05-26 06:00:00"),
                    302) {
  }
 protected:
  FakeDatabaseAccess fakeDb;
  db::DelayedSaveDatabaseAccess * database;
  kvalobs::kvStationInfo observation;

  virtual void SetUp() {
    database = new db::DelayedSaveDatabaseAccess(&fakeDb);
  }
  virtual void TearDown() {
    delete database;
  }
};

TEST_F(DelayedSaveDatabaseAccessTest, onlySaveOnCommit) {
  kvalobs::kvDataFactory f(
      10, boost::posix_time::time_from_string("2010-05-26 06:00:00"), 302);

  db::DatabaseAccess::DataList toSave;
  toSave.push_back(f.getData(42.0, 110));
  toSave.push_back(f.getData(21.0, 109));

  database->write(toSave);

  EXPECT_TRUE(fakeDb.savedData.empty());

  database->commit();

  EXPECT_EQ(2u, fakeDb.savedData.size());

  for (db::DatabaseAccess::DataList::const_iterator it = toSave.begin();
      it != toSave.end(); ++it) {
    FakeDatabaseAccess::SavedData::const_iterator find = fakeDb.savedData.find(
        *it);
    EXPECT_FALSE( find == fakeDb.savedData.end() ) << "Unable to find data: "
                                                   << *it;
    if (find != fakeDb.savedData.end())
      EXPECT_TRUE(kvalobs::compare::exactly_equal()(* find, * it ))
          << *find << " is different from " << *it;
  }
}

TEST_F(DelayedSaveDatabaseAccessTest, rereadSavedData) {
  db::DatabaseAccess::DataList data;
  database->getData(&data, observation, "RR_24", 0);

  ASSERT_EQ(1u, data.size());

  data.front().corrected(1000);

  database->write(data);

  db::DatabaseAccess::DataList secondRead;
  database->getData(&secondRead, observation, "RR_24", 0);
  ASSERT_EQ(1u, secondRead.size());
  EXPECT_EQ(1000, secondRead.front().corrected());

  EXPECT_TRUE(
      kvalobs::compare::exactly_equal()(secondRead.front(), data.front()));
}

TEST_F(DelayedSaveDatabaseAccessTest, moultipleSavesOfSameData) {
  db::DatabaseAccess::DataList data;
  database->getData(&data, observation, "RR_24", 0);

  ASSERT_EQ(1u, data.size());

  data.front().corrected(500);
  database->write(data);
  data.front().corrected(800);
  database->write(data);
  data.front().corrected(2000);
  database->write(data);

  db::DatabaseAccess::DataList secondRead;
  database->getData(&secondRead, observation, "RR_24", 0);
  ASSERT_EQ(1u, secondRead.size());
  EXPECT_EQ(2000, secondRead.front().corrected());

  EXPECT_TRUE(
      kvalobs::compare::exactly_equal()(secondRead.front(), data.front()));
}
