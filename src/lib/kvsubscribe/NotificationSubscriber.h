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


#ifndef NOTIFICATIONSUBSCRIBER_H_
#define NOTIFICATIONSUBSCRIBER_H_

#include "KafkaConsumer.h"
#include <functional>

namespace kvalobs
{
namespace subscribe
{
class Notification;


/**
 * Subscriber for new data notifications. Kvalobs' qabase application will
 * send out a notification whenever it has finished processing new data, even
 * if it has decided that no checks should be run. This class may be used to
 * pick up such notifications.
 *
 * Valid messages are handled through the provided handling function, while
 * errors are merely logged.
 */
class NotificationSubscriber: public KafkaConsumer
{
public:

    /**
     * Notification handling function
     */
    typedef std::function<void(const Notification &)> Handler;

    NotificationSubscriber(Handler handler, ConsumptionStart startAt = Stored, const std::string & brokers = "localhost");

    /**
     * The identifying string for this message stream
     */
    static std::string topic();

protected:

    virtual void data(const char * msg, unsigned length);
    virtual void error(int code, const std::string & msg);

private:
    Handler handler_;
};

} /* namespace subscribe */
} /* namespace kvalobs */

#endif /* NOTIFICATIONSUBSCRIBER_H_ */
