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
#include <db/returntypes/CheckSignature.h>

using qabase::CheckSignature;
using qabase::DataRequirement;

TEST(CheckSignatureTest, test) {
  CheckSignature s("obs;X;;|refobs;rX;;|model;mX;;|meta;X_MID,X_STD;;", 100);

  const DataRequirement * obs = s.obs();
  ASSERT_TRUE(obs != 0);
  EXPECT_EQ("obs", obs->requirementType());
  EXPECT_FALSE(obs->empty());

  const DataRequirement * refobs = s.refobs();
  ASSERT_TRUE(refobs != 0);
  EXPECT_EQ("refobs", refobs->requirementType());
  EXPECT_FALSE(refobs->empty());

  const DataRequirement * model = s.model();
  ASSERT_TRUE(model != 0);
  EXPECT_EQ("model", model->requirementType());
  EXPECT_FALSE(model->empty());

  const DataRequirement * meta = s.meta();
  ASSERT_TRUE(meta != 0);
  EXPECT_EQ("meta", meta->requirementType());
  EXPECT_FALSE(meta->empty());
}

TEST(CheckSignatureTest, missingModelSpec) {
  CheckSignature s("obs;X;;|refobs;rX;;||meta;X_MID,X_STD;;", 100);

  const DataRequirement * model = s.model();
  ASSERT_TRUE(model == 0);

  const DataRequirement * meta = s.meta();
  ASSERT_TRUE(meta != 0);
  EXPECT_EQ("meta", meta->requirementType());
  EXPECT_FALSE(meta->empty());
}
