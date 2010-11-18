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


#ifndef CONNECTIONHANDLER_H_
#define CONNECTIONHANDLER_H_

class QaBaseApp;
namespace dnmi
{
namespace db
{
class Connection;
}
}


/**
 * Handles db connection and dsconnection after timeout.
 *
 * \ingroup group_corba
 */
class ConnectionHandler
{
	QaBaseApp & app_;
	dnmi::db::Connection * con;
	int idleTime;
	static const int max_idle_time = 60;
public:

	ConnectionHandler(QaBaseApp & app);
	~ConnectionHandler();

	/**
	 * \returns a connection to db, either freshly generated or an old one
	 */
	dnmi::db::Connection * getConnection();

	/**
	 * Signal that the db connection is not needed. If this signal is used
	 * \c max_idle_time times in a row, the db connection will be released.
	 */
	void notNeeded();
};


#endif /* CONNECTIONHANDLER_H_ */
