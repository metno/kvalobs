/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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


#include "sql/SqlKvApp.h"
#include <kvalobs/kvPath.h>
#include <miutil/timeconvert.h>
#include <iostream>
#include <stdexcept>
#include <memory>

using kvservice::KvApp;

kvservice::KvApp * getApp(int argc, char ** argv)
{
	using kvservice::sql::SqlKvApp;
	return new SqlKvApp(argc, argv, SqlKvApp::readConf({"runit.conf", "kvalobs.conf", kvPath("sysconfdir") + "/kvalobs.conf"}));
}

void readKvData()
{
	kvservice::KvObsDataList dataList;
	kvservice::WhichDataHelper whichData;
	whichData.addStation(4260,
			boost::posix_time::time_from_string("2015-11-09 12:00:00"),
			boost::posix_time::time_from_string("2015-11-09 12:00:00")
			);
	whichData.addStation(180,
			boost::posix_time::time_from_string("2015-11-09 12:00:00"),
			boost::posix_time::time_from_string("2015-11-09 12:00:00")
			);
	if ( ! KvApp::kvApp->getKvData(dataList, whichData) )
		throw std::runtime_error("Unable to read data");
	for ( auto dl : dataList )
	{
		for ( auto d: dl.dataList() )
			std::cout << "d :" << d.stationID() << '/' << d.paramID() << ":\t" << d.original() << std::endl;
		for ( auto d: dl.textDataList() )
			std::cout << "t: " << d.stationID() << '/' << d.paramID() << ":\t" << d.original() << std::endl;

		std::cout << std::endl;
	}
}

void readParam()
{
	std::list<kvalobs::kvParam> paramList;
	if ( ! KvApp::kvApp->getKvParams(paramList) )
		throw std::runtime_error("Unable to read param");
	for ( auto p : paramList )
		std::cout << p.name() << ":\t" << p.paramID() << std::endl;
}


int main(int argc, char ** argv)
{
	std::unique_ptr<kvservice::KvApp> app(getApp(argc, argv));
	//readParam();
	readKvData();
}
