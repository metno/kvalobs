/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 Copyright (C) 2010 met.no

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

#include "ProxyDatabaseConnection.h"
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvPath.h>
#include <stdexcept>

namespace
{
// Holds all created connections
dnmi::db::DriverManager driverManager;
}

ProxyDatabaseConnection::ProxyDatabaseConnection(const std::string & databaseProxyFile, bool createDatabase)
{
	const std::string dbDriverPath = kvPath("pkglibdir") + "/db/";

	std::string proxyID;
	if ( ! driverManager.loadDriver(dbDriverPath + "sqlite3driver.so", proxyID) )
		throw std::runtime_error("Error when loading database driver: " + driverManager.getErr() );

	connection_ = driverManager.connect(proxyID, databaseProxyFile);

	if ( ! connection_)
		throw std::runtime_error("Cant create a database connection to " + databaseProxyFile);

	if ( createDatabase )
		createDatabase_();
}

ProxyDatabaseConnection::~ProxyDatabaseConnection()
{
	driverManager.releaseConnection(connection_);
}

void ProxyDatabaseConnection::createDatabase_()
{
	const char * createStatement =
			"CREATE TABLE data ("
			"stationid INTEGER NOT NULL,"
			"obstime TIMESTAMP NOT NULL,"
			"original FLOAT NOT NULL,"
			"paramid INTEGER NOT NULL,"
			"tbtime TIMESTAMP NOT NULL,"
			"typeid INTEGER NOT NULL,"
			"sensor CHAR(1) DEFAULT '0',"
			"level INTEGER DEFAULT 0,"
			"corrected FLOAT NOT NULL,"
			"controlinfo CHAR(16) DEFAULT '0000000000000000',"
			"useinfo CHAR(16) DEFAULT '0000000000000000',"
			"cfailed TEXT DEFAULT NULL,"
			"UNIQUE ( stationid, obstime, paramid, level, sensor, typeid ));";

	connection_->exec(createStatement);
}
