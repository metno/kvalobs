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

#include <boost/date_time/posix_time/ptime.hpp>
#include <gtest/gtest.h>
#include <miutil/timeconvert.h>
#include <decodeutility/decodeutility.h>

using namespace std;
using namespace decodeutility;

namespace pt = boost::posix_time;

class DecodeUtilityTest : public testing::Test
{
public:
    DecodeUtilityTest()
	{}

};

TEST_F(DecodeUtilityTest, hsCode2m)
{
    ASSERT_TRUE( hsCode2m( -1 ) == INT_MAX );
    ASSERT_TRUE( hsCode2m( 100 ) == INT_MAX );
	ASSERT_TRUE( hsCode2m( 0 ) == 0 );
	ASSERT_TRUE( hsCode2m( 1 ) == 30 );
	ASSERT_TRUE( hsCode2m( 50 ) == 1500 );
	ASSERT_TRUE( hsCode2m( 51 ) == INT_MAX );
	ASSERT_TRUE( hsCode2m( 52 ) == INT_MAX );
	ASSERT_TRUE( hsCode2m( 53 ) == INT_MAX );
	ASSERT_TRUE( hsCode2m( 54 ) == INT_MAX );
	ASSERT_TRUE( hsCode2m( 55 ) == INT_MAX );
	ASSERT_TRUE( hsCode2m( 56 ) == 1800 );
	ASSERT_TRUE( hsCode2m( 80 ) == 9000 );
	ASSERT_TRUE( hsCode2m( 81 ) == 10500 );
	ASSERT_TRUE( hsCode2m( 85 ) == 16500 );
	ASSERT_TRUE( hsCode2m( 89 ) == 21000 );
	ASSERT_TRUE( hsCode2m( 90 ) == 0 );
	ASSERT_TRUE( hsCode2m( 91 ) == 50 );
	ASSERT_TRUE( hsCode2m( 92 ) == 100 );
	ASSERT_TRUE( hsCode2m( 93 ) == 200 );
	ASSERT_TRUE( hsCode2m( 94 ) == 300 );
	ASSERT_TRUE( hsCode2m( 95 ) == 600 );
	ASSERT_TRUE( hsCode2m( 96 ) == 1000 );
	ASSERT_TRUE( hsCode2m( 97 ) == 1500 );
	ASSERT_TRUE( hsCode2m( 98 ) == 2000 );
	ASSERT_TRUE( hsCode2m( 99 ) == 2500 );
}

TEST_F(DecodeUtilityTest, hs2Code )
{
//    ASSERT_EQ( hs2Code( 0 ), string("00") );
//    ASSERT_EQ( hsCode2m( 30 ), string("01") );
//    ASSERT_EQ( hsCode2m( 1500 ), string("50") );
//    ASSERT_EQ( hsCode2m( 1800 ), string("56") );
//    ASSERT_EQ( hsCode2m( 9000 ), string("80") );
//    ASSERT_EQ( hsCode2m( 10500 ), string("81") );
//    ASSERT_EQ( hsCode2m( 16500 ), string("85") );
//    ASSERT_EQ( hsCode2m( 21000 ), string("89") );

    ASSERT_TRUE( hs2Code( 0 ) == "00" );
    ASSERT_TRUE( hs2Code( 30 ) == "01" );
    ASSERT_TRUE( hs2Code( 130 ) == "04" );
    ASSERT_TRUE( hs2Code( 1500 ) == "50" );
    ASSERT_TRUE( hs2Code( 1799 ) == "50" );
    ASSERT_TRUE( hs2Code( 1800 ) =="56" );
    ASSERT_TRUE( hs2Code( 3299 ) == "60" );
    ASSERT_TRUE( hs2Code( 9000 ) == "80" );
    ASSERT_TRUE( hs2Code( 10500 ) == "81" );
    ASSERT_TRUE( hs2Code( 11999 ) == "81" );
    ASSERT_TRUE( hs2Code( 16500 ) == "85" );
    ASSERT_TRUE( hs2Code( 21000 ) == "88" );
    ASSERT_TRUE( hs2Code( 21001 ) == "89" );
    //ASSERT_TRUE( hs2Code( 21000 ) == "89" );

//    ASSERT_TRUE( hsCode2m( 90 ) == 0 );
//    ASSERT_TRUE( hsCode2m( 91 ) == 50 );
//    ASSERT_TRUE( hsCode2m( 92 ) == 100 );
//    ASSERT_TRUE( hsCode2m( 93 ) == 200 );
//    ASSERT_TRUE( hsCode2m( 94 ) == 300 );
//    ASSERT_TRUE( hsCode2m( 95 ) == 600 );
//    ASSERT_TRUE( hsCode2m( 96 ) == 1000 );
//    ASSERT_TRUE( hsCode2m( 97 ) == 1500 );
//    ASSERT_TRUE( hsCode2m( 98 ) == 2000 );
//    ASSERT_TRUE( hsCode2m( 99 ) == 2500 );
//
}


