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
#include <miutil/timeconvert.h>
#include <miutil/miTimeParse.h>

using namespace miutil;
using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;

class miTimeParseTest : public testing::Test
{
public:
	miTimeParseTest()
	{}
};
TEST_F( miTimeParseTest, timeconvert)
{
    ptime t(time_from_string_nothrow("20060206100000"));

    ASSERT_EQ( time_from_string_nothrow("20060206100000"),
               ptime( date(2006,2,6), time_duration( 10, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("200602061000"),
                   ptime( date(2006,2,6), time_duration( 10, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("2006020610"),
               ptime( date(2006,2,6), time_duration( 10, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("20060206"),
               ptime( date(2006,2,6), time_duration( 0, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("2006-02-06 10:00:00"),
               ptime( date(2006,2,6), time_duration( 10, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("2006-02-06 10:00:00Z"),
               ptime( date(2006,2,6), time_duration( 10, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("2006-02-06 10:00:00 Z"),
               ptime( date(2006,2,6), time_duration( 10, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("2006-02-06 10:00:00+0100"),
               ptime( date(2006,2,6), time_duration( 9, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("2006-02-06 10:00:00+01"),
               ptime( date(2006,2,6), time_duration( 9, 0, 0)));


    ASSERT_EQ( time_from_string_nothrow("2006-02-06 10:00:00-0100"),
               ptime( date(2006,2,6), time_duration( 11, 0, 0)));

    ASSERT_EQ( time_from_string_nothrow("2006-02-06 10:00:00-01"),
               ptime( date(2006,2,6), time_duration( 11, 0, 0)));

}



TEST_F( miTimeParseTest, parse)
{

   ptime nt(time_from_string("2006-01-02 19:30:00") );
   string buf("200601020930bla");
   ptime time;

   try{
       string::size_type i=miTimeParse("%Y%m%d%H%M", buf, time, nt);
       ASSERT_EQ( time,  ptime( date(2006, 1, 2),
                                time_duration( 9, 30, 0) ));
       ASSERT_EQ( i, 12 );
    }
    catch(const miTimeParseException &ex){
       cerr << ex.what() << endl;
       FAIL();
    }


    try{
       buf="010220";
       string::size_type i=miTimeParse("%m%d%H", buf, time, nt);
       ASSERT_EQ( time,  ptime( time_from_string("2005-01-02 20:00:00")) );
       ASSERT_EQ( i, 6 );
    }
    catch(const miTimeParseException &ex){
       cerr << ex.what() << endl;
       FAIL();
    }

    try{
       ptime nt(time_from_string("2011-10-25 18:51:57"));
       nt += hours( 3 );
       //nt.addDay( 10 );
       buf="2522";
       string::size_type i=miTimeParse("%d%H", buf, time, nt);
    }
    catch(const miTimeParseException &ex){
       cerr << ex.what() << endl;
       FAIL();
    }

//	ASSERT_EQ(1u, out.size());
//
//	EXPECT_FLOAT_EQ(in.corrected(), out.front().corrected());
}


