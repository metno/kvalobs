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
#include <miutil/miTimeParse.h>

using namespace miutil;
using namespace std;
/*
class miTimeParseTest : public testing::Test
{
public:
	miTimeParseTest()
	{}
};

TEST_F( miTimeParseTest, parse)
{

   miTime nt("2006-01-02 19:30:00");
   string buf("200601020930bla");
   miTime time;

   try{
       string::size_type i=miTimeParse("%Y%m%d%H%M", buf, time, nt);
       ASSERT_EQ( time,  miTime( 2006, 1, 2, 9, 30 ) );
       ASSERT_EQ( i, 12 );
    }
    catch(miTimeParseException ex){
       cerr << ex.what() << endl;
       FAIL();
    }


    try{
       buf="010220";
       string::size_type i=miTimeParse("%m%d%H", buf, time, nt);
       ASSERT_EQ( time,  miTime( "2005-01-02 20:00:00") );
       ASSERT_EQ( i, 6 );
    }
    catch(miTimeParseException ex){
       cerr << ex.what() << endl;
       FAIL();
    }

    try{
       miTime nt("2011-10-25 18:51:57");
       nt.addHour( 3 );
       //nt.addDay( 10 );
       buf="2522";
       string::size_type i=miTimeParse("%d%H", buf, time, nt);
       cout << "myTime: " << time << " refTime: " << nt << endl;
//       ASSERT_EQ( time,  miTime( "2005-01-02 20:00:00") );
//       ASSERT_EQ( i, 6 );
    }
    catch(miTimeParseException ex){
       cerr << ex.what() << endl;
       FAIL();
    }

//	ASSERT_EQ(1u, out.size());
//
//	EXPECT_FLOAT_EQ(in.corrected(), out.front().corrected());
}
*/

