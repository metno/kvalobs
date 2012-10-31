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
}
}



#endif /* TIMECONVERT_H_ */
