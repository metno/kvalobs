/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvStationInfoCommand.cc,v 1.6.6.2 2007/09/27 09:02:31 paule Exp $

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
#include <kvalobs/kvStationInfoCommand.h>

using namespace miutil;
using namespace kvalobs;
using namespace std;

kvalobs::StationInfoCommand::StationInfoCommand()
{
}

kvalobs::StationInfoCommand::StationInfoCommand(
		const CKvalObs::StationInfoList &st)

{
	for (CORBA::Long i = 0; i < st.length(); i++)
	{
		kvStationInfoExt stationInfo(st[i].stationId, miTime(st[i].obstime),
				st[i].typeId_);
		stationInfoList.push_back(stationInfo);
	}
}

kvalobs::StationInfoCommand::StationInfoCommand(
		const CKvalObs::StationInfoExtList &st)
{
	for (CORBA::Long i = 0; i < st.length(); i++)
	{
		kvStationInfoExt stationInfo(st[i].stationId, miTime(st[i].obstime),
				st[i].typeId_);
		for (CORBA::Long j = 0; j < st[i].params.length(); ++j)
			stationInfo.addParam(
					kvStationInfoExt::Param(st[i].params[j].paramid,
							st[i].params[j].sensor, st[i].params[j].level));

		stationInfoList.push_back(stationInfo);
	}

}

kvalobs::StationInfoCommand::StationInfoCommand(
		const CKvalObs::StationInfoExt &st)
{
	kvStationInfoExt sti(st.stationId, miutil::miTime(st.obstime), st.typeId_);

	for (CORBA::Long i = 0; i < st.params.length(); ++i)
		sti.addParam(
				kvStationInfoExt::Param(st.params[i].paramid,
						st.params[i].sensor, st.params[i].level));

}

kvalobs::StationInfoCommand::StationInfoCommand(const CKvalObs::StationInfo &st)
{
	stationInfoList.push_back(
			kvStationInfoExt(st.stationId, miTime(st.obstime), st.typeId_));
}

bool kvalobs::StationInfoCommand::addStationInfo(const kvStationInfo &si)
{
	kvStationInfoExt sti(si.stationID(), si.obstime(), si.typeID());

	try
	{
		stationInfoList.push_back(sti);
		return true;
	} catch (...)
	{
		return false;
	}
}

bool kvalobs::StationInfoCommand::addStationInfo(
		const kvStationInfoExt &stationInfo)
{
	try
	{
		stationInfoList.push_back(stationInfo);
		return true;
	} catch (...)
	{
		return false;
	}
}

void kvalobs::StationInfoCommand::debugInfo(std::ostream &os) const
{
	CIkvStationInfoExtList it = stationInfoList.begin();

	os << "[StationInfoCommand: " << endl;

	for (; it != stationInfoList.end(); it++)
		os << *it;

	os << ":StationInfoCommand]" << endl;
}

kvalobs::kvStationInfoList&
kvalobs::StationInfoCommand::getStationInfo()
{
	stationInfoListLegacy.clear();

	for (kvStationInfoExtList::const_iterator it = stationInfoList.begin();
			it != stationInfoList.end(); ++it)
		stationInfoListLegacy.push_back(
				kvStationInfo(it->stationID(), it->obstime(), it->typeID()));

	return stationInfoListLegacy;
}

const kvalobs::kvStationInfoList&
kvalobs::StationInfoCommand::getStationInfo() const
{

	const_cast<kvalobs::StationInfoCommand*>(this)->stationInfoListLegacy.clear();

	for (kvStationInfoExtList::const_iterator it = stationInfoList.begin();
			it != stationInfoList.end(); ++it)
		const_cast<kvalobs::StationInfoCommand*>(this)->stationInfoListLegacy.push_back(
				kvStationInfo(it->stationID(), it->obstime(), it->typeID()));

	return stationInfoListLegacy;
}
