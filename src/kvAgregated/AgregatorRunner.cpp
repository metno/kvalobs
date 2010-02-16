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

#include "AgregatorRunner.h"
#include <kvcpp/kvevents.h>
#include <kvcpp/KvApp.h>
#include <milog/milog.h>
#include <puTools/miTime>
#include <boost/scoped_ptr.hpp>

AgregatorRunner::AgregatorRunner(const std::vector<int> & stations, kvservice::proxy::KvalobsProxy & proxy) :
	shutdown(false), incomingHandler(proxy)
{
    assert( kvservice::KvApp::kvApp );

    LOGINFO( "Subscribing to data from source" );

    kvservice::KvDataSubscribeInfoHelper sih;
    for ( std::vector<int>::const_iterator it = stations.begin(); it != stations.end(); ++ it )
    	sih.addStationId( * it );
    kvservice::KvApp::kvApp->subscribeData( sih, queue );
}

AgregatorRunner::~AgregatorRunner()
{
}

void AgregatorRunner::start()
{
	milog::LogContext context("AgregatorRunner::start");
	shutdown = false;
	try
	{
		run();
	} catch (std::exception & e)
	{
		LOGFATAL( e.what() );
		stop();
	} catch (...)
	{
		LOGFATAL( "Unknown exception!" );
		stop();
	}
	LOGDEBUG( "Stopping..." );
}

namespace
{
struct startit
{
	AgregatorRunner *p;
	startit(AgregatorRunner *p) :
		p(p)
	{
	}
	void operator()()
	{
		p->start();
	}
};
}

void AgregatorRunner::start_thread()
{
	milog::LogContext context("AgregatorRunner::start_thread");

	LOGDEBUG( "Start thread" );
	startit s(this);
	thread = new boost::thread(s);
}

void AgregatorRunner::stop()
{
	shutdown = true;
	incomingHandler.stopThreads();
	if (thread)
	{
		LOGDEBUG( "Stopping AgregatorRunner thread" );
		thread->join();
		delete thread;
		thread = 0;
		LOGDEBUG( "AgregatorRunner thread stopped" );
	}
}

void AgregatorRunner::run()
{
	milog::LogContext context("AgregatorRunner::run (main loop)");
	LOGDEBUG( "Running" );
	while (not shutdown)
	{
		awaitData(1);
//		miutil::miTime now = miutil::miTime::nowTime();
//		if (now.date() > lastCleaned and now.clock() > miClock(2, 15, 0))
//			db_cleanup();
	}
}

void AgregatorRunner::awaitData(int timeout)
{
	boost::scoped_ptr<dnmi::thread::CommandBase> base(queue.get(timeout));
	if (!base.get())
		return;
	kvservice::DataEvent *data =
			dynamic_cast<kvservice::DataEvent *> (base.get());

	if (!data)
	{
		LOGERROR( "Could not understand data received from kvalobs" );
		return;
	}

	assert( ( void* ) base.get() == ( void* ) data );

	data->dispatchEvent(incomingHandler);
}
