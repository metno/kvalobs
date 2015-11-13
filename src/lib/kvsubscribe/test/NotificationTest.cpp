/*
 * NotificationTest.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: vegardb
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
