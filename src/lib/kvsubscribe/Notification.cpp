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


#include "Notification.h"
#include <sstream>
#include <boost/date_time.hpp>
#include <jsoncpp/json/json.h>


namespace kvalobs
{
namespace subscribe
{

Notification::Notification(int station, int type, const boost::posix_time::ptime & obstime) :
            station_(station),
            type_(type),
            obstime_(obstime)

{
}

namespace
{
int getInt(const Json::Value & from, const std::string & value)
{
    const Json::Value & ret = from[value];
    if ( ret.isNull() )
        throw std::runtime_error("Missing <" + value + "> entry in data");
    return ret.asInt();
}

boost::posix_time::ptime getTime(const Json::Value & from, const std::string & value)
{
    const Json::Value & ret = from[value];
    if ( ret.isNull() )
        throw std::runtime_error("Missing <" + value + "> entry in data");
    return boost::posix_time::from_iso_string(ret.asString());
}
}

Notification::Notification(const std::string & message)
{
    Json::Reader reader;
    Json::Value root;
    if ( ! reader.parse(message, root) )
        throw std::runtime_error("Unable to parse notification: " + reader.getFormattedErrorMessages());

    station_ = getInt(root, "station");
    type_ = getInt(root, "type");
    obstime_ = getTime(root, "obstime");
}

Notification::~Notification()
{
}

std::string Notification::str() const
{
    Json::FastWriter writer;
    Json::Value root;

    root["station"] = Json::Value(station());
    root["type"] = Json::Value(type());
    root["obstime"] = Json::Value(boost::posix_time::to_iso_string(obstime()));

    return writer.write(root);
}


} /* namespace subscribe */
} /* namespace kvalobs */
