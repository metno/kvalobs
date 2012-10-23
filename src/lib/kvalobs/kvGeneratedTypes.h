/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvGeneratedTypes.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef _kvGeneratedTypes_h
#define _kvGeneratedTypes_h

#include <kvalobs/kvDbBase.h>

/* autogenerated c++ header file from kv databasescript */

namespace kvalobs
{

/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table generated_types in the kvalobs database.
 */

class kvGeneratedTypes: public kvDbBase
{
private:
	int stationid_;
	int typeid_;

public:

	kvGeneratedTypes()
	{
	}
	;
	kvGeneratedTypes(const dnmi::db::DRow& r)
	{
		set(r);
	}
	kvGeneratedTypes(int st, int ty)
	{
		set(st, ty);
	}

	bool set(int, int);
	bool set(const dnmi::db::DRow&);

	std::string uniqueKey() const;

	const char* tableName() const
	{
		return "generated_types";
	}
	std::string toSend() const;

	int stationID() const
	{
		return stationid_;
	}
	int typeID() const
	{
		return typeid_;
	}

};

/** @} */
}
;

#endif

