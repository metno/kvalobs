/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvQueries.cc,v 1.30.2.10 2007/09/27 09:02:30 paule Exp $

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
#include <kvalobs/kvQueries.h>
#include <miutil/timeconvert.h>

using namespace std;

/*
 Select all rows from table checks matching
 stationid in slist, language=lan and valid fromtime
 - Sort by qcx, stationid
 - Valid fromtime is found by using a 'correlated subquery'
 */
std::string kvQueries::selectChecks(const std::list<int> slist, const int lan,
		const boost::posix_time::ptime& otime)
{
	ostringstream ost;

	ost << " C1 WHERE C1.stationid IN (";

	std::list<int>::const_iterator sp = slist.begin();
	for (; sp != slist.end(); sp++)
		ost << (sp == slist.begin() ? "" : ",") << *sp;

	ost << ") AND C1.language=" << lan << "  AND  C1.fromtime=("
			<< "         SELECT MAX(C2.fromtime) FROM checks C2 WHERE"
			<< "         C2.fromtime<=\'" << to_kvalobs_string(otime) << "\'"
			<< "         AND  C2.stationid =C1.stationid"
			<< "         AND  C2.qcx =C1.qcx "
			<< "         AND  C2.language =C1.language "
			<< " ) ORDER BY C1.qcx,C1.stationid";

	return ost.str();
}

namespace
{
/**
 * Get the number of days since start of year (in range 1..366)
 */
int dayOfYear(const boost::posix_time::ptime & t)
{
	const boost::gregorian::date & d = t.date();
	const boost::gregorian::date startOfYear(d.year(), 1, 1);
	return (d - startOfYear).days() + 1;
}
}

/*
 Select all rows from table station_param matching
 stationid in slist, otime in [fromday - today],
 qcx = qcx and valid fromtime
 - Sort by descending stationid
 - Valid fromtime is found by using a 'correlated subquery'
 */
std::string kvQueries::selectStationParam(const std::list<int> slist,
		const boost::posix_time::ptime& otime, const string& qcx)
{
	ostringstream ost;

	ost << " SP1 WHERE SP1.stationid IN (";

	std::list<int>::const_iterator sp = slist.begin();
	for (; sp != slist.end(); sp++)
		ost << (sp == slist.begin() ? "" : ",") << *sp;

	ost << ") AND SP1.qcx=\'" << qcx << "\'" << " AND SP1.fromday<="
			<< dayOfYear(otime) << " AND SP1.today>=" << dayOfYear(otime)
			<< " AND SP1.fromtime=("
			<< "         SELECT MAX(SP2.fromtime) FROM station_param SP2 WHERE"
			<< "         SP2.fromtime<=\'" << to_kvalobs_string(otime) << "\'"
			<< "         AND  SP2.stationid =SP1.stationid"
			<< "         AND  SP2.paramid =SP1.paramid"
			<< "         AND  SP2.level =SP1.level"
			<< "         AND  SP2.sensor =SP1.sensor"
			<< "         AND  SP2.fromday =SP1.fromday"
			<< "         AND  SP2.today =SP1.today"
			<< "         AND  SP2.qcx =SP1.qcx "
			<< " ) ORDER BY SP1.stationid DESC";

	return ost.str();
}

std::string kvQueries::selectData(const boost::posix_time::ptime& otime)
{
	ostringstream ost;

	ost << " where obstime=\'" << to_kvalobs_string(otime) << "\'";

	return ost.str();
}

std::string kvQueries::selectData(const int sid, const int pid,
		const boost::posix_time::ptime& otime)
{
	ostringstream ost;

	ost << " where stationid=" << sid << " and paramid=" << pid
			<< " and obstime=\'" << to_kvalobs_string(otime) << "\'";

	return ost.str();
}

std::string kvQueries::selectDataFromType(const int sid, const int tid,
		const boost::posix_time::ptime& otime)
{
	ostringstream ost;

	ost << " where stationid=" << sid << " and typeid=" << tid
			<< " and obstime=\'" << to_kvalobs_string(otime)
			<< "\' ORDER BY paramid, level, sensor";

	return ost.str();
}

std::string kvQueries::selectTextDataFromType(const int sid, const int tid,
		const boost::posix_time::ptime& otime)
{
	ostringstream ost;

	ost << " where stationid=" << sid << " and typeid=" << tid
			<< " and obstime=\'" << to_kvalobs_string(otime) << "\' ORDER BY paramid";

	return ost.str();
}

std::string kvQueries::selectDataFromAbsType(const int sid, const int tid,
		const boost::posix_time::ptime& otime)
{
	ostringstream ost;

	ost << " where stationid=" << sid << " and abs(typeid)=" << tid
			<< " and obstime=\'" << to_kvalobs_string(otime) << "\'";

	return ost.str();
}

std::string kvQueries::selectData(const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime,
		const std::string& ob)
{
	ostringstream ost;

	ost << " where obstime>=\'" << to_kvalobs_string(stime) << "\'" << " and obstime<=\'"
			<< to_kvalobs_string(etime) << "\'" << " order by " << ob;

	return ost.str();
}

std::string kvQueries::selectDataByTabletime(const boost::posix_time::ptime& stime,
		const boost::posix_time::ptime& etime, const std::string& ob)
{
	ostringstream ost;

	ost << " where tbtime>=\'" << to_kvalobs_string(stime) << "\'" << " and tbtime<=\'"
			<< to_kvalobs_string(etime) << "\'" << " order by " << ob;

	return ost.str();
}

//Bxrge Moe
//16 oct 2002
//Added 'order by' clause to the select. The data will now
//be returned with oldest data first and newest data last. It
//will also guarantee that all data with a given obstime will be kept 
//together.

std::string kvQueries::selectData(const int sid, const boost::posix_time::ptime& stime,
		const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " where stationid=" << sid << " and obstime>=\'" << to_kvalobs_string(stime)
			<< "\'" << " and obstime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by obstime, typeid DESC";

	return ost.str();
}

std::string kvQueries::selectTextData(const int sid, const boost::posix_time::ptime& stime,
		const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " where stationid=" << sid << " and obstime>=\'" << to_kvalobs_string(stime)
			<< "\'" << " and obstime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by obstime,typeid DESC";

	return ost.str();
}

std::string kvQueries::selectDataByTbtime(const int sid,
		const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " where stationid=" << sid << " and tbtime>=\'" << to_kvalobs_string(stime)
			<< "\'" << " and tbtime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by tbtime,typeid";

	return ost.str();
}

//Knut Johansen
//22 jan 2003
//Added new 'order by' clause to the select. The data will now
//be returned with oldest data first and newest data last and ordered
// by station number. It will also guarantee that all data with 
//a given obstime will be kept together.

std::string kvQueries::selectData(const std::string& ob)
{
	ostringstream ost;

	ost << " order by " << ob;
	return ost.str();
}
//Knut Johansen
//2 sep 2003
//Select data from only a few stations
std::string kvQueries::selectDataStat(const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime,
		const std::string& statList)
{
	ostringstream ost;

	ost << " where obstime>=\'" << to_kvalobs_string(stime) << "\'" << " and obstime<=\'"
			<< to_kvalobs_string(etime) << "\'" << " and stationid in (" << statList
			<< ") order by stationid,obstime";

	return ost.str();
}

std::string kvQueries::selectData(const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " where obstime>=\'" << to_kvalobs_string(stime) << "\'" << " and obstime<=\'"
			<< to_kvalobs_string(etime) << "\'" << " order by stationid,obstime";

	return ost.str();
}

std::string kvQueries::selectData(const kvalobs::kvData &d)
{
	ostringstream ost;

	ost << " where" << " stationid=" << d.stationID() << " and "
			<< " obstime=\'" << d.obstime() << "\' and "
			<< " paramid=" << d.paramID() << " and " << " level=" << d.level()
			<< " and " << " sensor=\'" << d.sensor() << "\' and" << " typeid="
			<< d.typeID();

	return ost.str();
}

std::string kvQueries::selectParam(const std::string& ob)
{
	ostringstream ost;

	ost << " order by " << ob;
	return ost.str();
}

std::string kvQueries::selectModelData(const int sid, const boost::posix_time::ptime& stime,
		const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " where stationid=" << sid << " and obstime>=\'" << to_kvalobs_string(stime)
			<< "\'" << " and obstime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by obstime";

	return ost.str();
}

std::string kvQueries::selectReferenceStation(long stationid, long paramsetid)
{
	ostringstream ost;

	ost << " where stationid=" << stationid;

	if (paramsetid >= 0)
	{
		ost << " and paramsetid=" << paramsetid;
	}

	return ost.str();

}

std::string kvQueries::selectStationByStationId(long stationid)
{
	ostringstream ost;

	ost << " where stationid=" << stationid;

	return ost.str();
}

std::string kvQueries::selectStationByWmonr(long wmonr)
{
	ostringstream ost;

	ost << " where wmonr=" << wmonr;

	return ost.str();
}

std::string kvQueries::selectStationByNationalnr(long nationalnr)
{
	ostringstream ost;

	ost << " where nationalnr=" << nationalnr;

	return ost.str();
}

std::string kvQueries::selectStationByIcaoId(long icaoid)
{
	ostringstream ost;

	ost << " where icaoid=" << icaoid;

	return ost.str();
}

std::string kvQueries::selectStationByCall_sign(const std::string &cs)
{
	ostringstream ost;

	ost << " where call_sign=\'" << cs << "\'";

	return ost.str();
}

//milib/kvalobs/src/kvQueries.ccutil::
std::string kvQueries::selectStationOrdered()
{
	ostringstream ost;

	ost << " order by stationid";

	return ost.str();
}

std::string kvQueries::selectAllStations(const std::string &orderby)
{
	ostringstream ost;

	ost << " order by " << orderby;

	return ost.str();
}

std::string kvQueries::selectStationsByRange(long from, long to, bool order)
{
	ostringstream ost;

	if (from > to)
	{
		long tmp = from;
		from = to;
		to = tmp;
	}
	ost << " WHERE stationid >= " << from << " AND stationid < " << to;

	if (order)
		ost << " order by stationid";

	return ost.str();
}

/*
 Select all entries from obs_pgm matching stationid and
 valid fromtime
 - Sort by paramid
 - Valid fromtime is found by using a 'correlated subquery'
 */
std::string kvQueries::selectObsPgm(long stationid, const boost::posix_time::ptime& otime)
{
	ostringstream ost;
	string obst("\'" + to_kvalobs_string(otime) + "\'");

	ost << " WHERE stationid=" << stationid << " AND " << "       (( fromtime<="
			<< obst << " AND totime>" << obst << ") OR "
			<< "        ( fromtime<=" << obst << " AND totime IS NULL ) "
			<< "       ) ORDER BY paramid";
	/*
	 ost << " OP1 WHERE OP1.stationid=" << stationid
	 << " AND OP1.fromtime=("
	 << "         SELECT MAX(OP2.fromtime) FROM obs_pgm OP2 WHERE"
	 << "         OP2.fromtime<=\'" << to_kvalobs_string(otime) << "\'"
	 << "         AND OP2.stationid = OP1.stationid"
	 << "         AND OP2.paramid  = OP1.paramid"
	 << " ) ORDER BY OP1.paramid";
	 */
	return ost.str();
}

/*
 Select all entries from obs_pgm matching stationid, typeid and
 valid fromtime and totime
 - Sort by paramid
 - Valid fromtime is found by using a 'correlated subquery'
 */
std::string kvQueries::selectObsPgm(long stationid, long tid,
		const boost::posix_time::ptime& otime)
{
	ostringstream ost;
	string obst("\'" + to_kvalobs_string(otime) + "\'");

	ost << " WHERE stationid=" << stationid << " AND typeid=" << tid << " AND "
			<< "       (( fromtime<=" << obst << " AND totime>" << obst
			<< ") OR " << "        ( fromtime<=" << obst
			<< " AND totime IS NULL ) " << "       ) ORDER BY paramid";

	/*  ost << " OP1 WHERE OP1.typeid=" << tid << " AND OP1.stationid=" << stationid
	 << " AND OP1.fromtime=("
	 << "         SELECT MAX(OP2.fromtime) FROM obs_pgm OP2 WHERE"
	 << "         OP2.fromtime<=\'" << to_kvalobs_string(otime) << "\'"
	 << "         AND OP2.stationid = OP1.stationid"
	 << "         AND OP2.paramid  = OP1.paramid"
	 << "         AND OP2.typeid  = OP1.typeid"
	 << " ) ORDER BY OP1.paramid";
	 */
	return ost.str();
}

/*
 Select all entries from obs_pgm matching typeid and
 valid fromtime and totime.
 - Sort by stationid.
 - Valid fromtime is found by using a 'correlated subquery'
 */
std::string kvQueries::selectObsPgmByTypeid(long tid,
		const boost::posix_time::ptime& otime)
{
	ostringstream ost;
	string obst("\'" + to_kvalobs_string(otime) + "\'");

	ost << " WHERE typeid=" << tid << " AND " << "       (( fromtime<=" << obst
			<< " AND totime>" << obst << ") OR " << "        ( fromtime<="
			<< obst << " AND totime IS NULL ) "
			<< "       ) ORDER BY stationid, typeid";
	/*
	 ost << " OP1 WHERE OP1.typeid=" << tid
	 << " AND OP1.fromtime=("
	 << "         SELECT MAX(OP2.fromtime) FROM obs_pgm OP2 WHERE"
	 << "             OP2.fromtime<=\'" << to_kvalobs_string(otime) << "\' AND"
	 << "             OP2.stationid  = OP1.stationid AND"
	 << "             OP2.typeid     = OP1.typeid    AND"
	 << "             OP2.paramid    = OP1.paramid   AND"
	 << "             OP2.level      = OP1.level"
	 << " ) ORDER BY OP1.stationid, OP1.typeid";
	 */
	return ost.str();
}

std::string kvQueries::selectObsPgm(long stationid)
{
	ostringstream ss;
	ss << "WHERE stationid=" << stationid
			<< " ORDER BY stationid, typeid, paramid";
	return ss.str();
}

/*
 Select all entries from obs_pgm with valid fromtime and totime
 - Sort by stationid, typeid and paramid
 - Valid fromtime is found by using a 'correlated subquery'
 */
std::string kvQueries::selectObsPgm(const boost::posix_time::ptime& otime)
{
	ostringstream ost;
	string obst("\'" + to_kvalobs_string(otime) + "\'");

	ost << " WHERE ( fromtime<=" << obst << " AND totime>" << obst << ") OR "
			<< "       ( fromtime<=" << obst << " AND totime IS NULL ) "
			<< "        ORDER BY stationid, typeid, paramid";

	/*	ost << " OP1 WHERE OP1.fromtime=("
	 << "         SELECT MAX(OP2.fromtime) FROM obs_pgm OP2 WHERE"
	 << "         OP2.fromtime<=\'" << to_kvalobs_string(otime) << "\'"
	 << "         AND OP2.stationid = OP1.stationid"
	 << "         AND OP2.paramid  = OP1.paramid"
	 << " ) ORDER BY OP1.stationid,OP1.typeid,OP1.paramid";
	 */
	return ost.str();
}

std::string kvQueries::selectKeyValues(const std::string& package,
		const std::string& key)
{
	ostringstream ost;

	ost << " WHERE package=\'" << package << "\' AND key=\'" << key << "\'";

	return ost.str();
}

std::string kvQueries::selectIsGenerated(long stationid, int typeid_)
{
	ostringstream ost;

	ost << " WHERE stationid=" << stationid << " AND typeid=" << typeid_;

	return ost.str();
}

/** EGLITIS (for Qc2)
 * \brief select all rows from table \em data matching
 *  stationid in list, paramid=pid and obstime in [stime - etime]
 */
std::string kvQueries::selectData(const std::list<int> slist, const int pid,
		const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " WHERE stationid IN (";

	std::list<int>::const_iterator sp = slist.begin();
	for (; sp != slist.end(); sp++)
		ost << (sp == slist.begin() ? "" : ",") << *sp;

	ost << ") and paramid=" << pid << " and obstime>=\'" << to_kvalobs_string(stime)
			<< "\'" << " and obstime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by obstime";

	return ost.str();
}

/** EGLITIS (for Qc2)
 * \brief select all rows from table \em data matching
 *  stationid in list, paramid=pid and obstime in [stime - etime]
 */
std::string kvQueries::selectData(const std::list<int> slist, const int pid,
		const int tid, const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " WHERE stationid IN (";

	std::list<int>::const_iterator sp = slist.begin();
	for (; sp != slist.end(); sp++)
		ost << (sp == slist.begin() ? "" : ",") << *sp;

	ost << ") and paramid=" << pid << " and typeid=" << tid
			<< " and obstime>=\'" << to_kvalobs_string(stime) << "\'"
			<< " and obstime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by obstime";

	return ost.str();
}

/** EGLITIS (for Qc2)
 * \brief select all rows from table \em data matching
 *  a single stationid, paramid=pid and obstime in [stime - etime]
 */
std::string kvQueries::selectData(const int stid, const int pid, const int tid,
		const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " WHERE stationid=" << stid << " and paramid=" << pid
			<< " and typeid=" << tid << " and obstime>=\'" << to_kvalobs_string(stime)
			<< "\'" << " and obstime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by obstime";

	return ost.str();
}

/** EGLITIS (for Qc2)
 * \brief select all rows from table \em data matching
 *  a single stationid, paramid=pid and obstime in [stime - etime]
 */
std::string kvQueries::selectData(const int stid, const int pid,
		const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime)
{
	ostringstream ost;

	ost << " WHERE stationid=" << stid << " and paramid=" << pid
			<< " and obstime>=\'" << to_kvalobs_string(stime) << "\'"
			<< " and obstime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by obstime";

	return ost.str();
}

/** EGLITIS (for Qc2)
 * \brief select all rows from table \em data matching
 *  paramid=pid and obstime in [stime - etime] and controlinfo="control string"
 */
std::string kvQueries::selectData(const int pid, const int tid,
		const boost::posix_time::ptime& stime, const boost::posix_time::ptime& etime,
		const string& controlString)
{
	ostringstream ost;

	ost << " WHERE paramid=" << pid << " and typeid=" << tid
			<< " and controlinfo=\'" << controlString << "\'"
			<< " and obstime>=\'" << to_kvalobs_string(stime) << "\'"
			<< " and obstime<=\'" << to_kvalobs_string(etime) << "\'"
			<< " order by obstime";

	return ost.str();
}

/** EGLITIS (for Qc2)
 * \brief A query to pick out missing float values especially
 *  for Qc2 tests, for all stations at one particular time
 */
std::string kvQueries::selectMissingData(const float value, const int pid,
		const boost::posix_time::ptime& Ptime)
{
	ostringstream ost;

	ost << " WHERE original=" << value << " and paramid=" << pid
			<< " and obstime=\'" << to_kvalobs_string(Ptime) << "\'"
			<< " order by obstime";

	return ost.str();
}

/** EGLITIS (for Qc2)
 * \brief A query to pick out missing float values especially
 *  for Qc2 tests, for all stations at one particular time
 */
std::string kvQueries::selectMissingData(const float value, const int pid,
		const int tid, const boost::posix_time::ptime& Ptime)
{
	ostringstream ost;

	ost << " WHERE original=" << value << " and paramid=" << pid
			<< " and typeid=" << tid << " and obstime=\'" << to_kvalobs_string(Ptime)
			<< "\'" << " order by obstime";

	return ost.str();
}
