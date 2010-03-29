/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: checkrunnertest.cc,v 1.1.2.5 2007/09/27 09:02:21 paule Exp $

 Copyright (C) 2007 met.no

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
#include "../checkrunner.h"
#include "database/kvalobsdatabase.h"
#include <string>
#include <fstream>
#include <iterator>
#include <kvdb/kvdb.h>
#include <vector>
#include <set>
#include <kvalobs/kvDataOperations.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>

using namespace std;
using namespace kvalobs;
using namespace boost::filesystem;

namespace
{
std::string getInitSqlStatement()
{
	ifstream f(CHECKRUNNERTEST_INIT_SQL);
	if ( ! f )
	throw std::runtime_error( "Test file not found: " CHECKRUNNERTEST_INIT_SQL );
	std::string ret;
	getline(f, ret, char_traits<char>::to_char_type(char_traits<char>::eof()));
	return ret;
}
}

class CheckRunnerTest: public testing::Test
{
public:
	const kvalobs::kvStationInfo stationInfo;
	KvalobsDatabase db;

	CheckRunnerTest() :
		stationInfo(42, "2006-05-26 06:00:00", 302)
	{
		static const string init_sql_statement = getInitSqlStatement();
		db.getConnection() ->exec(init_sql_statement);
	}

	void runCheckRunner(const std::string & checkName)
	{
		runCheckRunner(checkName, stationInfo);
	}

	void runCheckRunner(const std::string & checkName,
			const kvalobs::kvStationInfo & si)
	{
		CheckRunner checkRunner(si, *db.getQaBaseConnection(), getLogPath(
				checkName));
		checkRunner();
	}

	template<typename InsertIterator> InsertIterator getData(InsertIterator out)
	{
		typedef std::auto_ptr<dnmi::db::Result> Result;
		Result res(db.getConnection()->execQuery("select * from data"));
		while (res->hasNext())
			out++ = res->next();
		return out;
	}

	string getLogPath(const string & name) const
	{
		static const path baseLogPath("test/var/log/");
		path ret = baseLogPath / name;
		create_directories(ret);
		return ret.native_directory_string();
	}
};

TEST_F(CheckRunnerTest, testNoChecks)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData = f.getData(94, 33);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	// remove all checks from database:
	db.getConnection() ->exec("delete from checks");

	// We merely check that this does not crash or throw any exceptions:
	runCheckRunner(__func__);
}

TEST_F(CheckRunnerTest, testNormal)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData = f.getData(94, 33);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	ASSERT_EQ( size_t( 1 ), result.size() );
	const kvData & outData = result.front();
	EXPECT_TRUE( kvalobs::rejected( outData ) );
	const kvalobs::kvControlInfo ci = outData.controlinfo();
	//   ASSERT_EQ( 1, ci.flag( kvalobs::flag::fqclevel ) ); // This flag is no longer in use
	EXPECT_EQ( 6, ci.flag( kvalobs::flag::fr ) );

	kvalobs::kvUseInfo ui;
	ui.setUseFlags( ci );
	EXPECT_EQ( ui, outData.useinfo() );
}

namespace
{
kvalobs::compare::exactly_equal eq_;
}

TEST_F(CheckRunnerTest, testSkipHqc)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData = f.getData(94, 33);
	kvalobs::hqc::hqc_accept(inData);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	ASSERT_EQ( size_t( 1 ), result.size() );
	const kvData & outData = result.front();

	//  EXPECT_TRUE( kvalobs::hqc::hqc_accepted( outData ) );
	EXPECT_TRUE( eq_(inData, outData) );
}

TEST_F(CheckRunnerTest, testSkipAggregated)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", -302);
	kvalobs::kvData inData = f.getData(94, 33);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	{
		kvalobs::kvStationInfo si(stationInfo.stationID(),
				stationInfo.obstime(), -stationInfo.typeID());
		CheckRunner checkRunner(si, *db.getQaBaseConnection(), getLogPath(
				__func__));
		checkRunner();
	}

	vector<kvData> result;
	getData(back_inserter(result));

	ASSERT_EQ( size_t( 1 ), result.size() );
	const kvData & outData = result.front();

	EXPECT_TRUE( eq_( inData, outData ) );
}

namespace
{
void setQC1HasRun(kvData & d, flag::ControlFlag f)
{
	kvalobs::kvControlInfo ci = d.controlinfo();
	ci.set(f, 1);
	kvalobs::kvUseInfo ui = d.useinfo();
	ui.setUseFlags(ci);
	d.controlinfo(ci);
	d.useinfo(ui);
}

kvData getQC1ModifiedData(int val, int param, kvDataFactory & f,
		flag::ControlFlag fl = flag::fpos)
{
	kvData ret = f.getData(val, param);
	setQC1HasRun(ret, fl);
	return ret;
}
}

TEST_F(CheckRunnerTest, testSkipPreChecked)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData = getQC1ModifiedData(94, 33, f);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	ASSERT_EQ( size_t( 1 ), result.size() );
	const kvData & outData = result.front();

	EXPECT_EQ( inData.controlinfo(), outData.controlinfo() );
	EXPECT_EQ( inData.useinfo(), outData.useinfo() );
	EXPECT_TRUE( eq_( inData, outData ) );
}

TEST_F(CheckRunnerTest, testNoSkipedIfOneNotChecked)
{
	typedef set<kvData, compare::lt_kvData> Container;

	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	Container dl;

	dl.insert(getQC1ModifiedData(94, 33, f));
	dl.insert(getQC1ModifiedData(23, 110, f));
	dl.insert(getQC1ModifiedData(1, 112, f));
	for (Container::iterator it = dl.begin(); it != dl.end(); ++it)
		db.getConnection() ->exec("insert into data values " + it->toSend());
	kvData unchecked = f.getData(42, 42);
	db.getConnection() ->exec("insert into data values " + unchecked.toSend());

	runCheckRunner(__func__);

	Container result;
	getData(inserter(result, result.begin()));

	ASSERT_EQ( size_t( 4 ), result.size() );

	for ( Container::const_iterator inData = dl.begin(); inData != dl.end(); ++ inData )
	{
		Container::const_iterator outData = result.find( * inData );
		EXPECT_TRUE( outData != result.end() );
		EXPECT_TRUE( kvControlInfo() != outData->controlinfo() );
		//CPPUNIT_ASSERT_ASSERTION_FAIL(
		//  ASSERT_EQ( kvControlInfo(), outData->controlinfo() )
		//);
		// We assume the bogus checks in the test input was wrong:
		EXPECT_FALSE( inData->controlinfo() == outData->controlinfo() );
	}
	// See if previously unchecked data has been rechecked
	Container::const_iterator outData = result.find( unchecked );
	ASSERT_TRUE( outData != result.end() );
	EXPECT_EQ( 7, outData->useinfo().flag( 0 ) ) << "Data has wrong controlinfo/useinfo flags: " << outData->toSend();
}

TEST_F(CheckRunnerTest, testReCheckResetsFlags)
{
	flag::ControlFlag originalFlag = flag::fcc;

	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData = getQC1ModifiedData(94, 33, f, originalFlag);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	{
		CheckRunner checkRunner(stationInfo, *db.getQaBaseConnection(),
				getLogPath(__func__));
		checkRunner(true);
	}

	vector<kvData> result;
	getData(back_inserter(result));

	ASSERT_EQ( size_t( 1 ), result.size() );
	const kvData & outData = result.front();

	EXPECT_EQ( 0, outData.controlinfo().flag( originalFlag ) );
}

TEST_F(CheckRunnerTest, testOriginalValueIsInput)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData = f.getData(94, 33);
	inData.corrected(2); // This corrected value won't be rejected
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	ASSERT_EQ( size_t( 1 ), result.size() );
	const kvData & outData = result.front();
	EXPECT_TRUE( kvalobs::rejected( outData ) );
}

TEST_F(CheckRunnerTest, testCorrectedValueWhenRejected)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData = f.getData(94, 33);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	ASSERT_EQ( size_t( 1 ), result.size() );
	const kvData & outData = result.front();
	EXPECT_TRUE( kvalobs::rejected( outData ) );
	EXPECT_EQ( float( -32766 ), outData.corrected() );
}

TEST_F(CheckRunnerTest, testInsertModelValueSetsCorrectUseinfo)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData = f.getMissing(110);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	ASSERT_EQ( size_t( 1 ), result.size() );
	const kvData & outData = result.front();

	EXPECT_TRUE( kvalobs::original_missing( outData ) );
	EXPECT_TRUE( kvalobs::valid( outData ) );

	kvUseInfo real_ui = outData.useinfo();
	kvUseInfo correct_ui;
	correct_ui.setUseFlags( outData.controlinfo() );
	EXPECT_EQ( correct_ui, real_ui );
}

TEST_F(CheckRunnerTest, testAllTypeidAreInputData)
{
	kvDataFactory f1(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData1 = f1.getData(1.0, 111);
	db.getConnection() ->exec("insert into data values " + inData1.toSend());

	kvDataFactory f2(42, "2006-05-26 06:00:00", 322);
	kvalobs::kvData inData2 = f2.getData(2.0, 110);
	db.getConnection() ->exec("insert into data values " + inData2.toSend());

	runCheckRunner(__func__);

	typedef std::set<kvalobs::kvData, kvalobs::compare::lt_kvData>
			ResultContainer;
	ResultContainer result;
	getData(inserter(result, result.begin()));

	ASSERT_EQ( size_t( 2 ), result.size() );

	ResultContainer::const_iterator it = result.find( inData2 );
	EXPECT_TRUE( it != result.end() );
	EXPECT_EQ( kvControlInfo( "1000300000000000" ), it->controlinfo() );
}

TEST_F(CheckRunnerTest, testPrefersCurrentTypeID)
{
	// Warning:
	// This test may incorrectly pass if the underlying database happens to deliver
	// inData1 to the checks before inData2.

	kvDataFactory f1(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData inData1 = f1.getData(1.0, 110);
	db.getConnection() ->exec("insert into data values " + inData1.toSend());

	kvDataFactory f2(42, "2006-05-26 06:00:00", 300);
	kvalobs::kvData inData2 = f2.getData(2.0, 110);
	db.getConnection() ->exec("insert into data values " + inData2.toSend());

	kvDataFactory f3(42, "2006-05-26 06:00:00", 307);
	kvalobs::kvData inData3 = f3.getData(2.0, 110);
	db.getConnection() ->exec("insert into data values " + inData3.toSend());

	runCheckRunner(__func__);

	typedef std::set<kvalobs::kvData, kvalobs::compare::lt_kvData>
			ResultContainer;
	ResultContainer result;
	getData(inserter(result, result.begin()));

	ASSERT_EQ( size_t( 3 ), result.size() );

	ResultContainer::const_iterator it = result.find( inData1 );
	EXPECT_TRUE( it != result.end() );
	EXPECT_EQ( kvControlInfo( "1000300000000000" ), it->controlinfo() );

	it = result.find( inData2 );
	ASSERT_TRUE( it != result.end() );
	EXPECT_EQ( kvControlInfo(), it->controlinfo() );

	it = result.find( inData3 );
	ASSERT_TRUE( it != result.end() );
	EXPECT_EQ( kvControlInfo(), it->controlinfo() );
}

namespace
{
struct hasTypeId: std::unary_function<kvalobs::kvData, bool>
{
	const int wanted;
	hasTypeId(int wantedTypeId) :
		wanted(wantedTypeId)
	{
	}
	bool operator ()(const kvalobs::kvData & d)
	{
		return d.typeID() == wanted;
	}
};
}

TEST_F(CheckRunnerTest, testHandlesSpecifiedTypeidInChecks)
{
	// This differs from testPrefersCurrentTypeID in that the checks table
	// specifies that the checks for paramid 111 only applies to typeid 303.

	kvDataFactory fa(9, "2006-05-26 06:00:00", 102);
	const kvalobs::kvData bogusDataA = fa.getData(0, 111);
	db.getConnection() ->exec("insert into data values " + bogusDataA.toSend());

	kvDataFactory fb(9, "2006-05-26 06:00:00", 303);
	const kvalobs::kvData inData = fb.getData(100000, 111);
	db.getConnection() ->exec("insert into data values " + inData.toSend());

	kvDataFactory fc(9, "2006-05-26 06:00:00", 304);
	const kvalobs::kvData bogusDataB = fc.getData(0, 111);
	db.getConnection() ->exec("insert into data values " + bogusDataB.toSend());

	kvalobs::kvStationInfo si(9, "2006-05-26 06:00:00", 303);
	runCheckRunner(__func__, si);

	vector<kvData> result;
	getData(back_inserter(result));
	ASSERT_EQ( size_t( 3 ), result.size() );

	vector<kvData>::const_iterator find;

	find = std::find_if(result.begin(), result.end(), hasTypeId(303));
	ASSERT_TRUE(find != result.end());
	EXPECT_EQ(4, find->controlinfo().flag(kvalobs::flag::fnum));

	find = std::find_if(result.begin(), result.end(), hasTypeId(102));
	ASSERT_TRUE(find != result.end());
	EXPECT_TRUE(eq_(bogusDataA, * find));

	find = std::find_if(result.begin(), result.end(), hasTypeId(304));
	ASSERT_TRUE(find != result.end());
	EXPECT_TRUE(eq_(bogusDataB, * find));
}

TEST_F(CheckRunnerTest, testDoesNotCheckWhenChecksSpecifyAnotherTypeid)
{
	kvDataFactory fa(9, "2006-05-26 06:00:00", 302);
	const kvalobs::kvData inData = fa.getData(21, 111);
	db.getConnection()->exec("insert into data values " + inData.toSend());

	kvalobs::kvStationInfo si(9, "2006-05-26 06:00:00", 302);
	runCheckRunner(__func__, si);

	vector<kvData> result;
	getData(back_inserter(result));
	ASSERT_EQ( size_t( 1 ), result.size() );

	const kvData & outData = result.front();

	EXPECT_EQ(inData.controlinfo(), outData.controlinfo());
	EXPECT_TRUE(eq_(inData, outData));
}

TEST_F(CheckRunnerTest, testChecksHighLevels)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302, 0, 25);
	kvData d = f.getData(5, 110);
	db.getConnection()->exec("insert into data values " + d.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	//  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
	//    cout << * it << endl;

	ASSERT_EQ( size_t( 1 ), result.size() );
	EXPECT_TRUE( result.front().controlinfo() != kvControlInfo() );
}

TEST_F(CheckRunnerTest, testChecksHighLevelsWhenLowLevelsPresent)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302, 0, 25);
	kvData d = f.getData(5, 110);
	db.getConnection()->exec("insert into data values " + d.toSend());

	kvDataFactory f0(42, "2006-05-26 06:00:00", 302);
	kvData d0 = f0.getData(2, 110);
	db.getConnection()->exec("insert into data values " + d0.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	//  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
	//    cout << * it << endl;

	ASSERT_EQ( size_t( 2 ), result.size() );

	vector<kvData>::const_iterator r = find_if(result.begin(), result.end(),
			std::bind2nd( kvalobs::compare::same_kvData(), d ) );

	ASSERT_TRUE(r != result.end());
	EXPECT_TRUE( r->controlinfo() != kvControlInfo() );
}

TEST_F(CheckRunnerTest, testChecksNonstandardSensor)
{
	kvDataFactory f(42, "2006-05-26 06:00:00", 302, 1, 0);
	kvData d = f.getData(5, 110);
	db.getConnection()->exec("insert into data values " + d.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	//  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
	//    cout << * it << endl;

	ASSERT_EQ( size_t( 1 ), result.size() );
	EXPECT_TRUE( result.front().controlinfo() != kvControlInfo() );
}

TEST_F(CheckRunnerTest, testOnlyUsesSpecificLevel)
{
	kvDataFactory f0(42, "2006-05-26 06:00:00", 302);
	kvData d0a = f0.getData(1, 112);
	db.getConnection()->exec("insert into data values " + d0a.toSend());
	// don't send this to the database:
	kvData d0b = f0.getMissing(42);
	db.getConnection()->exec("insert into data values " + d0b.toSend());

	kvDataFactory f25(42, "2006-05-26 06:00:00", 302, 0, 25);
	kvData d25a = f25.getData(2, 112);
	db.getConnection()->exec("insert into data values " + d25a.toSend());
	kvData d25b = f25.getData(2, 42);
	db.getConnection()->exec("insert into data values " + d25b.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	//  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
	//    cout << * it << endl;

	vector<kvData>::const_iterator L0 = find_if(result.begin(), result.end(),
			std::bind2nd(kvalobs::compare::same_kvData(), d0a));
	ASSERT_TRUE( L0 != result.end() );
	EXPECT_EQ( 8, L0->controlinfo().flag(2) );
	// 9 probably means level 0 data was checked against level 25 data
	// 0 means check was skipped, or that test results were not stored


	vector<kvData>::const_iterator L25 = find_if(result.begin(), result.end(),
			std::bind2nd( kvalobs::compare::same_kvData(), d25a ) );
	ASSERT_TRUE( L25 != result.end() );
	EXPECT_EQ( 2, L25->controlinfo().flag(2) );
}

TEST_F(CheckRunnerTest, testOnlyUsesSpecificSensor)
{
	kvDataFactory f0(42, "2006-05-26 06:00:00", 302);
	kvData d0a = f0.getData(1, 112);
	db.getConnection()->exec("insert into data values " + d0a.toSend());
	// don't send this to the database:
	kvData d0b = f0.getMissing(42);
	db.getConnection()->exec("insert into data values " + d0b.toSend());

	kvDataFactory f1(42, "2006-05-26 06:00:00", 302, 1);
	kvData d1a = f1.getData(2, 112);
	db.getConnection()->exec("insert into data values " + d1a.toSend());
	kvData d1b = f1.getData(2, 42);
	db.getConnection()->exec("insert into data values " + d1b.toSend());

	runCheckRunner(__func__);

	vector<kvData> result;
	getData(back_inserter(result));

	//  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
	//    cout << * it << endl;

	vector<kvData>::const_iterator L0 = find_if(result.begin(), result.end(),
			std::bind2nd(kvalobs::compare::same_kvData(), d0a));
	ASSERT_TRUE( L0 != result.end() );
	EXPECT_EQ( 8, L0->controlinfo().flag(2) );
	// 9 probably means level 0 data was checked against sensor 1 data
	// 0 means check was skipped, or that test results were not stored


	vector<kvData>::const_iterator L1 = find_if(result.begin(), result.end(),
			std::bind2nd( kvalobs::compare::same_kvData(), d1a ) );
	ASSERT_TRUE( L1 != result.end() );
	EXPECT_EQ( 2, L1->controlinfo().flag(2) );
}

TEST_F(CheckRunnerTest, testOnlyUsesSpecificStationWithSpecificLevel)
{
	// station 9, level 25
	kvDataFactory f_s9_l25(9, "2006-05-26 06:00:00", 302, 0, 25);
	kvData d_s9_l25 = f_s9_l25.getData(0, 112);
	db.getConnection()->exec("insert into data values " + d_s9_l25.toSend());

	// station 9, level 0
	kvDataFactory f_s9_l0(9, "2006-05-26 06:00:00", 302);
	kvData d_s9_l0 = f_s9_l0.getData(0, 112);
	db.getConnection()->exec("insert into data values " + d_s9_l0.toSend());

	// station 42, level 25
	kvDataFactory f_s42_l25(42, "2006-05-26 06:00:00", 302, 0, 25);
	kvData d_s42_l25 = f_s42_l25.getData(0, 112);
	db.getConnection()->exec("insert into data values " + d_s42_l25.toSend());

	kvalobs::kvStationInfo si(9, "2006-05-26 06:00:00", 302);
	runCheckRunner(__func__, si);

	vector<kvData> result;
	getData(back_inserter(result));

	//  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
	//    cout << * it << endl;

	ASSERT_EQ( size_t(3), result.size() );

	vector<kvData>::const_iterator find = std::find_if(result.begin(), result.end(),
			std::bind2nd( kvalobs::compare::same_kvData(), d_s9_l25 ) );
	ASSERT_TRUE( find != result.end() );
	EXPECT_EQ( 3, find->controlinfo().flag(5) );

	find = std::find_if(result.begin(), result.end(),
			std::bind2nd( kvalobs::compare::same_kvData(), d_s9_l0 ) );
	ASSERT_TRUE( find != result.end() );
	EXPECT_TRUE( ! find->controlinfo().flag(5) );

	find = std::find_if(result.begin(), result.end(),
			std::bind2nd( kvalobs::compare::same_kvData(), d_s42_l25 ) );
	ASSERT_TRUE( find != result.end() );
	EXPECT_TRUE( ! find->controlinfo().flag(5) );
}
