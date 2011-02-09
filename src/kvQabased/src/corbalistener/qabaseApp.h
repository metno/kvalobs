/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: qabaseApp.h,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $

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
#ifndef __qabaseApp_h__
#define __qabaseApp_h__

#include <kvalobs/kvapp.h>
#include <kvskel/managerInput.hh>
#include <kvalobs/kvStationInfo.h>
#include <dnmithread/CommandQue.h>
#include <kvdb/dbdrivermgr.h>

/**
 * Application type
 *
 * \ingroup group_corba
 */
class QaBaseApp: public KvApp
{
	QaBaseApp(); //No implementation

	dnmi::thread::CommandQue inQue;
	dnmi::db::DriverManager dbMgr;
	std::string dbConnect;
	std::string dbDriver;
	std::string dbDriverId;
	CKvalObs::CManager::CheckedInput_var refManager;
	bool shutdown_;
	bool orbIsDown;

	CKvalObs::CManager::CheckedInput_ptr lookUpManager(bool forceNS,
			bool &usedNS);

	boost::mutex mutex;

public:
	QaBaseApp(int argn, char **argv, const std::string &driver_,
			const std::string &connect_, const char *options[][2] = 0);
	virtual ~QaBaseApp();

	//inherited from KvApp
	virtual bool isOk() const;

	/*
	 * force a shutdown
	 */
	void doShutdown()
	{
		shutdown_ = true;
	}
	/**
	 * shutdown returns true when the application is in
	 * the terminating state. The command que 'inQue'
	 * is allowed to be processed before we shutdown.
	 */
	bool shutdown();

	/**
	* The program is in shutdown sequence. Just waiting for
	* the command que 'inQue' to be processed before shutdown.
	*/
	bool pendingShutdown();
	/**
	 * Creates a new connection to the database. The caller must
	 * call releaseDbConnection after use.
	 */
	dnmi::db::Connection *getNewDbConnection();
	void releaseDbConnection(dnmi::db::Connection *con);

	dnmi::thread::CommandQue& getInQue()
	{
		return inQue;
	}

	/**
	 * Used to return the result from the checks back to the kvManagerd.
	 */
	bool sendToManager(kvalobs::kvStationInfoList &retList,
			CKvalObs::CManager::CheckedInput_ptr callback);
};

#endif
