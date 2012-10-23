/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvAlgorithms.cc,v 1.3.6.2 2007/09/27 09:02:30 paule Exp $

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
#include <dnmithread/mtcout.h>
#include <kvalobs/kvAlgorithms.h>

using namespace std;
using namespace dnmi;
using namespace miutil;

std::string kvalobs::kvAlgorithms::toSend() const
{
	ostringstream ost;
	ost << "(" << language_ << "," << quoted(checkname_) << ","
			<< quoted(signature_) << "," << quoted(script_) << ")";

	return ost.str();
}

bool kvalobs::kvAlgorithms::set(const dnmi::db::DRow& r_)
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

			if (*it == "language")
			{
				language_ = atoi(buf.c_str());
			}
			else if (*it == "checkname")
			{
				checkname_ = buf;
			}
			else if (*it == "signature")
			{
				signature_ = buf;
			}
			else if (*it == "script")
			{
				script_ = buf;
			}
		} catch (...)
		{
			CERR("kvAlgorithms: unexpected exception ..... \n");
		}
	}

	sortBy_ = checkname_;

	return true;

}

bool kvalobs::kvAlgorithms::set(int language, const std::string &checkname,
		const std::string &signature, const std::string &script)
{

	language_ = language;
	checkname_ = checkname;
	signature_ = signature;
	script_ = script;

	sortBy_ = checkname_;

	return true;
}

std::string kvalobs::kvAlgorithms::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE checkname=" << quoted(checkname_) << " AND "
			<< "       language=" << language_;

	return ost.str();

}
