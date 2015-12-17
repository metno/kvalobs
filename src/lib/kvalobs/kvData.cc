/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvData.cc,v 1.18.6.2 2007/09/27 09:02:30 paule Exp $

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
#include <list>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <kvalobs/kvData.h>
#include <miutil/timeconvert.h>
#include <dnmithread/mtcout.h>
/*
 * Created by DNMI/IT: borge.moe@met.no
 * at Tue Aug 28 07:53:16 2002 
 */

using namespace std;
using namespace dnmi;
namespace pt = boost::posix_time;

kvalobs::kvData::kvData(const kvalobs::kvData &d) :
		stationid_(d.stationid_), obstime_(d.obstime_), original_(d.original_), paramid_(
				d.paramid_), tbtime_(d.tbtime_), typeid_(d.typeid_), sensor_(d.sensor_), level_(d.level_), corrected_(
				d.corrected_), controlinfo_(d.controlinfo_), useinfo_(
				d.useinfo_), cfailed_(d.cfailed_)
{
	createSortIndex();
}

void kvalobs::kvData::createSortIndex()
{
	std::ostringstream s;
	s << stationid_;
	s << paramid_;
	s << sensor_;
	s << level_;
	s << obstime_;
	sortBy_ = s.str();
}

void kvalobs::kvData::clean()
{
	stationid_ = 0;
	obstime_ = pt::second_clock::universal_time();
	original_ = 0;
	paramid_ = 0;
	tbtime_ = pt::second_clock::universal_time();
	typeid_ = 0;
	sensor_ = 0;
	level_ = 0;
	corrected_ = 0;
	controlinfo_.clear();
	useinfo_.clear();
	cfailed_ = "";
	createSortIndex();
}

bool kvalobs::kvData::set(const dnmi::db::DRow &r_)
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
				obstime_ = pt::time_from_string_nothrow(buf);
			}
			else if (*it == "original")
			{
				sscanf(buf.c_str(), "%f", &original_);
			}
			else if (*it == "paramid")
			{
				paramid_ = atoi(buf.c_str());
			}
			else if (*it == "tbtime")
			{
				tbtime_ = pt::time_from_string_nothrow(buf);
			}
			else if (*it == "typeid")
			{
				typeid_ = atoi(buf.c_str());
			}
			else if (*it == "sensor")
			{
				sensor_ = atoi(buf.c_str());
			}
			else if (*it == "level")
			{
				level_ = atoi(buf.c_str());
			}
			else if (*it == "corrected")
			{
				sscanf(buf.c_str(), "%f", &corrected_);
			}
			else if (*it == "controlinfo")
			{
				controlinfo_ = kvControlInfo(buf);
			}
			else if (*it == "useinfo")
			{
				useinfo_ = kvUseInfo(buf);
			}
			else if (*it == "cfailed")
			{
				cfailed_ = buf;
			}
			else
			{
				CERR("kvData::set .. unknown entry:" << *it << std::endl);
			}
		} catch (...)
		{
			CERR("kvData: unexpected exception ..... \n");
		}
	}

	createSortIndex();
	return true;
}

kvalobs::kvData&
kvalobs::kvData::operator=(const kvalobs::kvData &rhs)
{
	if (this != &rhs)
	{
		stationid_ = rhs.stationid_;
		obstime_ = rhs.obstime_;
		original_ = rhs.original_;
		paramid_ = rhs.paramid_;
		tbtime_ = rhs.tbtime_;
		typeid_ = rhs.typeid_;
		sensor_ = rhs.sensor_;
		level_ = rhs.level_;
		corrected_ = rhs.corrected_;
		controlinfo_ = rhs.controlinfo_;
		useinfo_ = rhs.useinfo_;
		cfailed_ = rhs.cfailed_;

		createSortIndex();
	}

	return *this;
}

bool kvalobs::kvData::set(int pos, const boost::posix_time::ptime &obt, float org,
		int par, const boost::posix_time::ptime &tbt, int typ, int sen, int lvl,
		float cor, const kvControlInfo &cin, const kvUseInfo &uin,
		const std::string &fai)
{
	stationid_ = pos;
	obstime_ = obt;
	original_ = org;
	paramid_ = par;
	tbtime_ = tbt;
	typeid_ = typ;
	sensor_ = sen;
	level_ = lvl;
	corrected_ = cor;
	controlinfo_ = cin;
	useinfo_ = uin;
	cfailed_ = fai;

	createSortIndex();

	return true;
}

bool kvalobs::kvData::set(int pos, const boost::posix_time::ptime &obt, float org,
		int par, const boost::posix_time::ptime &tbt, int typ, int lvl)
{
	stationid_ = pos;
	obstime_ = obt;
	original_ = org;
	paramid_ = par;
	tbtime_ = tbt;
	typeid_ = typ;
	level_ = lvl;

	corrected_ = org;

	createSortIndex();

	return true;
}

std::string kvalobs::kvData::toSend() const
{
	ostringstream ost;

	ost << "(" << stationid_ << "," << quoted(to_kvalobs_string(obstime_)) << "," << original_
			<< "," << paramid_ << "," << quoted(to_kvalobs_string(tbtime_)) << "," << typeid_
			<< "," << quoted(sensor_) << "," << level_ << "," << corrected_
			<< "," << quoted(controlinfo_.flagstring()) << ","
			<< quoted(useinfo_.flagstring()) << "," << quoted(cfailed_) << ")";

	return ost.str();
}

std::string kvalobs::kvData::toUpload() const
{
	ostringstream ost;

	ost << stationid_ << "," << to_kvalobs_string(obstime_) << "," << original_ << "," << paramid_
			<< "," << to_kvalobs_string(tbtime_) << "," << typeid_ << "," << sensor_ << ","
			<< level_ << "," << corrected_ << "," << controlinfo_.flagstring()
			<< "," << useinfo_.flagstring() << "," << cfailed_;

	return ost.str();
}

std::string kvalobs::kvData::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE stationid=" << stationid_ << " AND " << "       obstime="
			<< quoted(to_kvalobs_string(obstime_)) << " AND " << "       paramid="
			<< paramid_ << " AND " << "       typeid=" << typeid_ << " AND "
			<< "       sensor=" << quoted(sensor_) << " AND " << "       level="
			<< level_;

	return ost.str();
}

std::string kvalobs::kvData::toUpdate() const
{
	ostringstream ost;

	ost << "SET corrected=" << corrected_ << ", controlinfo="
			<< quoted(controlinfo_.flagstring()) << ", useinfo="
			<< quoted(useinfo_.flagstring()) << ", cfailed=" << quoted(cfailed_)
			<< " WHERE stationid=" << stationid_ << " AND " << "       obstime="
			<< quoted(to_kvalobs_string(obstime_)) << " AND " << "       paramid="
			<< paramid_ << " AND " << "       typeid=" << typeid_ << " AND "
			<< "       sensor=" << quoted(sensor_) << " AND " << "       level="
			<< level_;

	return ost.str();

}

void kvalobs::kvData::useinfo(int flag, char newVal)
{
	useinfo_.set(flag, newVal);
}

std::ostream&
kvalobs::operator<<(std::ostream& output, const kvalobs::kvData& d)
{
	output << "[" << "sid: " << d.stationID() << " otime: " << d.obstime()
			<< " tid: " << d.typeID() << " pid: " << d.paramID() << " lvl: "
			<< d.level() << " sen: " << d.sensor() << " orig: " << d.original()
			<< " cor: " << d.corrected() << " tbt: "<< d.tbtime() << " cinfo: " << d.controlinfo()
			<< " uinfo: " << d.useinfo() << "]";

	return output;
}

