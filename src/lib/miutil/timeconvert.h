/*
 * kvalobs
 *
 * (C) Copyright 2012, met.no
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */


#ifndef TIMECONVERT_H_
#define TIMECONVERT_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <iomanip>

#ifdef __dnmi_miTime__
namespace miutil
{
inline boost::posix_time::ptime to_ptime(const miTime & mt)
{
	if ( mt.undef() )
		return boost::posix_time::ptime();
	return boost::posix_time::ptime(
			boost::gregorian::date(mt.year(), mt.month(), mt.day()),
			boost::posix_time::time_duration(mt.hour(), mt.min(), mt.sec()));
}
}
#endif

namespace boost
{
namespace gregorian
{
inline std::string to_kvalobs_string(const date & d)
{
	std::ostringstream s;
	s << d.year() << '-'
			<< std::setfill('0') << std::setw(2) << std::right << d.month().as_number() << '-'
			<< std::setfill('0') << std::setw(2) << std::right << d.day();
	return s.str();
}
}
namespace posix_time
{
#ifdef __dnmi_miTime__
inline miutil::miTime to_miTime(const ptime & t)
{
	if ( t.is_not_a_date_time() )
		return miutil::miTime();

	const boost::gregorian::date & d = t.date();
	const boost::posix_time::time_duration c = t.time_of_day();
	return miutil::miTime(d.year(), d.month(), d.day(), c.hours(), c.minutes(), c.seconds());
}
#endif

inline std::string to_kvalobs_string(const time_duration & t)
{
	return to_simple_string(t);
}

inline std::string to_kvalobs_string(const ptime & t, char separator = ' ')
{
	std::ostringstream s;
	s << to_kvalobs_string(t.date()) << separator << to_kvalobs_string(t.time_of_day());
	return s.str();
}

inline boost::posix_time::ptime time_from_string_nothrow(const std::string & s)
{
	try
	{
		std::string::size_type pos = s.find('+');
		if  ( pos != std::string::npos )
		{
			// remove time zone specifiers from string
			std::string time = s.substr(0, pos);
			boost::posix_time::time_duration offset = boost::posix_time::hours(boost::lexical_cast<int>(s.substr(pos)));
			return boost::posix_time::time_from_string(time) - offset;
		}
		return boost::posix_time::time_from_string(s);
	}
	catch ( std::exception & e )
	{
#ifdef LOGWARN
		LOGWARN("Unable to interpret string as time: " << s);
#endif
		return boost::posix_time::ptime();
	}
}
}
}



#endif /* TIMECONVERT_H_ */
