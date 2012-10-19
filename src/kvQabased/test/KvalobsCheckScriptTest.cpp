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
#include "MockDatabaseAccess.h"
#include <scriptcreate/KvalobsCheckScript.h>
#include <scriptcreate/DataStore.h>
#include <scriptrunner/Script.h>
#include <boost/scoped_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include <algorithm>

using qabase::KvalobsCheckScript;


class KvalobsCheckScriptTest: public testing::Test
{
public:
	KvalobsCheckScriptTest() :
		observation(10, "2010-05-12 06:00:00", 302),
		factory    (10, "2010-05-12 06:00:00", 302)
	{
		database.setDefaultActions();
	}
protected:
	MockDatabaseAccess database;
	kvalobs::kvStationInfo observation;
	kvalobs::kvDataFactory factory;
};

namespace
{
kvalobs::kvChecks makeCheck(const std::string & signature, const std::string & name = "test_check_1")
{
	return kvalobs::kvChecks(10, "QC1-2-101", "QC1-2", 1, name, signature, "* * * * *", "2010-01-01 00:00:00");
}

kvalobs::kvAlgorithms makeAlgorithm(const std::string & signature, const std::string & script, const std::string & name = "test_check_1")
{
	return kvalobs::kvAlgorithms(1, name, signature, script);
}

std::string simpleScript =
		"sub check() {\n"
		" my @retvector;\n"
		" push(@retvector, \"X_0_0_corrected\");\n"
		" push(@retvector, $X[0]*2);\n"
		" my $numout = @retvector;\n"
		" return(@retvector,$numout);\n"
		"}\n";

std::string simpleFlagScript =
		"sub check() {\n"
		" my @retvector;\n"
		" push(@retvector, \"X_0_0_flag\");\n"
		" push(@retvector, $X[0]);\n"
		" push(@retvector, \"X_0_0_corrected\");\n"
		" push(@retvector, $X[0]*2);\n"
		" my $numout = @retvector;\n"
		" return(@retvector,$numout);\n"
		"}\n";
}

std::string simpleRejectScript =
		"sub check() {\n"
		" my @retvector;\n"
		" push(@retvector, \"X_0_0_missing\");\n"
		" push(@retvector, 2);\n"
		" my $numout = @retvector;\n"
		" return(@retvector,$numout);\n"
		"}\n";

TEST_F(KvalobsCheckScriptTest, runCheck)
{
	using namespace testing;

	EXPECT_CALL(database, getAlgorithm(__func__))
			.Times(AtLeast(1))
			.WillRepeatedly(Return(makeAlgorithm("obs;X;;", simpleScript, __func__)));

	KvalobsCheckScript script(database, observation, makeCheck("obs;RR_24;;", __func__));

	db::DatabaseAccess::DataList modifications;
	script.run(& modifications);

	ASSERT_EQ(1u, modifications.size());
	EXPECT_EQ(12, modifications.front().corrected());
}

TEST_F(KvalobsCheckScriptTest, missingFlagUpdatesCorrected)
{
	using namespace testing;

	EXPECT_CALL(database, getAlgorithm(__func__))
			.Times(AtLeast(1))
			.WillRepeatedly(Return(makeAlgorithm("obs;X;;", simpleRejectScript, __func__)));

	KvalobsCheckScript script(database, observation, makeCheck("obs;RR_24;;", __func__));

	db::DatabaseAccess::DataList modifications;
	script.run(& modifications);

	ASSERT_EQ(1u, modifications.size());
	EXPECT_EQ(-32766, modifications.front().corrected());
	EXPECT_TRUE(kvalobs::rejected(modifications.front()));
}


TEST_F(KvalobsCheckScriptTest, setsGreatestFlag)
{
	using namespace testing;

	EXPECT_CALL(database, getAlgorithm(__func__))
			.Times(AtLeast(1))
			.WillRepeatedly(Return(makeAlgorithm("obs;X;;", simpleFlagScript, __func__)));

	kvalobs::kvDataFactory factory(10, "2010-05-25 06:00:00", 302);
	db::DatabaseAccess::DataList dataList;
	kvalobs::kvData data = factory.getData(1, 110);
	data.controlinfo(kvalobs::kvControlInfo("0050000000000000"));
	dataList.push_back(data);
	EXPECT_CALL(database, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
				.Times(AtLeast(1))
				.WillRepeatedly(SetArgumentPointee<0>(dataList));

	KvalobsCheckScript script(database, observation, makeCheck("obs;RR_24;;", __func__));
	db::DatabaseAccess::DataList modifications;
	script.run(& modifications);

	ASSERT_EQ(1u, modifications.size());
	EXPECT_EQ(kvalobs::kvControlInfo("0050000000000000"), modifications.front().controlinfo());
}

TEST_F(KvalobsCheckScriptTest, setsFlagInCorrectPosition)
{
	using namespace testing;

	EXPECT_CALL(database, getAlgorithm(__func__))
			.Times(AtLeast(1))
			.WillRepeatedly(Return(makeAlgorithm("obs;X;;", simpleFlagScript, __func__)));

	kvalobs::kvDataFactory factory(10, "2010-05-25 06:00:00", 302);
	db::DatabaseAccess::DataList dataList;
	dataList.push_back(factory.getData(4, 110));
	EXPECT_CALL(database, getData(_, observation, qabase::DataRequirement::Parameter("RR_24"), 0))
				.Times(AtLeast(1))
				.WillRepeatedly(SetArgumentPointee<0>(dataList));

	KvalobsCheckScript script(database, observation,
			kvalobs::kvChecks(observation.stationID(), "QC1-3-101", "QC1-3", 1, __func__, "obs;RR_24;;", "* * * * *", "2010-01-01 00:00:00"));
	db::DatabaseAccess::DataList modifications;
	script.run(& modifications);

	ASSERT_EQ(1u, modifications.size());
	EXPECT_EQ(8, modifications.front().corrected());
	EXPECT_EQ(kvalobs::kvControlInfo("0004000000000000"), modifications.front().controlinfo());
}
