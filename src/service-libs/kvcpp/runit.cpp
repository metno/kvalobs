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
#include <iostream>
#include <stdexcept>
#include <memory>

using kvservice::KvApp;

kvservice::KvApp * getApp(int argc, char ** argv)
{
	using kvservice::sql::SqlKvApp;
	return new SqlKvApp(argc, argv, SqlKvApp::readConf({"runit.conf", "kvalobs.conf", kvPath("sysconfdir") + "/kvalobs.conf"}));
}

void readParam()
{
	std::list<kvalobs::kvParam> paramList;
	if ( ! KvApp::kvApp->getKvParams(paramList) )
		throw std::runtime_error("Uanble to read param");
	for ( auto p : paramList )
		std::cout << p.name() << ":\t" << p.paramID() << std::endl;
}


int main(int argc, char ** argv)
{
	std::unique_ptr<kvservice::KvApp> app(getApp(argc, argv));
	readParam();
}
