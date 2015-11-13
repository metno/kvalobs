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


#ifndef SRC_SERVICE_LIBS_KVSUBSCRIBE_NOTIFICATION_H_
#define SRC_SERVICE_LIBS_KVSUBSCRIBE_NOTIFICATION_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <string>


namespace kvalobs
{
namespace subscribe
{

/**
 * Ad-hoc data holder for notifications. To be replaced by something more flexible
 */
class Notification
{
public:
    Notification(int station, int type, const boost::posix_time::ptime & obstime);
    Notification(const std::string & message);
    Notification(const char * message) : Notification(std::string(message)) {}
    ~Notification();

    int station() const { return station_; }
    int type() const { return type_; }
    const boost::posix_time::ptime & obstime() const { return obstime_; }

    std::string str() const;

private:
    int station_;
    int type_;
    boost::posix_time::ptime obstime_;
};


} /* namespace subscribe */
} /* namespace kvalobs */

#endif /* SRC_SERVICE_LIBS_KVSUBSCRIBE_NOTIFICATION_H_ */
