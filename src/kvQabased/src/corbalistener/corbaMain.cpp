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

#include "corbaMain.h"
#include <Configuration.h>
#include <boost/thread/thread.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include "qabaseApp.h"
#include "qabaseInputImpl.h"
#include "QaWorkThread.h"
#include <milog/milog.h>
#include <miconfparser/miconfparser.h>
#include <fileutil/pidfileutil.h>
#include "AdminImpl.h"
#include <kvalobs/kvPath.h>

//For test
const char* options[][2] =
{
{ "InitRef", "NameService=corbaname::monsoon.oslo.dnmi.no" },
{ 0, 0 } };

using namespace std;
using namespace boost;

namespace
{
std::string getDbDriver(miutil::conf::ConfSection * conf)
{
	if ( conf )
	{
		miutil::conf::ValElementList val = conf->getValue("database.dbdriver");

		if (val.size() == 1)
			return val[0].valAsString();
	}
	//Use postgresql as a last guess.
	return "pgdriver.so";
}

bool checkPidFile()
{
	filesystem::path rundir(kvPath("rundir"));
	if (!boost::filesystem::exists(rundir))
	{
		try
		{
			filesystem::create_directories(rundir);
		} catch (filesystem::filesystem_error & e)
		{
			LOGFATAL( e.what() );
			return false;
		}
	}
	else if (!filesystem::is_directory(rundir))
	{
		LOGFATAL( rundir.native_file_string() << "exists but is not a directory" );
		return false;
	}
	filesystem::path pidfile(dnmi::file::createPidFileName(
			rundir.native_file_string(), "kvQabased"));

	bool error;
	if (dnmi::file::isRunningPidFile(pidfile.native_file_string(), error))
	{
		if (error)
		{
			LOGFATAL( "An error occured while reading the pidfile:" << endl
					<< pidfile.native_file_string() << " remove the file if it exist and"
					<< endl << "kvQabased is not running. " <<
					"If it is running and there is problems. Kill kvQabased and"
					<< endl << "restart it." << endl << endl );
			return false;
		}
		else
		{
			LOGFATAL( "Is kvQabased allready running?" << endl
					<< "If not remove the pidfile: " << pidfile.native_file_string() );
			return false;
		}
	}
	return true;
}
}

int corbaMain(int argc, char** argv, const qabase::Configuration & config)
{
   string logdir( kvPath("logdir") );
	miutil::conf::ConfSection *conf = KvApp::getConfiguration();
	string constr(KvApp::createConnectString());

	milog::createGlobalLogger( logdir, "kvQabased_transaction", "failed", milog::DEBUG );
	milog::createGlobalLogger( logdir, "kvQabased", "transaction", milog::DEBUG,
	                           1024*10, 2,  new milog::StdLayout1() );


	string dbdriver = getDbDriver(conf);

	LOGINFO( "KvQabased: starting ...." );

	if ( ! checkPidFile() )
		return 1;

	QaBaseApp app(argc, argv, dbdriver, constr, options);

	if (!app.isOk())
	{
		LOGFATAL( "FATAL: can't  initialize " << argv[ 0 ] << "!\n" );
		return 1;
	}

	CORBA::ORB_ptr orb = app.getOrb();
	PortableServer::POA_ptr poa = app.getPoa();

	QaWork qaWork(app, config);
	thread qaWorkThread(qaWork);

	try
	{

		QaBaseInputImpl* qabaseImpl = new QaBaseInputImpl(app);
		AdminImpl *admImpl = new AdminImpl(app);

		PortableServer::ObjectId_var mgrImplIid = poa->activate_object(
				qabaseImpl);
		PortableServer::ObjectId_var admImplIid = poa->activate_object(admImpl);

		{
			// IDL interface: CKvalObs::CQabase::QabaseInput
			CORBA::Object_var ref = qabaseImpl->_this();

			if (!app.putRefInNS(ref, "kvQabaseInput"))
			{
				LOGFATAL( "FATAL: can't register with CORBA nameserver!\n" );
				return 1;
			}

			CORBA::String_var sior(orb->object_to_string(ref));
			cout << "IDL object kvQabaseInput IOR = '" << (char*) sior << "'"
					<< endl;

			// IDL interface: micutil::Admin
			ref = admImpl->_this();

			if (!app.putRefInNS(ref, "kvQabaseAdmin"))
			{
				LOGFATAL( "FATAL: can't register with CORBA nameserver!\n" );
				return 1;
			}

			sior = orb->object_to_string(ref);
			cout << "IDL object micutil::Admin IOR = '" << (char*) sior << "'"
					<< endl;

		}

		// Obtain a POAManager, and tell the POA to start accepting
		// requests on its objects.
		PortableServer::POAManager_var pman = app.getPoaMgr();
		pman->activate();

		app.createPidFile("kvQabased");

		orb->run();
		orb->destroy();
	} catch (CORBA::SystemException&)
	{
		LOGFATAL( "Caught CORBA::SystemException." );
		app.deletePidFile();
		exit(1);
	} catch (CORBA::Exception&)
	{
		LOGFATAL( "Caught CORBA::Exception." );
		app.deletePidFile();
		exit(1);
	} catch (omniORB::fatalException & fe)
	{
		LOGFATAL( "Caught omniORB::fatalException:" << endl
				<< "  file: " << fe.file() << endl
				<< "  line: " << fe.line() << endl
				<< "  mesg: " << fe.errmsg() );
		app.deletePidFile();
		exit(1);
	} catch (...)
	{
		LOGFATAL( "Caught unknown exception." );
		app.deletePidFile();
		exit(1);
	}

	CERR("kvQabased: exit ....\n");
	app.deletePidFile();
	return 0;
}

