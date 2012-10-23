/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvDataOperations.cc,v 1.1.2.16 2007/09/27 09:02:30 paule Exp $

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
#include <kvalobs/kvDataOperations.h>
#include <boost/static_assert.hpp>
#include <cmath>

using namespace kvalobs::flag;

namespace kvalobs
{
namespace
{
const float rejectedValue_ = -32766;
const float missingValue_ = -32767;
}

bool valid(const kvData & d)
{
	return not missing(d) and not rejected(d);
}

bool corrected(const kvData & d)
{
	int missing = d.controlinfo().flag(fmis);
	return missing != 0 and missing != 3;
}

bool missing(const kvData & d)
{
	return d.controlinfo().flag(fmis) == 3;
}

bool original_missing(const kvData & d)
{
	return d.controlinfo().flag(fmis) & 1;
}

bool rejected(const kvData & d)
{
	return d.controlinfo().flag(fmis) == 2;
}

bool has_model_value(const kvData & d)
{
	return d.controlinfo().flag(fnum) == 6 and not hqc::hqc_touched(d);
}

void reject(kvData & d)
{
	if (valid(d))
	{
		kvControlInfo ci = d.controlinfo();
		int old_fmis = ci.flag(fmis);
		int new_fmis = old_fmis | 2;
		new_fmis &= 3; // remove edited flag
		ci.set(fmis, new_fmis);

		d.controlinfo(ci);
		d.corrected(original_missing(d) ? missingValue_ : rejectedValue_);
	}
}

void correct(kvData & d, float new_val)
{
	kvControlInfo ci = d.controlinfo();
	int old_fmis = ci.flag(fmis);

	int new_fmis = old_fmis & 1 ? 1 : 4;

	ci.set(fmis, new_fmis);

	d.controlinfo(ci);
	d.corrected(new_val);
}

kvData getMissingKvData(int stationID, const miutil::miTime & obsTime,
		int paramID, int typeID, int sensor, int level)
{
	kvDataFactory f(stationID, obsTime, typeID, sensor, level);
	return f.getMissing(paramID);
}

kvDataFactory::kvDataFactory(int stationID, const miutil::miTime & obstime,
		int typeID, int sensor, int level) :
		stationID_(stationID), obstime_(obstime), typeID_(typeID), sensor_(
				sensor), level_(level)
{
}

kvDataFactory::kvDataFactory(const kvData & d) :
		stationID_(d.stationID()), obstime_(d.obstime()), typeID_(d.typeID()), sensor_(
				d.sensor()), level_(d.level())
{
}

kvData kvDataFactory::getMissing(int paramID,
		const miutil::miTime & obstime) const
{
	kvData ret = getData(missingValue_, paramID, obstime);
	kvControlInfo ci = ret.controlinfo();
	ci.set(fmis, 3);
	ret.controlinfo(ci);
	return ret;
}

kvData kvDataFactory::getData(float val, int paramID,
		const miutil::miTime & obstime) const
{
	kvData ret(stationID_, obstime.undef() ? obstime_ : obstime, val, paramID,
			miutil::miTime::nowTime(), typeID_, sensor_, level_, val,
			kvControlInfo(), kvUseInfo(), std::string(""));
	return ret;

}

namespace hqc
{
static void setFhqc_(kvData & d, int val)
{
	kvControlInfo ci = d.controlinfo();
	ci.set(fhqc, val);
	d.controlinfo(ci);
}

void hqc_accept(kvData & d)
{
	//int f = d.controlinfo().flag( fhqc );
	//if ( ( f < 5 or f > 7 ) and valid( d ) )
	setFhqc_(d, 1);
}

void hqc_reject(kvData & d)
{
	::kvalobs::reject(d);
	setFhqc_(d, 0xA);
}

void hqc_correct(kvData & d, float new_val)
{
	::kvalobs::correct(d, new_val);
	setFhqc_(d, 7);
}

void hqc_interpol(kvData & d, float new_val)
{
	::kvalobs::correct(d, new_val);
	setFhqc_(d, 5);
}

void hqc_distribute(kvData & d, float new_val)
{
	::kvalobs::correct(d, new_val);

	const int old_fd = d.controlinfo().flag(fd);
	const int new_fd = (old_fd == 4 || old_fd == 8 || old_fd == 0xA) ? 0xA : 9;

	kvControlInfo ci = d.controlinfo();
	ci.set(fd, new_fd);
	ci.set(fhqc, 6); // manual redistribution
	d.controlinfo(ci);
}

void hqc_auto_correct(kvData & d, float new_val)
{
	if (d.controlinfo().flag(fd) < 2 or d.controlinfo().flag(fd) == 3)
	{
		if (original_missing(d))
			hqc_interpol(d, new_val);
		else
			hqc_correct(d, new_val);
	}
	else
		// Collected
		hqc_distribute(d, new_val);
}

bool hqc_touched(const kvData & d)
{
	return d.controlinfo().flag(fhqc);
}

bool hqc_corrected(const kvData & d)
{
	int f = d.controlinfo().flag(fhqc);
	return f >= 5 and f <= 7;
}

bool hqc_accepted(const kvData & d)
{
	return d.controlinfo().flag(fhqc) == 1;
}

bool hqc_rejected(const kvData & d)
{
	return d.controlinfo().flag(fhqc) == 0xA;
}
}

namespace compare
{

bool eq_sensor(int sA, int sB)
{
	return sA == sB or std::abs(sA - sB) == '0';
}

bool lt_sensor(int sA, int sB)
{
	while (sA >= '0')
		sA -= '0';
	while (sB >= '0')
		sB -= '0';
	return sA < sB;
}

bool lt_kvData::operator()(const kvData & a, const kvData & b) const
{
	if (a.stationID() != b.stationID())
		return a.stationID() < b.stationID();
	if (a.typeID() != b.typeID())
		return a.typeID() < b.typeID();
	if (a.level() != b.level())
		return a.level() < b.level();
	if (not eq_sensor(a.sensor(), b.sensor()))
		return lt_sensor(a.sensor(), b.sensor());
	if (a.obstime() != b.obstime())
		return a.obstime() < b.obstime();
	return a.paramID() < b.paramID();
}

bool lt_kvData::operator()(const kvData * a, const kvData * b) const
{
	return operator()(*a, *b);
}

bool same_kvData::operator()(const kvData & a, const kvData & b) const
{
	return a.stationID() == b.stationID() and a.typeID() == b.typeID()
			and a.level() == b.level() and eq_sensor(a.sensor(), b.sensor())
			and a.obstime() == b.obstime() and a.paramID() == b.paramID();
}

/*
 bool lt_kvData_without_paramID::operator()( const kvData & a, const kvData & b ) const
 {
 if ( a.stationID() != b.stationID() )
 return a.stationID() < b.stationID();
 if ( a.obstime() != b.obstime() )
 return a.obstime() < b.obstime();
 if ( a.typeID() != b.typeID() )
 return a.typeID() < b.typeID();
 if ( a.sensor() != b.sensor() )
 return a.sensor() < b.sensor();
 return a.level() < b.level();
 }
 */

bool same_obs_and_parameter::operator()(const kvData & a,
		const kvData & b) const
{
	return a.stationID() == b.stationID() and a.paramID() == b.paramID()
			and a.obstime() == b.obstime() and a.typeID() == b.typeID()
			and eq_sensor(a.sensor(), b.sensor()) and a.level() == b.level();
}

bool exactly_equal_ex_tbtime::operator()(const kvData & a,
		const kvData & b) const
{
	return same_obs_and_parameter()(a, b)
			and std::abs(a.original() - b.original()) < 0.01
			and std::abs(a.corrected() - b.corrected()) < 0.01
			and a.controlinfo() == b.controlinfo()
			and a.useinfo() == b.useinfo() and a.cfailed() == b.cfailed();
}

bool exactly_equal::operator()(const kvData & a, const kvData & b) const
{
	return exactly_equal_ex_tbtime()(a, b) and a.tbtime() == b.tbtime();
}

}

namespace flag
{
BOOST_STATIC_ASSERT( fhqc == 15);
BOOST_STATIC_ASSERT( ui15 == 15);
}
}
