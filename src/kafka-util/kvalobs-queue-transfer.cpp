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

#include <csignal>
#include <iostream>
#include <memory>
#include <thread>
#include <kvsubscribe/DataProducer.h>
#include <kvsubscribe/queue.h>
#include <kvsubscribe/NotificationProducer.h>
#include <kvsubscribe/Notification.h>
#include <kvcpp/corba/CorbaKvApp.h>
#include <kvcpp/kvevents.h>
#include <kvalobs/kvPath.h>
#include <decodeutility/kvalobsdata.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <milog/Logger.h>


using namespace kvalobs::subscribe;

bool shutdown = false;
void stopApplication(int signal)
{
	shutdown = true;
	kvservice::KvApp::kvApp->doShutdown();
}

void onError(const std::string & data, const std::string & errorMessage)
{
	std::clog << "ERROR:\t" << errorMessage << std::endl;
}

miutil::conf::ConfSection * getConfSection()
{
	std::string basePath = kvalobs::kvPath(kvalobs::sysconfdir);

	std::string myconf = basePath + "/kvalobs-queue-transfer.conf";
	miutil::conf::ConfSection * confSec = kvservice::corba::CorbaKvApp::readConf(myconf);
	if (!confSec)
	{
		myconf = basePath + "/kvalobs.conf";
		confSec = kvservice::corba::CorbaKvApp::readConf(myconf);
	}
	if (!confSec)
		throw std::runtime_error("Cannot open conf file: " + myconf);
	return confSec;
}


void produceData(const std::string brokers = "localhost")
{
	dnmi::thread::CommandQue queue;
	kvservice::KvDataSubscribeInfoHelper helper;
	auto id = kvservice::KvApp::kvApp->subscribeData(helper, queue);
	if ( id.empty() )
		throw std::runtime_error("Subscription error!");
	DataProducer output(brokers, onError);

	std::clog << "Data queue ready (" << queue::data() << ")" << std::endl;

	long long count = 0;
	while ( not shutdown )
	{
		output.catchup();
		boost::scoped_ptr<dnmi::thread::CommandBase> base(queue.get(1));
		if ( ! base )
			continue;

		kvservice::DataEvent * event = dynamic_cast<kvservice::DataEvent *>(base.get());
		if ( ! event )
		{
			std::clog << "Unable to understand incoming data" << std::endl;
			continue;
		}

		auto dataList = event->data();
		for ( const kvservice::KvObsData & data : * dataList )
		{
			kvalobs::serialize::KvalobsData d(data.dataList(), data.textDataList());
			output.send(d);
		}
	}
	std::clog << "done" << std::endl;
}

void produceNotifications(const std::string brokers = "localhost")
{
	dnmi::thread::CommandQue queue;
	kvservice::KvDataSubscribeInfoHelper helper;
	auto id = kvservice::KvApp::kvApp->subscribeDataNotify(helper, queue);
	if ( id.empty() )
		throw std::runtime_error("Subscription error!");
	NotificationProducer output(brokers, onError);

	std::clog << "Data queue ready (" << queue::notification() << ")" << std::endl;

	long long count = 0;
	while ( not shutdown )
	{
		output.catchup();
		boost::scoped_ptr<dnmi::thread::CommandBase> base(queue.get(1));
		if ( ! base )
			continue;

		kvservice::DataNotifyEvent * event = dynamic_cast<kvservice::DataNotifyEvent *>(base.get());
		if ( ! event )
		{
			std::clog << "Unable to understand incoming notification" << std::endl;
			continue;
		}

		for ( auto what : * event->what() )
		{
			Notification n(what.stationID(), what.typeID(), what.obsTime());
			output.send(n);
		}
	}
	std::clog << "done" << std::endl;
}


int main(int argc, char ** argv)
{
	milog::Logger::logger().logLevel(milog::FATAL);

	std::unique_ptr<miutil::conf::ConfSection> confSec(getConfSection());
	kvservice::corba::CorbaKvApp app(argc, argv, confSec.get());

	signal(SIGINT, stopApplication);
	signal(SIGTERM, stopApplication);

	std::string brokers = "localhost";
	std::thread data([&](){produceData(brokers);});
	//std::thread notify([&](){produceNotifications(brokers);});

	app.run();
	data.join();
	//notify.join();
}
