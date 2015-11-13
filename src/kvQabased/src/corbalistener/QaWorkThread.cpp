/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: QaWorkThread.cc,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $

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

#include "QaWorkThread.h"
#include "ConnectionHandler.h"
#include "qabaseApp.h"
#include "QaWorkCommand.h"
#include <Configuration.h>
#include <CheckRunner.h>
#include <db/KvalobsDatabaseAccess.h>
#include <kvsubscribe/KafkaProducer.h>
#include <kvsubscribe/Notification.h>
#include <kvsubscribe/NotificationSubscriber.h>
#include <kvsubscribe/DataSubscriber.h>
#include <decodeutility/kvalobsdata.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <sstream>
#include <kvalobs/kvDbGate.h>
#include <miutil/timeconvert.h>
#include <boost/regex.hpp>
#include <memory>
#include <stdexcept>

using namespace kvalobs;
using namespace std;
using namespace miutil;

namespace
{
void processNotficationErrors(const std::string & data,
        const std::string & errorMessage)
{
    LOGERROR("Unable to send notification: " + errorMessage + "  Data: <" + data + ">");
}
void processDataErrors(const std::string & data,
        const std::string & errorMessage)
{
    LOGERROR("Unable to send data: " + errorMessage + "  Data: <" + data + ">");
}
}



QaWork::QaWork(QaBaseApp & app, const qabase::Configuration & config) :
	app(app),
	notifier_(new subscribe::KafkaProducer(
				subscribe::NotificationSubscriber::topic(),
				processNotficationErrors)),
	dataSender_(new subscribe::KafkaProducer(subscribe::DataSubscriber::topic(),
				processDataErrors)),
	logCreator_(config.baseLogDir()),
	logLevel_(config.logLevel())

{
}

namespace
{
void doUpdateWorkQue(kvDbGate & gate, const kvStationInfo & si,
		const string & colName)
{
	LOGDEBUG( "UPDATE: workque!" );
	ostringstream ost;
	ost << "UPDATE workque SET " << colName << "='"
			<< to_kvalobs_string(boost::posix_time::second_clock::universal_time()) << "' WHERE stationid="
			<< si.stationID() << "  AND obstime='" << to_kvalobs_string(si.obstime())
			<< "' AND typeid=" << si.typeID();

	if (!gate.exec(ost.str()))
		LOGERROR( "QaWorkThread: (" << colName << ") Cant update table workque." <<
				"-- Stationid: " << si.stationID() << endl <<
				"--   obstime: " << si.obstime() << endl <<
				"--    typeid: " << si.typeID() <<
				"-- query: " << ost.str() << endl <<
				"-- reason: " << gate.getErrorStr() );
}

bool updateWorkQue_(const QaWorkCommand & work)
{
	string work_update_workque;
	if (work.getKey("update_workque", work_update_workque))
		if (work_update_workque == "false")
			return false;
	return true;
}
}

void QaWork::operator()()
{
	milog::Logger::logger().logLevel(logLevel_);

	LOGDEBUG( "QaWork: starting work thread!\n" );

	ConnectionHandler connectionHandler(app);

	while (!app.shutdown())
	{
		try
		{
			auto_ptr<const dnmi::thread::CommandBase>
					cmd(app.getInQue().get(1));
			if (!cmd.get())
			{
				connectionHandler.notNeeded();
				continue;
			}
			if (app.shutdown())
				continue;

			LOGDEBUG( "QaWork: command received....\n" );

			const QaWorkCommand * work =
					dynamic_cast<const QaWorkCommand*> (cmd.get());

			// The list will have one and only one element when it is received from kvManager.
			if (work and not work->getStationInfo().empty())
			{
				dnmi::db::Connection * con = connectionHandler.getConnection();
				if (!con)
				{
					LOGERROR( "Could not get connection to database" );
					continue;
				}
				if (not app.shutdown())
					process(*con, *work);
			}
				else
				LOGERROR( "QaWork: Unexpected command ....\n" );
		} catch (std::bad_alloc &)
		{
			LOGFATAL("Memory allocation error!");
			app.doShutdown();
			break;
		} catch (std::exception & e)
		{
			LOGERROR(std::string("Error when processing data set: ") + e.what())
		}
	}
	LOGDEBUG( "QaWork: Thread terminating!" );
}

void QaWork::process(dnmi::db::Connection & con, const QaWorkCommand & work)
{
	kvDbGate gate(&con);
	const kvStationInfo & si = work.getStationInfo().front();

	if (updateWorkQue_(work))
		doUpdateWorkQue(gate, si, "qa_start");
		else
		LOGDEBUG( "NO UPDATE: workque!" );

	kvalobs::kvStationInfoList retList;
	doWork(si, retList, con);

	if (!retList.empty())
	{
		if (updateWorkQue_(work))
			doUpdateWorkQue(gate, si, "qa_stop");

		//Return the result to kvManager.
		CKvalObs::CManager::CheckedInput_var callback = work.getCallback();
		app.sendToManager(retList, callback);
	}

	notifySubscribers_(retList);
}

/**
 * The retList must contain the result that is to be returned to
 * the kvManager. The result may contain more parameters and there
 * may be results for additional stations. But the station that came
 * in and is to be processed must be at the head of the retList. Other
 * stations that is touched in the processing must be pushed at the tail.
 */
void QaWork::doWork(const kvalobs::kvStationInfo & params,
		kvalobs::kvStationInfoList & retList, dnmi::db::Connection & con)
{
	retList.push_back(params);

	LOGDEBUG( "QaWork::doWork at:" << boost::posix_time::microsec_clock::universal_time() << "  Processing " << params );

	db::KvalobsDatabaseAccess db(& con, false);
	qabase::CheckRunner checkRunner(db);

	qabase::LogFileCreator::LogStreamPtr log = logCreator_.getLogStream(params);
	checkRunner.newObservation(params, log.get());
}

void QaWork::notifySubscribers_(const kvalobs::kvStationInfoList & changeList)
{
    sendNotifications_(changeList);
    sendData_(changeList);
}

void QaWork::sendNotifications_(const kvalobs::kvStationInfoList & changeList)
{
    for (const kvalobs::kvStationInfo & info : changeList)
    {
        kvalobs::subscribe::Notification n(info.stationID(), info.typeID(),
                info.obstime());
        notifier_->send(n.str());
    }
    notifier_->catchup();
}

class AutoReleaseConnection
{
public:
    AutoReleaseConnection(QaBaseApp & app) :
            app_(app)
    {
        connection_ = app_.getNewDbConnection();
    }

    ~AutoReleaseConnection()
    {
        app_.releaseDbConnection(connection_);
    }

    dnmi::db::Connection & connection()
    {
        return *connection_;
    }

private:
    QaBaseApp & app_;
    dnmi::db::Connection * connection_;
};

void QaWork::sendData_(const kvalobs::kvStationInfoList & changeList)
{
    AutoReleaseConnection arc(app);
    dnmi::db::Connection & connection = arc.connection();

    for (const kvStationInfo & info : changeList)
    {
        kvalobs::serialize::KvalobsData d;

        std::ostringstream q;
        q << "SELECT * FROM data WHERE stationid=" << info.stationID()
                << " AND typeid=" << info.typeID() << " AND obstime='"
                << to_kvalobs_string(info.obstime()) << "';";
        std::unique_ptr<dnmi::db::Result> r(connection.execQuery(q.str()));
        while (r->hasNext())
            d.insert(kvalobs::kvData(r->next()));

        q.clear();
        q << "SELECT * FROM text_data WHERE stationid=" << info.stationID()
                << " AND typeid=" << info.typeID() << " AND obstime='"
                << to_kvalobs_string(info.obstime()) << "';";
        r.reset(connection.execQuery(q.str()));
        while (r->hasNext())
            d.insert(kvalobs::kvTextData(r->next()));

        std::string data = kvalobs::serialize::KvalobsDataSerializer::serialize(
                d);
        dataSender_->send(data);
    }

    dataSender_->catchup();
}
