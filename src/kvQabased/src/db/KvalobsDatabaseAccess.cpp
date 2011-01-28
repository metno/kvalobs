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

#include "KvalobsDatabaseAccess.h"
#include "databaseResultFilter.h"
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvPath.h>
#include <milog/milog.h>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <iomanip>



namespace db
{
namespace
{
typedef boost::scoped_ptr<dnmi::db::Result> ResultPtr;
}

KvalobsDatabaseAccess::KvalobsDatabaseAccess(const std::string & databaseConnect) :
		ownsConnection_(true)
{
	static dnmi::db::DriverManager dbMgr;
	static std::string driverId;
	if (driverId.empty())
	{
		std::string driver = kvalobs::kvPath(kvalobs::libdir) + "/kvalobs/db/pgdriver.so";

		if (!dbMgr.loadDriver(driver, driverId))
			throw std::runtime_error("Unable to load driver " + driver);
	}
	connection_ = dbMgr.connect(driverId, databaseConnect);
	if ( ! connection_ )
		throw std::runtime_error("Unable to connect to database");
}

KvalobsDatabaseAccess::KvalobsDatabaseAccess(dnmi::db::Connection * connection, bool takeOwnershipOfConnection) :
		connection_(connection),
		ownsConnection_(takeOwnershipOfConnection)
{}

KvalobsDatabaseAccess::~KvalobsDatabaseAccess()
{
	if ( ownsConnection_ )
		delete connection_;
}

void KvalobsDatabaseAccess::getChecks(CheckList * out,
		const kvalobs::kvStationInfo & si) const
{
	std::ostringstream query;
	query << "SELECT * FROM checks WHERE ";
	query << "(stationid=0 OR stationid=" << si.stationID() << ")";
	query << " AND fromtime<='" << si.obstime() << "'";
	query << " ORDER BY stationid, fromtime;";


	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	std::map<std::string, kvalobs::kvChecks> checks;

	ResultPtr result(connection_->execQuery(query.str()));
	while ( result->hasNext() )
	{
		// "Order by" in query will first insert 0-station values, and then
		// overwrite with specific-value stations likewise for varying
		// fromtime - latest fromtime for station is preferred
		kvalobs::kvChecks check(result->next());
		checks[check.qcx()] = check;
	}
	for ( std::map<std::string, kvalobs::kvChecks>::const_iterator it = checks.begin(); it != checks.end(); ++ it )
		out->push_back(it->second);
}

int KvalobsDatabaseAccess::getQcxFlagPosition(const std::string & qcx) const
{
	std::string mediumQcx(qcx, 0, qcx.find_last_of('-'));

	std::ostringstream query;
	query << "SELECT controlpart FROM qcx_info WHERE medium_qcx='" << mediumQcx << "';";

	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	ResultPtr result(connection_->execQuery(query.str()));

	if ( result->hasNext() )
		return boost::lexical_cast<int>(result->next()[0]);
	else
		throw std::runtime_error(mediumQcx + ": No such check type");
}


void KvalobsDatabaseAccess::getExpectedParameters(ParameterList * out,
		const kvalobs::kvStationInfo & si) const
{
	std::ostringstream query;
	query << "SELECT name FROM param WHERE paramid IN (SELECT paramid FROM obs_pgm WHERE "
		"stationid=" << si.stationID() << " AND "
		"typeid=" << si.typeID() << " AND "
		"fromtime<='" << si.obstime() << "' AND "
		"(totime IS NULL OR totime>'" << si.obstime() << "'));";
// TODO: should this be in query?
//		"(totime IS NULL OR totime<'" << si.obstime() << "') AND "
//		"kl" << std::setfill('0') << std::setw(2) << si.obstime().hour() << "  AND " <<
//		si.obstime().date().shortweekday(miutil::miDate::English) << ");";

	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	ResultPtr result(connection_->execQuery(query.str()));
	while ( result->hasNext() )
	{
		dnmi::db::DRow & row = result->next();
		out->insert(row[0]);
	}
}


kvalobs::kvAlgorithms KvalobsDatabaseAccess::getAlgorithm(
		const std::string & algorithmName) const
{
	std::ostringstream query;
	query << "SELECT * FROM algorithms WHERE ";
	query << "checkname='" << algorithmName << "';";

	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	ResultPtr result(connection_->execQuery(query.str()));

	if ( result->hasNext() )
		return kvalobs::kvAlgorithms(result->next());
	else
		throw std::runtime_error(algorithmName + ": No such algorithm");
}

std::string KvalobsDatabaseAccess::getStationParam(const kvalobs::kvStationInfo & si, const std::string & parameter, const std::string & qcx) const
{
	int dayNumber = si.obstime().dayOfYear();

	std::ostringstream query;
	query << "SELECT metadata FROM station_param WHERE "
			"stationid in (0, " << si.stationID() << ") AND "
			"paramid=(SELECT paramid FROM param WHERE name='" << parameter << "') AND "
			"fromday<=" << dayNumber << " AND " << dayNumber << "<=today AND "
			"qcx='" << qcx << "' AND "
			"level=0 AND sensor='0' AND "
			"fromtime<='" << si.obstime() << "' "
			"ORDER BY stationid DESC, fromtime DESC "
			"LIMIT 1;";

	ResultPtr result(connection_->execQuery(query.str()));

	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	if ( result->hasNext() )
	{
		std::string metadata = result->next()[0];
		return metadata;
	}
	else
		throw std::runtime_error("Unable to find station_param for " + parameter + " for qcx=" + qcx);
}

kvalobs::kvStation KvalobsDatabaseAccess::getStation(int stationid) const
{
	std::ostringstream query;
	query << "SELECT * FROM station WHERE stationid=" << stationid << " ORDER BY fromtime DESC LIMIT 1;";

	ResultPtr result(connection_->execQuery(query.str()));

	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	if ( not result->hasNext() )
	{
		std::ostringstream s;
		s << "Unable to find station information for station id " << stationid;
		throw std::runtime_error(s.str());
	}

	kvalobs::kvStation ret(result->next());
	return ret;
}

void KvalobsDatabaseAccess::getModelData(ModelDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minutesBackInTime ) const
{
	std::ostringstream query;
	query << "SELECT * FROM model_data WHERE "
			"stationid=" << si.stationID() << " AND ";
	if ( minutesBackInTime != 0 )
	{
		miutil::miTime first = si.obstime();
		first.addMin(minutesBackInTime);
		query << "obstime BETWEEN '" << first << "' AND '" << si.obstime() << "' AND ";
	}
	else
		query << "obstime = '" << si.obstime() << "' AND ";
	query << "paramid = (SELECT paramid FROM param WHERE name='" << parameter.baseName() << "')";
	if ( parameter.haveLevel() )
		query << " AND level=" << parameter.level();
	query << " ORDER BY obstime DESC;";

	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	ResultPtr result(connection_->execQuery(query.str()));

	while ( result->hasNext() )
		out->push_back(kvalobs::kvModelData(result->next()));
}

void KvalobsDatabaseAccess::getData(DataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const
{
	std::ostringstream query;
	query << "SELECT * FROM data WHERE ";
	query << "stationid=" << si.stationID() << " AND ";
	query << "paramid IN (SELECT paramid FROM param WHERE name='" << parameter.baseName() << "') AND ";
	if ( parameter.haveLevel() )
		query << "level=" << parameter.level() << " AND ";
	if ( parameter.haveSensor() )
		query << "sensor='" << parameter.sensor() << "' AND ";
	if ( parameter.haveType() )
		query << "typeid=" << parameter.type() << " AND ";
	miutil::miTime t = si.obstime();
	t.addMin(minuteOffset);
	query << "obstime BETWEEN '" << t << "' AND '" << si.obstime() << "'";
	query << " ORDER BY obstime DESC;";

	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	ResultPtr result(connection_->execQuery(query.str()));

	db::DatabaseAccess::DataList data;
	while ( result->hasNext() )
		data.push_back(kvalobs::kvData(result->next()));

	db::resultfilter::filter(data, si.typeID());

	out->swap(data);
}

void KvalobsDatabaseAccess::getTextData(TextDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const
{
	std::ostringstream query;
	query << "SELECT * FROM text_data WHERE ";
	query << "stationid=" << si.stationID() << " AND ";
	query << "paramid IN (SELECT paramid FROM param WHERE name='" << parameter.baseName() << "') AND ";
	if ( parameter.haveType() )
		query << "typeid=" << parameter.type() << " AND ";
	miutil::miTime t = si.obstime();
	t.addMin(minuteOffset);
	query << "obstime BETWEEN '" << t << "' AND '" << si.obstime() << "'";
	query << " ORDER BY obstime DESC;";

	milog::LogContext context("query");
	LOGDEBUG1(query.str());

	ResultPtr result(connection_->execQuery(query.str()));

	db::DatabaseAccess::TextDataList data;
	while ( result->hasNext() )
		data.push_back(kvalobs::kvTextData(result->next()));

	db::resultfilter::filter(data, si.typeID());

	out->swap(data);
}


void KvalobsDatabaseAccess::write(const DataList & data)
{
#ifdef QABASE_NO_SAVE
	LOGINFO("Pretending to save " << data.size() << " elements to database");
#else
	milog::LogContext context("query");
	LOGINFO("Saving " << data.size() << " elements to database");
//	for ( DataList::const_iterator it = data.begin(); it != data.end(); ++ it )
//		LOGINFO("Write to database: " << * it);


	for (int i = 0; i < 3; ++i)
	{
		try
		{
			connection_->beginTransaction();

			for ( DataList::const_iterator it = data.begin(); it != data.end(); ++ it )
			{
				std::ostringstream query;
				query << "UPDATE data SET "
						"controlinfo='" << it->controlinfo().flagstring() << "', "
						"useinfo='" << it->useinfo().flagstring() << "', "
						"corrected=" << it->corrected() << ", "
						"cfailed='" << it->cfailed() << "'" <<
						it->uniqueKey() << ';';

				LOGDEBUG1(query.str());
				connection_->exec(query.str());
			}

			connection_->endTransaction();
			return;
		}
		catch (dnmi::db::SQLException & e)
		{
			LOGWARN(e.what());
			connection_->rollBack();
//			sleep(1);
		}
	}
	LOGERROR("Unable to save data!");
#endif
}

}
