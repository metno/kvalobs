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
#include <db/returntypes/kvCronString.h>

using kvalobs::CronString;

using namespace boost::posix_time;
using namespace boost::gregorian;

TEST(kvCronStringTest, fiveStarsMatchAnything)
{
	CronString cs("* * * * *");

	for ( ptime t = time_from_string("2010-04-22 00:00:00"); t < time_from_string("2010-04-23 00:00:00"); t += hours(1) )
		EXPECT_TRUE(cs.active(t)) << t;

	for ( ptime t = time_from_string("2010-04-01 00:00:00"); t < time_from_string("2010-05-01 00:00:00"); t += days(1) )
		EXPECT_TRUE(cs.active(t)) << t;
}

TEST(kvCronStringTest, specificHour)
{
	CronString cs("* 6 * * *");

	for ( ptime t = time_from_string("2010-04-22 07:00:00"); t < time_from_string("2010-04-23 06:00:00"); t += hours(1) )
		EXPECT_FALSE(cs.active(t)) << t;

	EXPECT_TRUE(cs.active(time_from_string("2010-04-22 06:00:00")));
}

TEST(kvCronStringTest, multipleHours)
{
	CronString cs("* 6,18 * * *");

	for ( ptime t = time_from_string("2010-04-22 07:00:00"); t <  time_from_string("2010-04-22 18:00:00"); t += hours(1) )
		EXPECT_FALSE(cs.active(t)) << t;
	for ( ptime t = time_from_string("2010-04-22 19:00:00"); t < time_from_string("2010-04-23 06:00:00"); t += hours(1) )
		EXPECT_FALSE(cs.active(t)) << t;

	EXPECT_TRUE(cs.active(time_from_string("2010-04-22 06:00:00")));
	EXPECT_TRUE(cs.active(time_from_string("2010-04-22 18:00:00")));
}
