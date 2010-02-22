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


#include <limits.h>
#include <string>
#include <kvalobs/kvData.h>
#include <list>
#include <kvalobs/kvStation.h>
#include "kvSynopDecoder.h"

#include <gtest/gtest.h>

using namespace std;

TEST( SynopDecode, hVVN_missing )
{

}

/**
 * Test decoding of x. x should be interpreted as / in the synop, ie not set.
 */
TEST( SynopDecode, decodeX )
{
	list<kvalobs::kvStation> stationList;
	list<kvalobs::kvData> dataList;
	kvalobs::kvRejectdecode rejectInfo;
	kvSynopDecoder synopDecoder;

	/*
	 *     kvStation( int st, float la, float lo, float he, float max,
	       const miutil::miString& na, int wm, int nn,
	       const miutil::miString& ic, const miutil::miString& ca,
	       const miutil::miString& ss, int environmentid,
	       bool static_, const miutil::miTime& fromtime)
	 *
	 */
	stationList.push_back( kvalobs::kvStation(76900, 66, 2, 6, 0,"MIKE", 0, 76900, "", "LDWR","",7, true,
			                                  miutil::miTime("1977-01-01 00:00:00")));

	ASSERT_TRUE( synopDecoder.initialise( stationList, 10, 20 ) ) << "Cant initialize the synopdecoder.";
	string shipmsg="BBXX LDWR 07221 99662 10018 41997 02718 10036 21036 40067 53095 70111 8xx//"
			       " 22200 04064 11016 3//// 4//// 5//// 70082=";

	bool decoded = synopDecoder.decode( shipmsg , dataList );

	EXPECT_TRUE( decoded ) << "Failed to decode ship message.";

	if( ! decoded )  {
		rejectInfo = synopDecoder.rejected("synop");
		cerr << "Rejected: " << rejectInfo << endl;
	}

	int Nh=INT_MAX; //paramid 14
	int Cl=INT_MAX; //paramid 23

	for( list<kvalobs::kvData>::iterator it=dataList.begin(); it != dataList.end(); ++it ) {
		if( it->paramID() == 14 )
			Nh = static_cast<int>(it->original());
		else if( it->paramID() == 23 )
			Cl = static_cast<int>( it->original() );
	}

	EXPECT_TRUE( Nh == INT_MAX ) << "Nh is invalid set.";
	EXPECT_TRUE( Cl == INT_MAX ) << "Cl is invalid set.";
}


int
main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
