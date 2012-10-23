/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvTypes.cc,v 1.7.2.2 2007/09/27 09:02:31 paule Exp $

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
#include <kvalobs/kvTypes.h>

using namespace std;
using namespace miutil;

std::string kvalobs::kvTypes::toSend() const
{
	ostringstream ost;
	ost << "(" << typeid_ << "," << quoted(format_) << "," << earlyobs_ << ","
			<< lateobs_ << "," << quoted(read_) << "," << quoted(obspgm_) << ","
			<< quoted(comment_) << ")";
	return ost.str();
}

bool kvalobs::kvTypes::set(int typeid__, std::string format__, int earlyobs__,
		int lateobs__, std::string read__, std::string obspgm__,
		std::string comment__)
{
	typeid_ = typeid__;
	format_ = format__;
	earlyobs_ = earlyobs__, lateobs_ = lateobs__, read_ = read__, obspgm_ =
			obspgm__, comment_ = comment__;
	sortBy_ = std::string(format_);
	return true;
}

bool kvalobs::kvTypes::set(const dnmi::db::DRow& r_)
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
			if (*it == "typeid")
			{
				typeid_ = atoi(buf.c_str());
			}
			else if (*it == "format")
			{
				format_ = buf;
			}
			else if (*it == "earlyobs")
			{
				earlyobs_ = atoi(buf.c_str());
			}
			else if (*it == "lateobs")
			{
				lateobs_ = atoi(buf.c_str());
			}
			else if (*it == "read")
			{
				read_ = buf;
			}
			else if (*it == "obspgm")
			{
				read_ = buf;
			}
			else if (*it == "comment")
			{
				comment_ = buf;
			}
		} catch (...)
		{
			CERR("kvTypes: exception ..... \n");
		}
	}

	sortBy_ = std::string(format_);
	return true;
}

std::string kvalobs::kvTypes::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE typeid=" << typeid_;

	return ost.str();
}

