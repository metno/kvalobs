/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvParam.cc,v 1.7.2.2 2007/09/27 09:02:30 paule Exp $

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
#include <stdlib.h>
#include <stdio.h>
#include <kvalobs/kvParam.h>
#include <dnmithread/mtcout.h>
#include <boost/lexical_cast.hpp>
/*
 * Created by DNMI/IT: borge.moe@met.no
 * at Tue Aug 28 15:23:16 2002 
 */
using namespace std;
using namespace dnmi;

bool kvalobs::kvParam::set(const dnmi::db::DRow &r_)
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

			if (*it == "paramid")
			{
				paramid_ = atoi(buf.c_str());
			}
			else if (*it == "name")
			{
				name_ = buf;
			}
			else if (*it == "description")
			{
				description_ = buf;
			}
			else if (*it == "unit")
			{
				unit_ = buf;
			}
			else if (*it == "level_scale")
			{
				level_scale_ = atoi(buf.c_str());
			}
			else if (*it == "comment")
			{
				comment_ = buf;
			}
		} catch (...)
		{
			CERR("kvParam: unexpected exception ..... \n");
		}
	}

	sortBy_ = boost::lexical_cast<std::string>(paramid_);
	return true;
}

bool kvalobs::kvParam::set(int paramid, const std::string &name,
		const std::string &description, const std::string &unit,
		int level_scale, const std::string &comment)
{
	paramid_ = paramid;
	name_ = name;
	description_ = description;
	unit_ = unit;
	level_scale_ = level_scale;
	comment_ = comment;

	sortBy_ = boost::lexical_cast<std::string>(paramid_);

	return true;
}

std::string kvalobs::kvParam::toSend() const
{
	ostringstream ost;

	ost << "(" << paramid_ << "," << quoted(name_) << ","
			<< quoted(description_) << "," << quoted(unit_) << ","
			<< level_scale_ << "," << quoted(comment_) << ")";

	return ost.str();
}

std::string kvalobs::kvParam::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE  paramid=" << paramid_;

	return ost.str();
}

