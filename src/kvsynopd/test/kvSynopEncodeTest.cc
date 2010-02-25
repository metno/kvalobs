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
#include <miutil/cmprspace.h>
#include <kvalobs/kvData.h>
#include <list>
#include <kvalobs/kvStation.h>
#include <milog/milog.h>
#include "synop.h"
#include "kvSynopEncodeTestConf.h"
#include "StationInfoParse.h"
#include <sstream>
#include "ReadDataFile.h"
#include <gtest/gtest.h>

using namespace std;

namespace {
}


class SynopEncodeTest : public testing::Test
{

protected:
	Synop synopEncoder;
	std::list<StationInfoPtr> stationList;

	StationInfoPtr findWmoNo( int wmono ) {
		for( std::list<StationInfoPtr>::iterator it=stationList.begin(); it!=stationList.end(); ++it ) {
			if( (*it)->wmono() == wmono )
				return *it;
		}

		return StationInfoPtr();
	}

	///Called before each test case.
	virtual void SetUp() {
		using namespace miutil::conf;
		ConfParser confParser;
		istringstream iconf(testconf);

		//Turn off almost all logging.
		milog::Logger::logger().logLevel( milog::ERROR );

		synopEncoder.setTest( true );
		//cerr << "[" << endl << testconf << endl << "]" << endl;

		ConfSection *conf = confParser.parse( iconf );

		ASSERT_TRUE( conf ) << "Cant parse the configuration settings.";

		StationInfoParse stationParser;

		ASSERT_TRUE( stationParser.parse( conf, stationList ) ) << "Cant parse the station information.";
	}

	///Called after each test case.
	virtual void TearDown() {

	}


};



TEST_F( SynopEncodeTest, RR24_for_RRRtr )
{
	SynopDataList data;
	StationInfoPtr stInfo;
	string synop;
	stInfo = findWmoNo( 1389 );


	ASSERT_TRUE( stInfo ) << "No station information for wmono " << 1389;

	loadSynopDataFromFile( "data_7010-1.dat", stInfo, data );
	EXPECT_TRUE( synopEncoder.doSynop( stInfo, data, synop, false ) ) << "FAILED: Cant generate synop for "<< 1389;
	miutil::cmprspace( synop, true );
    EXPECT_EQ( synop, "AAXX 23061 01389 16/// ///// 1//// 2//// 69912 333 70003 555 41///=") << "Generated synop 1: " << synop;

    loadSynopDataFromFile( "data_7010-2.dat", stInfo, data );
    EXPECT_TRUE( synopEncoder.doSynop( stInfo, data, synop, false ) ) << "FAILED: Cant generate synop for "<< 1389;
    miutil::cmprspace( synop, true );
    EXPECT_EQ( synop, "AAXX 23061 01389 16/// ///// 1//// 2//// 60002 333 70000 555 40///=")<< "Generated synop 2: " << synop;

    loadSynopDataFromFile( "data_7010-3.dat", stInfo, data );
    EXPECT_TRUE( synopEncoder.doSynop( stInfo, data, synop, false ) ) << "FAILED: Cant generate synop for "<< 1389;
    miutil::cmprspace( synop, true );
    EXPECT_EQ( synop, "AAXX 23061 01389 16/// ///// 1//// 2//// 60002 333 79999 555 40///=")<< "Generated synop 3: " << synop;

    loadSynopDataFromFile( "data_7010-4.dat", stInfo, data );
    EXPECT_TRUE( synopEncoder.doSynop( stInfo, data, synop, false ) ) << "FAILED: Cant generate synop for "<< 1389;
    miutil::cmprspace( synop, true );
    EXPECT_EQ( synop, "AAXX 23061 01389 16/// ///// 1//// 2//// 69902 333 79999 555 40///=")<< "Generated synop 4: " << synop;

    loadSynopDataFromFile( "data_7010-5.dat", stInfo, data );
    EXPECT_TRUE( synopEncoder.doSynop( stInfo, data, synop, false ) ) << "FAILED: Cant generate synop for "<< 1389;
    miutil::cmprspace( synop, true );
    EXPECT_EQ( synop, "AAXX 23061 01389 16/// ///// 1//// 2//// 69912 333 70001 555 41///=")<< "Generated synop 5: " << synop;

}


int
main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
