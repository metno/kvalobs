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
#include <scriptcreate/ScriptResultIdentifier.h>

using qabase::ScriptResultIdentifier;

TEST(ScriptResultIdentifierTest, test)
{
	ScriptResultIdentifier result("X_1_0_corrected");

	EXPECT_EQ("X", result.parameter());
	EXPECT_EQ(1, result.timeIndex());
	EXPECT_EQ(0, result.stationIndex());
	EXPECT_EQ(ScriptResultIdentifier::Corrected, result.correctionType());
}

TEST(ScriptResultIdentifierTest, underscoreInParameter)
{
	ScriptResultIdentifier result("X_12_0_1_flag");

	EXPECT_EQ("X_12", result.parameter());
	EXPECT_EQ(0, result.timeIndex());
	EXPECT_EQ(1, result.stationIndex());
	EXPECT_EQ(ScriptResultIdentifier::Flag, result.correctionType());
}

TEST(ScriptResultIdentifierTest, throwOnSpaceInParameter)
{
	EXPECT_THROW(ScriptResultIdentifier result("X_ 0_0_corrected"), ScriptResultIdentifier::SyntaxError);
}

TEST(ScriptResultIdentifierTest, throwOnUndefinedCorrectionType)
{
	EXPECT_THROW(ScriptResultIdentifier result("X_0_0_foo"), ScriptResultIdentifier::UndefinedCorrectionType);
}
