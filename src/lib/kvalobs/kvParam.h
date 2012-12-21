/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvParam.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvParam_h__
#define __kvParam_h__

#include <kvalobs/kvDbBase.h>

/*
 * Created by DNMI/IT: borge.moe@met.no
 * at Tue Aug 28 07:53:16 2002
 */

namespace kvalobs
{

/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table param in the kvalobs database.
 */

class kvParam: public kvDbBase
{
private:
	int paramid_;
	std::string name_;
	std::string description_;
	std::string unit_;
	int level_scale_;
	std::string comment_;

public:
	kvParam()
	{
	}
	kvParam(const dnmi::db::DRow &r)
	{
		set(r);
	}
	kvParam(int paramid, const std::string &name,
			const std::string &description, const std::string &unit,
			int level_scale, const std::string &comment)
	{
		set(paramid, name, description, unit, level_scale, comment);
	}

	bool set(int paramid, const std::string &name,
			const std::string &description, const std::string &unit,
			int level_scale, const std::string &comment);

	bool set(const dnmi::db::DRow&);
	const char* tableName() const
	{
		return "param";
	}
	std::string toSend() const;
	std::string uniqueKey() const;

	int paramID() const
	{
		return paramid_;
	}
	const std::string & name() const
	{
		return name_;
	}
	const std::string & description() const
	{
		return description_;
	}
	const std::string & unit() const
	{
		return unit_;
	}
	int level_scale() const
	{
		return level_scale_;
	}
	const std::string & comment() const
	{
		return comment_;
	}
};

/** @} */
}

#endif

