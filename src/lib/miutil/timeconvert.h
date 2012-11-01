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
#include <puTools/miTime.h>
#include <sstream>

namespace miutil
{
inline boost::posix_time::ptime to_ptime(const miTime & mt)
{
	return boost::posix_time::ptime(
			boost::gregorian::date(mt.year(), mt.month(), mt.day()),
			boost::posix_time::time_duration(mt.hour(), mt.min(), mt.sec()));
}
}

namespace boost
{
namespace posix_time
{
inline miutil::miTime to_miTime(const ptime & t)
{
	const boost::gregorian::date & d = t.date();
	const boost::posix_time::time_duration c = t.time_of_day();
	return miutil::miTime(d.year(), d.month(), d.day(), c.hours(), c.minutes(), c.seconds());
}

inline std::string to_kvalobs_string(const ptime & t)
{
	static std::ostringstream * ss = 0;
	if ( ! ss )
	{
		ss = new std::ostringstream;
		time_facet * facet = new time_facet;
		const char * format = "%Y-%m-%d %H:%M:%S";
		facet->format(format);
		ss->imbue(std::locale(std::cout.getloc(), facet));
	}
	else
		ss->str(std::string());

	(*ss) << t;

	return ss->str();
}

inline std::string to_kvalobs_string(const time_duration & t)
{
	return to_simple_string(t);
}

inline boost::posix_time::ptime time_from_string_nothrow(const std::string & s)
{
	try
	{
		return boost::posix_time::time_from_string(s);
	}
	catch ( std::exception & e )
	{
		return boost::posix_time::ptime();
	}
}
}

namespace gregorian
{
inline std::string to_kvalobs_string(const date & d)
{
	static std::ostringstream * ss = 0;
	if ( ! ss )
	{
		ss = new std::ostringstream;
		date_facet * facet = new date_facet;
		const char * format = "%Y-%m-%d";
		facet->format(format);
		ss->imbue(std::locale(std::cout.getloc(), facet));
	}
	else
		ss->str(std::string());

	(*ss) << d;

	return ss->str();
}
}
}



#endif /* TIMECONVERT_H_ */
