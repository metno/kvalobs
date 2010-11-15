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

#include "ConnectionHandler.h"
#include "qabaseApp.h"
#include <milog/milog.h>


ConnectionHandler::ConnectionHandler(QaBaseApp & app) :
	app_(app), con(0), idleTime(0)
{
}

ConnectionHandler::~ConnectionHandler()
{
	LOGDEBUG("Closing the database connection before termination!");
	if (con)
		app_.releaseDbConnection(con);
}

/**
 * \returns a connection to db, either freshly generated or an old one
 */
dnmi::db::Connection * ConnectionHandler::getConnection()
{
	if (!con)
	{
		while (!app_.shutdown())
		{
			con = app_.getNewDbConnection();
			if (con)
			{
				LOGDEBUG("Created a new connection to the database!");
				break;
			}
			LOGINFO(
					"Can't create a connection to the database, retry in 5 seconds ..");
			sleep(5);
		}
	}
	idleTime = 0;
	return con;
}

/**
 * Signal that the db connection is not needed. If this signal is used
 * \c max_idle_time times in a row, the db connection will be released.
 */
void ConnectionHandler::notNeeded()
{
	if (con and ++idleTime >= max_idle_time)
	{
		LOGDEBUG("Closing the database connection!");
		app_.releaseDbConnection(con);
		con = 0;
		idleTime = 0;
	}
}
