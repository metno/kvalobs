/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvTimecontrol.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef _kvTimecontrol_h
#define _kvTimecontrol_h

#include <kvalobs/kvDbBase.h>

/* Created by DNMI/FoU/PU: j.schulze@dnmi.no
 at Mon Aug 26 14:27:59 2002 */
/* Edited by T.Reite 9. October 2002 */

namespace kvalobs
{

/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table timecontrol in the kvalobs database.
 */

class kvTimecontrol: public kvDbBase
{
private:
	int fromday_;
	int today_;
	int time_;
	int priority_;
	std::string qcx_;

public:
	kvTimecontrol()
	{
	}
	kvTimecontrol(const dnmi::db::DRow& r)
	{
		set(r);
	}
	kvTimecontrol(int fr, int to, int ti, int pr, const std::string& qc)
	{
		set(fr, to, ti, pr, qc);
	}

	bool set(const dnmi::db::DRow&);
	bool set(int, int, int, int, const std::string&);

	const char* tableName() const
	{
		return "timecontrol";
	}
	std::string toSend() const;
	std::string uniqueKey() const;

	int fromday() const
	{
		return fromday_;
	}
	int today() const
	{
		return today_;
	}
	int time() const
	{
		return time_;
	}
	int priority() const
	{
		return priority_;
	}
	std::string qcx() const
	{
		return qcx_;
	}

	miutil::miDate fromDAY(int y = -1) const
	{
		return julianDayThatYear(fromday_, y);
	}

	miutil::miDate toDAY(int y = -1) const
	{
		return julianDayThatYear(today_, y);
	}

};

/** @} */
}

#endif
