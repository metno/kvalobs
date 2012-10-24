/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvTextDataOperations.cc,v 1.1.2.3 2007/09/27 09:02:31 paule Exp $

 Copyright (C) 2007 met.no

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
#include <kvalobs/kvTextDataOperations.h>

namespace kvalobs
{

kvTextDataFactory::kvTextDataFactory(int stationID,
		const boost::posix_time::ptime & obsTime, int typeID) :
		stationID_(stationID), typeID_(typeID), obstime_(obsTime)
{
}

kvTextDataFactory::kvTextDataFactory(const kvTextData & d) :
		stationID_(d.stationID()), typeID_(d.typeID()), obstime_(d.obstime())
{
}

kvTextData kvTextDataFactory::getData(std::string val, int paramID,
		const boost::posix_time::ptime & obsTime) const
{
	kvTextData ret(stationID_, obsTime.is_not_a_date_time() ? obstime_ : obsTime, val,
			paramID, boost::posix_time::microsec_clock::universal_time(), typeID_);
	return ret;
}

namespace compare
{
bool lt_kvTextData::operator()(const kvTextData & a, const kvTextData & b) const
{
	if (a.stationID() != b.stationID())
		return a.stationID() < b.stationID();
	if (a.typeID() != b.typeID())
		return a.typeID() < b.typeID();
	if (a.obstime() != b.obstime())
		return a.obstime() < b.obstime();
	return a.paramID() < b.paramID();
}

bool kvTextData_same_obs_and_parameter::operator()(const kvTextData & a,
		const kvTextData & b) const
{
	return a.stationID() == b.stationID() and a.paramID() == b.paramID()
			and a.obstime() == b.obstime() and a.typeID() == b.typeID();
}

bool kvTextData_exactly_equal_ex_tbtime::operator()(const kvTextData & a,
		const kvTextData & b) const
{
	return kvTextData_same_obs_and_parameter()(a, b)
			and a.original() == b.original();
}
}
}

