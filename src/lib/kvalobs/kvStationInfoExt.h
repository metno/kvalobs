/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvStationInfo.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvStationInfoExt_h__
#define __kvStationInfoExt_h__

#include <iostream>
#include <list>
#include <puTools/miTime.h>
#include <kvalobs/kvDataFlag.h>
#include <kvalobs/kvData.h>
#include <kvskel/commonStationInfo.hh>

namespace kvalobs
{
/**
 * \addtogroup kvinternalhelpers
 * @{
 */

/**
 * \brief This class is used as an interface to the CORBA interface
 * CKvalObs::StationInfo.
 *
 * The class plays an importen role for the data flow in the
 * kvalobs system.
 */
class kvStationInfoExt
{
public:
	struct Param
	{
		int paramid;
		int sensor;
		int level;

		Param(int pid, int sensor = 0, int level = 0) :
				paramid(pid), sensor(sensor), level(level)
		{
		}
	};

private:
	long stationid_;
	miutil::miTime obstime_;
	int typeid_;
	std::list<Param> params_;

public:
	/**
	 * \brief Initialize the object.
	 *
	 * \param stationid The kvalobs stationid.
	 * \param obstime The observation time for this observation.
	 * \param typeId The typeid to the observation.
	 */
	kvStationInfoExt(long stationid, const miutil::miTime &obstime, int typeId) :
			stationid_(stationid), obstime_(obstime), typeid_(typeId)
	{
	}

	kvStationInfoExt(long stationid, const miutil::miTime &obstime, int typeId,
			int paramid) :
			stationid_(stationid), obstime_(obstime), typeid_(typeId)
	{
		params_.push_back(Param(paramid));
	}

	kvStationInfoExt(long stationid, const miutil::miTime &obstime, int typeId,
			std::list<kvStationInfoExt::Param> &params) :
			stationid_(stationid), obstime_(obstime), typeid_(typeId), params_(
					params)
	{
	}

	kvStationInfoExt(const kvStationInfoExt &info);
	kvStationInfoExt &operator=(const kvStationInfoExt &info);

	///The stationid for this observation
	long stationID() const
	{
		return stationid_;
	}

	///The observation time for this observation
	miutil::miTime obstime() const
	{
		return obstime_;
	}

	///The typeid for this observation
	int typeID() const
	{
		return typeid_;
	}

	std::list<kvStationInfoExt::Param> params() const
	{
		return params_;
	}

	void addParam(const kvStationInfoExt::Param &param);

	friend std::ostream& operator<<(std::ostream& os,
			const kvStationInfoExt &c);

};

typedef std::list<kvStationInfoExt> kvStationInfoExtList;
typedef std::list<kvStationInfoExt>::iterator IkvStationInfoExtList;
typedef std::list<kvStationInfoExt>::const_iterator CIkvStationInfoExtList;

void populateCorbaKvStationInfoExtList(
		const kvalobs::kvStationInfoExtList &stList,
		CKvalObs::StationInfoExtList &cstList);

inline bool operator ==(const kvStationInfoExt & a, const kvStationInfoExt & b)
{
	return a.stationID() == b.stationID() and a.obstime() == b.obstime()
			and a.typeID() == b.typeID();
}
inline bool operator !=(const kvStationInfoExt & a, const kvStationInfoExt & b)
{
	return not operator ==(a, b);
}

/** @} */
}
;

#endif
