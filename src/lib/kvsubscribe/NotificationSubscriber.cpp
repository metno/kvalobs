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


#include "NotificationSubscriber.h"
#include "Notification.h"
#include "queue.h"
#include <milog/milog.h>
#include <iostream>

namespace kvalobs
{
namespace subscribe
{

NotificationSubscriber::NotificationSubscriber(Handler handler, ConsumptionStart startAt, const std::string & brokers) :
        KafkaConsumer(startAt, topic(), brokers),
        handler_(handler)

{
}

std::string NotificationSubscriber::topic()
{
    return queue::notification();
}

void NotificationSubscriber::data(const char * msg, unsigned length)
{
    std::string message(msg, length);
    Notification n(message);
    handler_(n);
}

void NotificationSubscriber::error(int code, const std::string & msg)
{
    milog::LogContext context("NotificationSubscriber");
    LOGERROR(msg);

    std::clog << "ERORR:\t" << msg << std::endl;
}



} /* namespace subscribe */
} /* namespace kvalobs */
