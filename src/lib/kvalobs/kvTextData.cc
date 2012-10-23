/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvTextData.cc,v 1.7.6.2 2007/09/27 09:02:31 paule Exp $

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
#include <kvalobs/kvTextData.h>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace miutil;

std::string kvalobs::kvTextData::toSend() const
{
	ostringstream ost;
	string myTbtime;

	if (tbtimemsec_ > 0)
	{
		ost << tbtime_ << "." << tbtimemsec_;
		myTbtime = ost.str();
	}
	else
	{
		ost << tbtime_;
		myTbtime = ost.str();
	}

	ost.str("");

	ost << "(" << stationid_ << "," << quoted(obstime_) << ","
			<< quoted(original_) << "," << paramid_ << "," << quoted(myTbtime)
			<< "," << typeid_ << ")";
	return ost.str();
}

bool kvalobs::kvTextData::set(int sta, const miutil::miTime& obt,
		const std::string& org, int pid, const miutil::miTime& tbt, int typ)
{
	stationid_ = sta;
	obstime_ = obt;
	original_ = org;
	paramid_ = pid;
	tbtime_ = tbt;
	tbtimemsec_ = 0;
	typeid_ = typ;
	sortBy_ = boost::lexical_cast<std::string>(sta) + obt.isoTime();
	return true;
}

bool kvalobs::kvTextData::set(const dnmi::db::DRow& r_)
{
	dnmi::db::DRow & r = const_cast<dnmi::db::DRow&>(r_);
	list<string> names = r.getFieldNames();
	list<string>::iterator it = names.begin();
	std::string buf;

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
			else if (*it == "original")
			{
				original_ = buf;
			}
			else if (*it == "paramid")
			{
				paramid_ = atoi(buf.c_str());
			}
			else if (*it == "tbtime")
			{
				tbtime_ = decodeTimeWithMsec(buf, tbtimemsec_);
			}
			else if (*it == "typeid")
			{
				typeid_ = atoi(buf.c_str());
			}
		} catch (...)
		{
			CERR("kvTextData: exception ..... \n");
		}
	}
	sortBy_ = boost::lexical_cast<std::string>(stationid_) + obstime_.isoTime();
	return true;
}

void kvalobs::kvTextData::tbtime(const miutil::miTime &tbtime, int msec)
{
	tbtime_ = tbtime;

	if (msec > 0)
		tbtimemsec_ = msec;
	else
		tbtimemsec_ = 0;
}

std::string kvalobs::kvTextData::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE stationid=" << stationid_ << " AND " << "       obstime="
			<< quoted(obstime_.isoTime()) << " AND " << "       paramid="
			<< paramid_ << " AND " << "       typeid=" << typeid_;

	return ost.str();
}

