/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvReferenceStation.cc,v 1.2.6.2 2007/09/27 09:02:30 paule Exp $

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
#include <kvalobs/kvReferenceStation.h>
#include <boost/lexical_cast.hpp>

using namespace std;

std::string kvalobs::kvReferenceStation::toSend() const
{
	ostringstream ost;
	ost << "(" << stationid_ << "," << paramsetid_ << "," << quoted(reference_)
			<< ")";
	return ost.str();
}

bool kvalobs::kvReferenceStation::set(int stationid, int paramsetid,
		const std::string& reference)
{
	stationid_ = stationid;
	paramsetid_ = paramsetid;
	reference_ = reference;
	sortBy_ = boost::lexical_cast<std::string>(stationid_);
	return true;
}

bool kvalobs::kvReferenceStation::set(const dnmi::db::DRow& r_)
{
	dnmi::db::DRow &r = const_cast<dnmi::db::DRow&>(r_);
	string buf;
	list<string> names = r.getFieldNames();
	list<string>::iterator it = names.begin();

	for (; it != names.end(); it++)
	{
		try
		{
			buf = r[*it];
			if (*it == "stationid")
			{
				stationid_ = atoi(buf.c_str());
			}
			else if (*it == "paramsetid")
			{
				paramsetid_ = atoi(buf.c_str());
			}
			else if (*it == "reference")
			{
				reference_ = buf;
			}
		} catch (...)
		{
			CERR("kvReferenceStation: exception ..... \n");
		}
	}
	sortBy_ = boost::lexical_cast<std::string>(stationid_);
	return true;
}

std::string kvalobs::kvReferenceStation::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE stationid=" << stationid_ << " AND " << "       paramsetid="
			<< paramsetid_;

	return ost.str();
}
