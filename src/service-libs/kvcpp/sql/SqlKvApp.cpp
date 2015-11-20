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

#include "SqlKvApp.h"
#include <kvdb/kvdb.h>
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvPath.h>
#include <milog/milog.h>

namespace kvservice
{
namespace sql
{
namespace
{
std::string getValue(const std::string & key, const miutil::conf::ConfSection *conf)
{
	auto val = conf->getValue(key);
	if ( val.empty() )
		throw std::runtime_error("missing <" +  key + "> in config file");
	if ( val.size() > 1 )
		throw std::runtime_error("Too many entries for <" + key + "> in config file");
	return val.front().valAsString();
}

dnmi::db::Connection * createConnection(const miutil::conf::ConfSection *conf)
{
	std::string connectString = getValue("database.dbconnect", conf);
	std::string driver = kvalobs::kvPath(kvalobs::libdir) + "/kvalobs/db/" + getValue("database.dbdriver", conf);

	std::string driverId;
	static dnmi::db::DriverManager dbMgr;
	if ( ! dbMgr.loadDriver(driver, driverId) )
		throw std::runtime_error("Unable to load driver " + driver);
	dnmi::db::Connection * connection = dbMgr.connect(driverId, connectString);
	if ( ! connection or not connection->isConnected() )
		throw std::runtime_error("Unable to connect to database");
	return connection;
}

bool query(dnmi::db::Connection * connection, std::string q, std::function<void(const dnmi::db::DRow &)> handler)
{
	if ( q.empty() )
		return false;

	if ( q.back() != ';' )
		q = q + ';';

	try
	{
		if ( not connection->isConnected() )
			if ( ! connection->tryReconnect() )
				throw std::runtime_error("Database connection unavailable!");

		std::unique_ptr<dnmi::db::Result> result(connection->execQuery(q));
		while ( result->hasNext() )
			handler(result->next());
		return true;
	}
	catch ( std::exception & e )
	{
		LOGERROR(e.what());
		return false;
	}
}
}

SqlKvApp::SqlKvApp(int &argc, char **argv, miutil::conf::ConfSection *conf) :
		corba::CorbaKvApp(argc, argv, conf),
		connection_(createConnection(conf))
{
}

SqlKvApp::~SqlKvApp()
{
}

bool SqlKvApp::getKvData( KvGetDataReceiver &dataReceiver, const WhichDataHelper &wd )
{
	return CorbaKvApp::getKvData(dataReceiver, wd);
}

bool SqlKvApp::getKvRejectDecode( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, kvservice::RejectDecodeIterator &it )
{
	return CorbaKvApp::getKvRejectDecode(decodeInfo, it);
}

bool SqlKvApp::getKvParams( std::list<kvalobs::kvParam> &paramList )
{
	return query(connection_.get(),
			"select * from param",
			[&paramList](const dnmi::db::DRow & row) {
		paramList.push_back(kvalobs::kvParam(row));
	});
}

bool SqlKvApp::getKvStations( std::list<kvalobs::kvStation> &stationList )
{
	return CorbaKvApp::getKvStations(stationList);
}

bool SqlKvApp::getKvModelData( std::list<kvalobs::kvModelData> &dataList, const WhichDataHelper &wd )
{
	return CorbaKvApp::getKvModelData(dataList, wd);
}

bool SqlKvApp::getKvReferenceStations( int stationid, int paramid, std::list<kvalobs::kvReferenceStation> &refList )
{
	return CorbaKvApp::getKvReferenceStations(stationid, paramid, refList);
}

bool SqlKvApp::getKvTypes( std::list<kvalobs::kvTypes> &typeList )
{
	return CorbaKvApp::getKvTypes(typeList);
}

bool SqlKvApp::getKvOperator( std::list<kvalobs::kvOperator> &operatorList )
{
	return CorbaKvApp::getKvOperator(operatorList);
}

bool SqlKvApp::getKvStationParam( std::list<kvalobs::kvStationParam> &stParam, int stationid, int paramid, int day )
{
	return CorbaKvApp::getKvStationParam(stParam, stationid, paramid, day);
}

bool SqlKvApp::getKvStationMetaData( std::list<kvalobs::kvStationMetadata> &stMeta,
		                             int stationid, const boost::posix_time::ptime &obstime,
		                             const std::string & metadataName)
{
	return CorbaKvApp::getKvStationMetaData(stMeta, stationid, obstime, metadataName);
}

bool SqlKvApp::getKvObsPgm( std::list<kvalobs::kvObsPgm> &obsPgm, const std::list<long> &stationList, bool aUnion )
{
	return CorbaKvApp::getKvObsPgm(obsPgm, stationList, aUnion);
}

bool SqlKvApp::getKvData( KvObsDataList &dataList, const WhichDataHelper &wd )
{
	return CorbaKvApp::getKvData(dataList, wd);
}

bool SqlKvApp::getKvWorkstatistik(CKvalObs::CService::WorkstatistikTimeType timeType,
                                const boost::posix_time::ptime &from, const boost::posix_time::ptime &to,
                                kvservice::WorkstatistikIterator &it
                                )
{
	return CorbaKvApp::getKvWorkstatistik(timeType, from, to, it);
}



} /* namespace sql */
} /* namespace kvservice */
