/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvTextData.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef _kvTextData_h
#define _kvTextData_h

#include <kvalobs/kvDbBase.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace kvalobs
{

/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table text_data in the kvalobs database.
 */

class kvTextData: public kvDbBase
{
private:
	int stationid_;
	boost::posix_time::ptime obstime_;
	std::string original_;
	int paramid_;
	boost::posix_time::ptime tbtime_;
	int tbtimemsec_;
	int typeid_;

public:

	kvTextData()
	{
	}
	;
	kvTextData(const dnmi::db::DRow& r)
	{
		set(r);
	}
	kvTextData(int sta, const boost::posix_time::ptime& obt, const std::string& org,
			int pid, const boost::posix_time::ptime& tbt, int typ)
	{
		set(sta, obt, org, pid, tbt, typ);
	}

	bool set(const dnmi::db::DRow&);
	bool set(int sta, const boost::posix_time::ptime& obt, const std::string& org,
			int pid, const boost::posix_time::ptime& tbt, int typ);

	const char* tableName() const
	{
		return "text_data";
	}
	std::string toSend() const;
	std::string uniqueKey() const;

	int stationID() const
	{
		return stationid_;
	}
	boost::posix_time::ptime obstime() const
	{
		return obstime_;
	}
	std::string original() const
	{
		return original_;
	}
	int paramID() const
	{
		return paramid_;
	}
	boost::posix_time::ptime tbtime() const
	{
		return tbtime_;
	}
	int tbtimemsec() const
	{
		return tbtimemsec_;
	}
	int typeID() const
	{
		return typeid_;
	}

	void tbtime(const boost::posix_time::ptime &tbtime)
	{
		tbtime_ = tbtime;
	}
	void typeID(int t)
	{
		typeid_ = t;
	}
};

/** @} */
}
;

#endif

