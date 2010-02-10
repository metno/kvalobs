/* Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: main.cc,v 1.4.2.9 2007/09/27 09:02:15 paule Exp $

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
#include <kvcpp/corba/CorbaKvApp.h>
#include "AgregatorHandler.h"
#include "BackProduction.h"
#include "proxy/KvalobsProxy.h"
#include "configuration/AgregatorConfiguration.h"
#include <kvalobs/kvStation.h>
#include <milog/milog.h>
#include <milog/FLogStream.h>
#include <puTools/miClock.h>
#include <kvdb/dbdrivermgr.h>
#include <set>
#include <fileutil/pidfileutil.h>
#include <kvalobs/kvPath.h>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "minmax.h"
#include "rr.h"
#include "ra2rr_12.h"
#include <memory>

using namespace std;
using namespace agregator;
using namespace miutil;
using namespace milog;
using namespace dnmi::db;
namespace opt = boost::program_options;

typedef kvservice::corba::CorbaKvApp KvApp;
// typedef kvservice::KvApp KvApp;

namespace
{
void setupPidFile(dnmi::file::PidFileHelper & pidFile)
{
	//PID-file
	std::string pidFileName;
	pidFileName = dnmi::file::createPidFileName(kvPath("localstatedir") + "/run", "kvAgregated");
	bool pidfileError;
	if (dnmi::file::isRunningPidFile(pidFileName, pidfileError))
	{
		if (pidfileError)
		{
			std::ostringstream msg;
			msg << "An error occured while reading the pidfile " <<
					pidFileName	<< ". Remove the file if it exist and "
					"<agregate> is not running. If it is running and there are "
					"problems. Kill <agregate> and restart it.";
			throw std::runtime_error(msg.str());
		}
		else
		{
			std::ostringstream msg;
			msg << "Is <agregate> allready running? If not remove the pidfile: " << pidFileName;
			throw std::runtime_error(msg.str());;
		}
	}
	pidFile.createPidFile(pidFileName);
}

void runDaemon(kvservice::proxy::KvalobsProxy & proxy)
{
	proxy.start_thread();

	LOGINFO("Starting main loop");
	KvApp::kvApp->run();
}

std::auto_ptr<FLogStream> createLog(const std::string & logFile, milog::LogLevel level, int maxSize)
{
	std::auto_ptr<FLogStream> ret(new FLogStream(9, maxSize));

	ret->open(kvPath("localstatedir") + "/log/" + logFile);
	ret->loglevel(level);
	LogManager::instance()->addStream(ret.get());
	return ret;
}

Connection * getProxyDatabaseConnection(const std::string & databaseFile)
{
	// Proxy database
	DriverManager manager;
	std::string proxyID;
	const string dbDriverPath = kvPath("pkglibdir") + "/db/";
	if (!manager.loadDriver(dbDriverPath + "sqlite3driver.so", proxyID))
		throw std::runtime_error("Error when loading database driver: " + manager.getErr() );

	Connection * dbConn = manager.connect(proxyID, databaseFile);

	if (!dbConn)
		throw std::runtime_error("Cant create a database connection to " + databaseFile);

	return dbConn;
}

miutil::conf::ConfSection * getConfSection()
{
	string myconf = "kvAgregated.conf";
	miutil::conf::ConfSection * confSec = KvApp::readConf(myconf);
	if (!confSec)
	{
		myconf = kvPath("sysconfdir") + "/kvalobs.conf";
		confSec = KvApp::readConf(myconf);
	}
	if (!confSec)
		throw std::runtime_error("Cant open conf file: " + myconf);
	return confSec;
}

void runThreadWithBackProduction(BackProduction & back, kvservice::proxy::KvalobsProxy & proxy)
{
	boost::thread t(back);
	runDaemon(proxy);
	t.join();
}

void runAgregator(const AgregatorConfiguration & conf,
		kvservice::proxy::KvalobsProxy & proxy)
{
	if ( conf.backProduction() )
	{
		BackProduction back(proxy, conf.backProductionSpec());
		if (!conf.daemonMode())
			back(); // run outside a thread
		else
			runThreadWithBackProduction(back, proxy);
	}
	else
	{
		// even if no backproduction was set, we want to generate data for 3
		// hours back in time
		miutil::miTime to(miutil::miDate::today(), miutil::miClock(	miutil::miClock::oclock().hour(), 0, 0));
		miutil::miTime from(to);
		from.addHour(-3);

		BackProduction back(proxy, from, to);
		runThreadWithBackProduction(back, proxy);
	}
}

}

int main(int argc, char **argv)
{
	AgregatorConfiguration conf;
	AgregatorConfiguration::ParseResult result = conf.parse(argc, argv);

	if (result != AgregatorConfiguration::No_Action)
		return result;

	try
	{
		// PidFile
		dnmi::file::PidFileHelper pidFile;
		if (conf.runInDaemonMode())
			setupPidFile(pidFile);

		// Logging
		std::auto_ptr<FLogStream> fine = createLog("kvAgregated.log", INFO, 1024 * 1024);
		std::auto_ptr<FLogStream> error = createLog("kvAgregated.warn.log", INFO, 100 * 1024);

		// KvApp
		boost::scoped_ptr<miutil::conf::ConfSection> confSec(getConfSection());
		KvApp app(argc, argv, confSec.get());

		// Proxy database
		boost::scoped_ptr<Connection> dbConn(getProxyDatabaseConnection(conf.proxyDatabaseName()));
		kvservice::proxy::KvalobsProxy proxy(*dbConn, conf.stations(), false);

		AgregatorHandler handler(proxy);
		handler.setParameterFilter(conf.parameters());
		if (conf.repopulateDatabase())
			proxy.db_repopulate();

		// Standard times
		set<miClock> six;
		six.insert(miClock(6, 0, 0));
		six.insert(miClock(18, 0, 0));

		// Add handlers
		MinMax tan12 = min(TAN, TAN_12, 12, six);
		handler.addHandler(&tan12);
		MinMax tax12 = max(TAX, TAX_12, 12, six);
		handler.addHandler(&tax12);
		MinMax tgn12 = min(TGN, TGN_12, 12, six);
		handler.addHandler(&tgn12);
		boost::scoped_ptr<rr_1> rr1(conf.runInDaemonMode() ? new rr_1 : (rr_1 *) 0);
		if ( rr1 )
			handler.addHandler(rr1.get());
		rr_12 rr12 = rr_12();
		handler.addHandler(&rr12);
		rr_24 rr24 = rr_24();
		handler.addHandler(&rr24);
		ra2rr_12_backward ra2rr_b;
		handler.addHandler(&ra2rr_b);
		ra2rr_12_forward ra2rr_f;
		handler.addHandler(&ra2rr_f);

		// Backproduction instead of daemon mode?
		try
		{
		    runAgregator(conf, proxy);
		}
		catch (std::exception & e)
		{
			LOGFATAL(e.what());
			return 1;
		}
	}
	catch ( std::exception & e )
	{
		LOGFATAL(e.what());
		return 1;
	}
}
