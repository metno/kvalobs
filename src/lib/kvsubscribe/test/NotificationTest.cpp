/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  Copyright (C) 2015 met.no

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
#include "../Notification.h"
#include <boost/date_time/posix_time/posix_time.hpp>


using namespace kvalobs::subscribe;
using namespace boost::posix_time;
using namespace boost::gregorian;



TEST(NotificationTest, simpleConstruct)
{
    Notification n(180, 302, boost::posix_time::time_from_string("2015-10-29 12:00:00"));

    EXPECT_EQ(180, n.station());
    EXPECT_EQ(302, n.type());
    EXPECT_EQ(ptime(date(2015,Oct,29), time_duration(12,0,0)), n.obstime());
}


TEST(NotificationTest, createFromString)
{
    Notification n("{\"station\":180,\"type\":302,\"obstime\":\"20151029T120000\"}");

    EXPECT_EQ(180, n.station());
    EXPECT_EQ(302, n.type());
    EXPECT_EQ(ptime(date(2015,Oct,29), time_duration(12,0,0)), n.obstime());
}

TEST(NotificationTest, createFromStringWithDelimiterT)
{
    Notification n("{\"station\":184,\"type\":2,\"obstime\":\"20160104T063000\"}");

    EXPECT_EQ(184, n.station());
    EXPECT_EQ(2, n.type());
    EXPECT_EQ(ptime(date(2016,Jan,4), time_duration(6,30,0)), n.obstime());
}

TEST(NotificationTest, implicitCreateFromString)
{
    Notification n = "{\"station\":100,\"type\":501,\"obstime\":\"20141220T120000\"}";

    EXPECT_EQ(100, n.station());
    EXPECT_EQ(501, n.type());
    EXPECT_EQ(ptime(date(2014,12,20), time_duration(12,0,0)), n.obstime());
}

TEST(NotificationTest, roundTrip)
{
    Notification a(10, 20, ptime(date(2016,4,21), time_duration(6,0,0)));
    Notification b(a.str());

    EXPECT_EQ(a.station(), b.station());
    EXPECT_EQ(a.type(), b.type());
    EXPECT_EQ(a.obstime(), b.obstime());
}
