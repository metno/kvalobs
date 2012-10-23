/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvKeyVal.cc,v 1.1.6.2 2007/09/27 09:02:30 paule Exp $

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
#include <kvalobs/kvKeyVal.h>
#include <milog/milog.h>

using namespace std;
using namespace miutil;
using namespace dnmi;

void kvalobs::kvKeyVal::createSortIndex()
{
	sortBy_ = std::string(package_) + std::string(key_);
}

void kvalobs::kvKeyVal::clean()
{
	package_.erase();
	key_.erase();
	val_.erase();

	createSortIndex();
}

bool kvalobs::kvKeyVal::set(const dnmi::db::DRow &r_)
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

			if (*it == "package")
			{
				package_ = buf;
			}
			else if (*it == "key")
			{
				key_ = buf;
			}
			else if (*it == "val")
			{
				val_ = buf;
			}
			else
			{
				LOGWARN("kvKeyVal::set .. unknown entry:" << *it << std::endl);
			}
		} catch (...)
		{
			LOGWARN("kvKeyVal: unexpected exception ..... \n");
		}
	}

	createSortIndex();
	return true;
}

bool kvalobs::kvKeyVal::set(const kvKeyVal &s)
{
	package_ = s.package_;
	key_ = s.key_;
	val_ = s.val_;

	createSortIndex();

	return true;
}

bool kvalobs::kvKeyVal::set(const std::string &package, const std::string &key,
		const std::string &val)
{
	package_ = package;
	key_ = key;
	val_ = val;

	createSortIndex();

	return true;
}

std::string kvalobs::kvKeyVal::toSend() const
{
	ostringstream ost;

	ost << "(" << quoted(package_) << "," << quoted(key_) << "," << quoted(val_)
			<< ")";

	return ost.str();
}

std::string kvalobs::kvKeyVal::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE package=" << quoted(package_) << " AND key=" << quoted(key_);

	return ost.str();
}

std::string kvalobs::kvKeyVal::toUpdate() const
{
	ostringstream ost;

	ost << "SET val=" << quoted(val_) << " WHERE package=" << quoted(package_)
			<< " AND key=" << quoted(key_);

	return ost.str();
}
