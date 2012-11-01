/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvStationInfoExt.cc,v 1.4.6.2 2007/09/27 09:02:31 paule Exp $

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
#include <kvalobs/kvStationInfoExt.h>
#include <miutil/timeconvert.h>

using namespace std;

kvalobs::kvStationInfoExt::kvStationInfoExt(
		const kvalobs::kvStationInfoExt &info) :
		stationid_(info.stationid_), obstime_(info.obstime_), typeid_(
				info.typeid_), params_(info.params_)
{
}

kvalobs::kvStationInfoExt&
kvalobs::kvStationInfoExt::operator=(const kvStationInfoExt &info)
{
	if (this == &info)
		return *this;

	stationid_ = info.stationid_;
	obstime_ = info.obstime_;
	typeid_ = info.typeid_;
	params_ = info.params_;

	return *this;
}

void kvalobs::kvStationInfoExt::addParam(const Param &param)
{
	for (list<Param>::iterator it = params_.begin(); it != params_.end(); ++it)
		if (it->paramid == param.paramid && it->sensor == param.sensor
				&& it->level == param.level)
			return;

	params_.push_back(param);
}

void kvalobs::populateCorbaKvStationInfoExtList(
		const kvalobs::kvStationInfoExtList &stList,
		CKvalObs::StationInfoExtList &cstList)
{
	kvalobs::kvStationInfoExtList::const_iterator it;
	CORBA::Long i, k;
	list<kvalobs::kvStationInfoExt::Param> params;
	list<kvalobs::kvStationInfoExt::Param>::const_iterator pit;

	if (stList.empty())
	{
		cstList.length(0);
		return;
	}

	cstList.length(stList.size());
	it = stList.begin();

	for (it = stList.begin(), k = 0; it != stList.end(); ++it, ++k)
	{
		cstList[k].stationId = it->stationID();
		cstList[k].obstime = CORBA::string_dup(to_kvalobs_string(it->obstime()).c_str());
		cstList[k].typeId_ = it->typeID();
		params = it->params();

		cstList[k].params.length(params.size());
		for (pit = params.begin(), i = 0; pit != params.end(); ++pit, ++i)
		{
			cstList[k].params[i].paramid = pit->paramid;
			cstList[k].params[i].sensor = pit->sensor;
			cstList[k].params[i].level = pit->level;
		}
	}

}

std::ostream&
kvalobs::operator<<(std::ostream& os, const kvalobs::kvStationInfoExt &inf_)
{
	kvalobs::kvStationInfoExt &inf =
			const_cast<kvalobs::kvStationInfoExt&>(inf_);

	os << "kvStationInfoExt: stationid: " << inf.stationID() << " obstime: "
			<< inf.obstime() << " typeId: " << inf.typeID() << " #params: "
			<< inf.params_.size() << endl;

	return os;
}
