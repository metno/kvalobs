/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: synop.h,v 1.12.2.5 2007/09/27 09:02:18 paule Exp $

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

#include <float.h>
#include <limits.h>
#include <string>
#include <kvalobs/kvData.h>
#include <list>
#include <kvalobs/kvStation.h>
#include "kvSynopDecoder.h"

#include <gtest/gtest.h>

using namespace std;
using namespace miutil;

namespace {
struct ParamVal {
	int paramid;
	float *val;
};

}

class SynopDecodeTest : public testing::Test
{

protected:
	kvSynopDecoder synopDecoder;

	///Called before each test case.
	virtual void SetUp() {
		//Populate a station list and initialize the synop decoder with it.
		list<kvalobs::kvStation> stationList;

		/*
		 *     kvStation( int st, float la, float lo, float he, float max,
		 *                const std::string& na, int wm, int nn,
		 *                const std::string& ic, const std::string& ca,
		 *                const std::string& ss, int environmentid,
		 *                bool static_, const miutil::miTime& fromtime)
		 *
		 */
		stationList.push_back( kvalobs::kvStation(4460, 60.1173, 10.829, 170, 0,"Hakadal jernbanestasjon", 1488, 4460, "", "","",8, true,
				                                  boost::posix_time::time_from_string("2007-01-08 00:00:00")));

		stationList.push_back( kvalobs::kvStation(76900, 66, 2, 6, 0,"MIKE", 0, 76900, "", "LDWR","",7, true,
				boost::posix_time::time_from_string("1977-01-01 00:00:00")));

		stationList.push_back( kvalobs::kvStation( 40745, 60.1173, 10.829, 170, 0,"Test1", 40745, 40745, "", "","",8, true,
				boost::posix_time::time_from_string("2007-01-08 00:00:00")));


		ASSERT_TRUE( synopDecoder.initialise( stationList, 10, 20 ) ) << "Cant initialize the synopdecoder.";

	}

	///Called after each test case.
	virtual void TearDown() {

	}

	bool decode_h_VV_N( const std::string &synop, int &h, int &VV, int &N, kvalobs::kvRejectdecode &rejectInfo ) {
		list<kvalobs::kvData> dataList;

		if( ! synopDecoder.decode( synop, dataList ) ) {
			rejectInfo = synopDecoder.rejected("synop");
			return false;
		}

		h=INT_MAX;  //paramid 55
		VV=INT_MAX; //paramid 273
		N=INT_MAX;  //paramid 15

		for( list<kvalobs::kvData>::iterator it=dataList.begin(); it != dataList.end(); ++it ) {
			if( it->paramID() == 55 )
				h = static_cast<int>(it->original());
			else if( it->paramID() == 273 )
				VV = static_cast<int>( it->original() );
			else if( it->paramID() == 15 )
				N = static_cast<int>( it->original() );
		}

		return true;

	}

	bool decode2ParamVal( const std::string &synop, ParamVal *paramval, kvalobs::kvRejectdecode &rejectInfo ) {
		list<kvalobs::kvData> dataList;

		for( int i=0; paramval[i].paramid; ++i )
			*paramval[i].val = FLT_MAX;

		if( ! synopDecoder.decode( synop, dataList ) ) {
			rejectInfo = synopDecoder.rejected("synop");
			return false;
		}

		for( list<kvalobs::kvData>::iterator it=dataList.begin(); it != dataList.end(); ++it ) {
			for( int i=0; paramval[i].paramid; ++i ) {
				if( it->paramID() == paramval[i].paramid ) {
					*paramval[i].val = it->original();
					break;
				}
			}
		}

		return true;

	}


};



/*
 * Fra Lars Andresen: Har jeg f�tt f�lgende spesifikasjon for � skille mellom HL=-3 og
 * HL=-32767: Hvis skymengde, N, eller sikt, VV, mangler ("/"), enten den ene
 * eller andre eller begge (selv om gruppe 7 er med), s� anses HL="/" som
 * manglende.
 */
TEST_F( SynopDecodeTest, hVVN_missing )
{
	kvalobs::kvRejectdecode rejectInfo;
	int h;  //paramid 55
	int VV; //paramid 273
	int N;  //paramid 15
	string synopmsg;

	//h=/ VV=// N=/
	synopmsg="AAXX 18061 01488 16/// /1701 10139 20115 60062 333 20128 70192 91106 555 0/402 10150=";
	EXPECT_TRUE( decode_h_VV_N( synopmsg, h, VV, N, rejectInfo ) ) << "Rejected: " << rejectInfo;
	EXPECT_TRUE( h==INT_MAX && VV == INT_MAX && N == INT_MAX ) << "Expect: h=INT_MAX, VV=INT_MAX and N=INT_MAX. Got h=" << h << ", VV=" << VV << " and N=" << N;

	//h=/ VV=// N=8
	synopmsg="AAXX 18061 01488 16/// 81701 10139 20115 60062 333 20128 70192 91106 555 0/402 10150=";
	EXPECT_TRUE( decode_h_VV_N( synopmsg, h, VV, N, rejectInfo ) ) << "Rejected: " << rejectInfo;
	EXPECT_TRUE( h == INT_MAX && VV == INT_MAX && N == 8 ) << "Expect: h=INT_MAX, VV=INT_MAX and N=8. Got h=" << h << ", VV=" << VV << " and N=" << N;

	//h=/ VV=56 N=/
	synopmsg="AAXX 18061 01488 16/56 /1701 10139 20115 60062 333 20128 70192 91106 555 0/402 10150=";
	EXPECT_TRUE( decode_h_VV_N( synopmsg, h, VV, N, rejectInfo ) ) << "Rejected: " << rejectInfo;
	EXPECT_TRUE( h==INT_MAX && VV == 6000 && N == INT_MAX ) << "Expect: h=INT_MAX, VV=6000 and N=INT_MAX. Got h=" << h << ", VV=" << VV << " and N=" << N;

	//h=/ VV=56 N=8
	synopmsg="AAXX 18061 01488 16/56 81701 10139 20115 60062 333 20128 70192 91106 555 0/402 10150=";
	EXPECT_TRUE( decode_h_VV_N( synopmsg, h, VV, N, rejectInfo ) ) << "Rejected: " << rejectInfo;
	EXPECT_TRUE( h==-3 && VV == 6000 && N == 8 ) << "Expect: h=-3, VV=6000 and N=8. Got h=" << h << ", VV=" << VV << " and N=" << N;


	//h=3 VV=56 N=8
	synopmsg="AAXX 18061 01488 16356 81701 10139 20115 60062 333 20128 70192 91106 555 0/402 10150=";
	EXPECT_TRUE( decode_h_VV_N( synopmsg, h, VV, N, rejectInfo ) ) << "Rejected: " << rejectInfo;
	EXPECT_TRUE( h==200 && VV == 6000 && N == 8 ) << "Expect: h=200, VV=6000 and N=8. Got h=" << h << ", VV=" << VV << " and N=" << N;
}

/**
 * Test decoding of x. x should be interpreted as / in the synop, ie not set.
 */
TEST_F( SynopDecodeTest, decodeX )
{
	kvalobs::kvRejectdecode rejectInfo;
	float Nh, Cl;
	ParamVal param[]={ {14, &Nh},
			           {23, &Cl},
				       {0, 0} };

	string shipmsg="BBXX LDWR 07221 99662 10018 41997 02718 10036 21036 40067 53095 70111 8xx//"
			       " 22200 04064 11016 3//// 4//// 5//// 70082=";

	EXPECT_TRUE( decode2ParamVal( shipmsg, param, rejectInfo) ) << "Failed to decode ship message.";
	EXPECT_FLOAT_EQ( Nh, FLT_MAX) << "Expected: Nh=FLT_MAX. Got Nh=" << Nh;
	EXPECT_FLOAT_EQ( Cl, FLT_MAX) << "Expected: Cl=FLT_MAX. Got Cl=" << Cl;
}

/**
 * Test scaling of DW1 and DW2.
 */
TEST_F( SynopDecodeTest, scaleDW1_DW2)
{
	kvalobs::kvRejectdecode rejectInfo;
	float dw1, dw2;
	ParamVal param[]={ {65, &dw1},
			           {66, &dw2},
				       {0, 0} };

	string synopmsg="AAXX 18061 01488 16/// /1701 10139 20115 60062 222XX 30112=";

	EXPECT_TRUE( decode2ParamVal( synopmsg, param, rejectInfo) ) << "Failed to decode synop message.";
	EXPECT_FLOAT_EQ( dw1, 10.0) << "Expected: DW1=10.0. Got DW1=" << dw1;
	EXPECT_FLOAT_EQ( dw2, 120.0) << "Expected: DW2=120.0. Got DW2=" << dw2;
}


/**
 * Test decoding of 333 7RRRR.
 */
TEST_F( SynopDecodeTest, decode333_7RRR)
{
	kvalobs::kvRejectdecode rejectInfo;
	string synopmsg;
	float RR24;
	ParamVal param[]={ {110, &RR24},
			           {0, 0} };

	synopmsg="AAXX 18061 01488 16/// /1701 10139 20115 60062 333 79999=";
	EXPECT_TRUE( decode2ParamVal( synopmsg, param, rejectInfo) ) << "Failed to decode synop message.";
	EXPECT_FLOAT_EQ( RR24, 0.0 ) << "Expected: RR24=0.0. Got RR24=" << RR24;

	synopmsg="AAXX 18061 01488 16/// /1701 10139 20115 60062 333 70000=";
	EXPECT_TRUE( decode2ParamVal( synopmsg, param, rejectInfo) ) << "Failed to decode synop message.";
	EXPECT_FLOAT_EQ( RR24, -1.0 ) << "Expected: RR24=-1.0. Got RR24=" << RR24;

	synopmsg="AAXX 18061 01488 16/// /1701 10139 20115 60062 333 70012=";
	EXPECT_TRUE( decode2ParamVal( synopmsg, param, rejectInfo) ) << "Failed to decode synop message.";
	EXPECT_FLOAT_EQ( RR24, 1.2 ) << "Expected: RR24=1.2. Got RR24=" << RR24;
}

/**
 * Decoding of 333 3Ejjj.
 * If E is given, set SA=-1.
 */
TEST_F( SynopDecodeTest, decode333_3Ejjj)
{
	kvalobs::kvRejectdecode rejectInfo;
	string synopmsg;
	float SA; //paramid 112 (sss from 4E'sss)
	float E_; //paramid 7 (E' from 4E'sss)
	float SD; //paramid 18, deduced from E_
	float E; //paramid 129, (E from 3Ejjj)
	float jjj; //Not used, (jjj from 3Ejjj)
	ParamVal param[]={{7,   &E_},
			          {18,  &SD},
			          {112, &SA},
			          {129, &E},
		              {0, 0} };


	synopmsg="AAXX 18061 40745 46/// ///// 333 31002=";

	EXPECT_TRUE( decode2ParamVal( synopmsg, param, rejectInfo) ) << "Rejected: " << rejectInfo.comment();
	EXPECT_TRUE( E_==FLT_MAX && SD==FLT_MAX ) << "Failed (4E'sss): E'=" << E_ << " SD=" << SD;
	EXPECT_FLOAT_EQ( SA, -1 ) << "Failed: (3Ejjj) SA=" << SA;
	EXPECT_FLOAT_EQ( E, 1 ) << "Failed: (3Ejjj) E=" << E;
}

TEST_F( SynopDecodeTest, first_last_day_month )
{
   miTime ref("2012-02-01 23:59:59");
   miTime firstDay = kvSynopDecoder::firstDayNextMonth( ref );
   miTime lastDay = kvSynopDecoder::lastDayThisMonth( ref );

   ASSERT_EQ( firstDay, miTime( "2012-03-01 00:00:00") );
   ASSERT_EQ( lastDay, miTime( "2012-02-29 00:00:00") );

   ref = miTime("2012-02-01 00:00:00");

   ASSERT_EQ( firstDay, miTime( "2012-03-01 00:00:00") );
   ASSERT_EQ( lastDay, miTime( "2012-02-29 00:00:00") );
}

TEST_F( SynopDecodeTest, createObsTime )
{

   miTime ref("2011-09-15 23:00:00");
   miTime refT=ref;
   refT.addDay( 10 ); //2011-09-25 23:00:00

   miTime obsTime= kvSynopDecoder::createObsTime( 1, 0, ref );
   ASSERT_EQ( obsTime, miTime( "2011-09-01 00:00:00") );

   obsTime= kvSynopDecoder::createObsTime( 15, 0, ref );
   ASSERT_EQ( obsTime, miTime( "2011-09-15 00:00:00") );

   obsTime= kvSynopDecoder::createObsTime( 15, 21, ref );
   ASSERT_EQ( obsTime, miTime( "2011-09-15 21:00:00") );

   obsTime= kvSynopDecoder::createObsTime( 16, 00, ref );
   ASSERT_EQ( obsTime, miTime( "2011-09-16 00:00:00") );

   obsTime= kvSynopDecoder::createObsTime( 25, 23, ref );
   ASSERT_EQ( obsTime, miTime( "2011-09-25 23:00:00") );

   obsTime= kvSynopDecoder::createObsTime( 26, 0, ref );
   ASSERT_EQ( obsTime, miTime( "2011-08-26 00:00:00") );

   //Test for correct handling of lastday in month.
   ref = miTime("2011-09-30 23:51:57");
   obsTime= kvSynopDecoder::createObsTime( 1, 0, ref );
   ASSERT_EQ( miTime( "2011-10-01 00:00:00"), obsTime );
}


int
main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
