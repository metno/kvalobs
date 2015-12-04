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
#include <boost/timer/timer.hpp>
#include <kvalobs/kvPath.h>
#include <miutil/timeconvert.h>
#include <iostream>
#include <stdexcept>
#include <memory>

using boost::timer::cpu_timer;
using kvservice::KvApp;

static const std::string OK = "\t\033[1;32m+";
static const std::string NOT_OK = "\t\033[1;31m-";
static const std::string EOL = "\033[0m\n";

kvservice::KvApp * getApp(int argc, char ** argv)
{
	using kvservice::sql::SqlKvApp;
	return new SqlKvApp(argc, argv, SqlKvApp::readConf({"runit.conf", "kvalobs.conf", kvPath("sysconfdir") + "/kvalobs.conf"}));
}

int test(std::string testDescription, bool test) {
	if ( test ) {
		std::cout << OK << testDescription << EOL;
		return 1;
	}
	std::cout << NOT_OK << testDescription << EOL;
	return 0;
}


void readModelData()
{
	kvservice::WhichDataHelper whichData;
	whichData.addStation(100,
			boost::posix_time::time_from_string("2015-12-02 00:00:00"),
			boost::posix_time::time_from_string("2015-12-02 12:00:00"));

	std::list<kvalobs::kvModelData> data;
	if ( ! KvApp::kvApp->getKvModelData(data, whichData) )
		throw std::runtime_error("Unable to read data");
	for ( auto d : data )
		std::cout << d.insertQuery(false) << std::endl;
}

void readData()
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

int readParams()
{
	std::list<kvalobs::kvParam> paramList;
	return test(" retrieve parameters from the database. ",
			KvApp::kvApp->getKvParams(paramList));
	//for ( auto p : paramList )
	//	std::cout << p.name() << ":\t" << p.paramID() << std::endl;
}

void readWorkStatistik()
{
	kvservice::WorkstatistikIterator it;
	if (!KvApp::kvApp->getKvWorkstatistik(
			CKvalObs::CService::TbTime,
			boost::posix_time::time_from_string("2015-11-09 12:00:00"),
			boost::posix_time::time_from_string("2015-11-09 12:30:00"),
			it))
		throw std::runtime_error("Unable to get workstatistik messages");
	kvalobs::kvWorkelement e;
	while (it.next(e))
		std::cout << e.insertQuery(false) << std::endl;
}

void readRejectDecode()
{
	CKvalObs::CService::RejectDecodeInfo decodeInfo;
	decodeInfo.fromTime = "2015-11-10 12:00:00";
	decodeInfo.toTime = "2015-11-10 12:30:00";
	kvservice::RejectDecodeIterator it;
	if ( ! KvApp::kvApp->getKvRejectDecode(decodeInfo, it) )
		throw std::runtime_error("Unable to get rejectdecode messages");
	kvalobs::kvRejectdecode d;
	while ( it.next(d) )
		std::cout << d.tbtime() << ":\t" << d.message() << std::endl;
}

void readStationMetaData()
{
	std::list<kvalobs::kvStationMetadata> stMeta;
	int stationid = 0;
	auto obstime = boost::posix_time::time_from_string("2015-11-24 12:00:00");
	std::string metadataName; // = "VS";

	if ( ! KvApp::kvApp->getKvStationMetaData(stMeta, stationid, obstime, metadataName) )
		throw std::runtime_error("Unable to read metadata");
	for ( auto m : stMeta )
		std::cout << "METADATA: " << m.paramID() << " (" << m.name() << "): " << m.metadata() << std::endl;
}

int readStations()
{
	std::list<kvalobs::kvStation> stationList;
	return test(" retrieve stations from the database. ",
				KvApp::kvApp->getKvStations(stationList));
	//for ( auto s : stationList )
	//	std::cout << s.name() << ":\t" << s.stationID() << std::endl;
}

void readObsPgm()
{
	std::list<kvalobs::kvObsPgm> obsPgm;
	const std::list<long> stationList = {180, 100};

	if ( ! KvApp::kvApp->getKvObsPgm(obsPgm, stationList, true) )
		throw std::runtime_error("Unable to read metadata");

	for ( auto op : obsPgm )
		std::cout << op.insertQuery(false) << std::endl;
}


int main(int argc, char ** argv)
{
	std::unique_ptr<kvservice::KvApp> app(getApp(argc, argv));
	const int totalTests = 2;
	int t = 0;
	std::cout << "\033[1;36mSqlKvApp should" << EOL;
	cpu_timer test_timer;
	// Actual tests
	t += readParams();
	t += readStations();
	//readWorkStatistik();
	//readKvData();
	//readObsPgm();
	//readModelData();
	//readStationMetaData();
	// Print out results
	int time = test_timer.elapsed().wall / 1000000;
	std::cout << "\033[1;36mFinished in " << time << "ms" << EOL;
	std::cout << "\033[1;36mPassed " << t << " of " << totalTests << " tests"<< EOL;
	return (t == totalTests ? 0 : 1);
}
