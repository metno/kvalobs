/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvChecks.h,v 1.1.2.2 2007/09/27 09:02:29 paule Exp $

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
#ifndef __kvChecks_h__
#define __kvChecks_h__

#include <kvalobs/kvDbBase.h>

/* Created by DNMI/IT: borge.moe@met.no
 Aug 27 2002 */
/* Edited by T.Reite 5 jun 2003 */

namespace kvalobs
{

/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table checks in the kvalobs database.
 */
class kvChecks: public kvDbBase
{
private:
	int stationid_;
	std::string qcx_;
	std::string medium_qcx_;
	int language_;
	std::string checkname_;
	std::string checksignature_;
	std::string active_;
	boost::posix_time::ptime fromtime_;

public:
	kvChecks()
	{
	}
	kvChecks(const dnmi::db::DRow &r)
	{
		set(r);
	}
	kvChecks(int stationid, const std::string &qcx,
			const std::string &medium_qcx, int language,
			const std::string &checkname, const std::string &checksignature,
			const std::string &active, const boost::posix_time::ptime &fromtime)
	{
		set(stationid, qcx, medium_qcx, language, checkname, checksignature,
				active, fromtime);
	}

	bool set(int stationid, const std::string &qcx,
			const std::string &medium_qcx, int language,
			const std::string &checkname, const std::string &checksignature,
			const std::string &active, const boost::posix_time::ptime &fromtime);

	bool set(const dnmi::db::DRow&);
	const char* tableName() const
	{
		return "checks";
	}
	std::string toSend() const;
	std::string uniqueKey() const;

	int stationID() const
	{
		return stationid_;
	}
	std::string qcx() const
	{
		return qcx_;
	}
	std::string medium_qcx() const
	{
		return medium_qcx_;
	}
	int language() const
	{
		return language_;
	}
	std::string checkname() const
	{
		return checkname_;
	}
	std::string checksignature() const
	{
		return checksignature_;
	}
	std::string active() const
	{
		return active_;
	}
	const boost::posix_time::ptime fromtime() const
	{
		return fromtime_;
	}

	/*
	 miutil::miDate fromDAY(int y=-1) const
	 { return  julianDayThatYear(fromday_,y); }

	 miutil::miDate toDAY(int y=-1) const
	 { return  julianDayThatYear(today_,y); }
	 */
};

/** @} */
}
;
#endif
