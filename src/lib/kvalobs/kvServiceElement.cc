/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvServiceElement.cc,v 1.2.2.2 2007/09/27 09:02:31 paule Exp $

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
#include <kvalobs/kvServiceElement.h>
#include <sstream>

using namespace std;
using namespace miutil;
using namespace dnmi;

void kvalobs::kvServiceElement::createSortIndex()
{
	std::ostringstream s;
	s << stationid_;
	s << obstime_.isoTime();
	s << typeid_;
	sortBy_ = s.str();
}

kvalobs::kvServiceElement::kvServiceElement()
{
}

bool kvalobs::kvServiceElement::set(int sid, const miutil::miTime &obt, int tid)
{
	stationid_ = sid;
	obstime_ = obt;
	typeid_ = tid;

	createSortIndex();

	return true;
}

bool kvalobs::kvServiceElement::set(const dnmi::db::DRow& r_)
{
	db::DRow &r = const_cast<db::DRow&>(r_);
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
			else if (*it == "obstime")
			{
				obstime_ = miTime(buf);
			}
			else if (*it == "typeid")
			{
				typeid_ = atoi(buf.c_str());
			}
			else
			{
				CERR(
						"kvServiceElement::set .. unknown entry:" << *it << std::endl);
			}
		} catch (...)
		{
			CERR("kvServiceElement: unexpected exception ..... \n");
		}
	}

	createSortIndex();
	return true;

}

std::string kvalobs::kvServiceElement::toSend() const
{
	ostringstream ost;

	ost << "(" << stationid_ << "," << quoted(obstime_) << "," << typeid_
			<< ")";

	return ost.str();

}

std::string kvalobs::kvServiceElement::toUpdate() const
{
	//Nothing to update
	return "";
}

std::string kvalobs::kvServiceElement::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE stationid=" << stationid_ << " AND " << "       obstime="
			<< quoted(obstime_.isoTime()) << " AND " << "       typeid="
			<< typeid_;

	return ost.str();

}

