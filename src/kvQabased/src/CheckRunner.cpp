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

#include "CheckRunner.h"
#include <scriptcreate/KvalobsCheckScript.h>
#include <db/DelayedSaveDatabaseAccess.h>
#include <db/CachedDatabaseAccess.h>
#include <db/returntypes/CheckSignature.h>
#include <db/returntypes/kvCronString.h>
#include <milog/milog.h>
#include <iterator>
#include <new>


namespace qabase
{

CheckRunner::CheckRunner(db::DatabaseAccess & database) :
		db_(& database)
{
}

CheckRunner::~CheckRunner()
{
}

namespace
{
struct have_typeid : std::unary_function<kvalobs::kvData, bool>
{
	int type_;
	have_typeid(int typeID) : type_(typeID) {}
	bool operator () (const kvalobs::kvData & d) const
	{
		return d.typeID() == type_;
	}
};
}

void CheckRunner::newObservation(const kvalobs::kvStationInfo & obs, std::ostream * scriptLog)
{
	std::ostringstream logContext;
	logContext << obs.obstime() << '/' << obs.typeID() << '/' << obs.stationID();
	milog::LogContext context(logContext.str());

	if ( not shouldRunAnyChecks(obs) )
	{
		LOGINFO("Will not run any checks on observation: " << obs);
		return;
	}

	db::CachedDatabaseAccess cdb(db_, obs);
	db::DelayedSaveDatabaseAccess db(& cdb);

	LOGINFO("Checking " << obs);

	LOGDEBUG("Getting checks for observation");
	db::DatabaseAccess::CheckList checkList;
	db.getChecks(& checkList, obs);

	LOGDEBUG("Getting list of expected parameters from station");
	db::DatabaseAccess::ParameterList expectedParameters;
	db.getExpectedParameters(& expectedParameters, obs);

	LOGDEBUG("Fetching observation data from database");
	db::DatabaseAccess::DataList observationData;
	for ( db::DatabaseAccess::ParameterList::const_iterator it = expectedParameters.begin(); it != expectedParameters.end(); ++ it )
		db.getData(& observationData, obs, * it, 0);
	observationData.remove_if(std::not1(have_typeid(obs.typeID())));

	if ( haveAnyHqcCorrectedElements(observationData) )
	{
		LOGINFO("Observation is HQC-modified. Will not run tests on this");
		return;
	}

	if ( qcxFilter_.empty() )
	{
		resetObservationDataFlags(observationData, expectedParameters);
		resetCFailed(observationData);
		db.write(observationData);
	}

	for ( db::DatabaseAccess::CheckList::const_iterator check = checkList.begin(); check != checkList.end(); ++ check )
	{
		try
		{
			if ( shouldRunCheck(obs, * check, expectedParameters) )
			{
				db::DatabaseAccess::DataList modifications;

				KvalobsCheckScript script(db, obs, * check, scriptLog);

				LOGDEBUG("Running check " << * check);
				script.run(& modifications);

				// Set new useinfo flags
				for ( db::DatabaseAccess::DataList::iterator it = modifications.begin(); it != modifications.end(); ++ it )
				{
					kvalobs::kvUseInfo ui = it->useinfo();
					ui.setUseFlags(it->controlinfo());
					it->useinfo(ui);
				}

				LOGDEBUG("check " << check->checkname() << " done. modification size " << modifications.size());

				db.write(modifications);
			}
		}
		catch ( std::bad_alloc & e )
		{
			LOGFATAL(e.what());
			throw;
		}
		catch ( std::exception & e )
		{
			LOGERROR(e.what());
		}
	}

	if ( scriptLog and not db.uncommitted().empty() )
	{
		(*scriptLog) << "Saving " << db.uncommitted().size() << " elements to database:";
		for ( db::DelayedSaveDatabaseAccess::SavedData::const_iterator it = db.uncommitted().begin(); it != db.uncommitted().end(); ++ it )
			(*scriptLog) << '\n' << * it;
		(*scriptLog) << std::endl;
	}
	db.commit(); // hva hvis denne feiler?
}

namespace
{
bool isShip(const kvalobs::kvStationInfo & obs)
{
	// Ships get assigned a stationid higher than this
	return obs.stationID() > 10000000;
}

bool signatureMatchesExpectedParameters(const CheckSignature & concreteSignature,
		const db::DatabaseAccess::ParameterList & expectedParameters)
{
	const DataRequirement * obsReq = concreteSignature.obs();
	if (obsReq)
	{
		for (DataRequirement::ParameterList::const_iterator param =
				obsReq->parameter().begin(); param != obsReq->parameter().end(); ++param)
			if (expectedParameters.count(param->baseName()))
				return true;
		return false;
	}
	return true;
}

bool checkShouldRunAtThisHour(const std::string & checkActive,
		const kvalobs::kvStationInfo & obs)
{
	kvalobs::CronString cs(checkActive);
	return cs.active(obs.obstime());
}
}

bool CheckRunner::shouldRunCheck(const kvalobs::kvStationInfo & obs,
		const kvalobs::kvChecks & check,
		const db::DatabaseAccess::ParameterList & expectedParameters) const
{
	if ( not qcxFilter_.empty() )
		if ( qcxFilter_.find(check.qcx()) == qcxFilter_.end() )
			return false;

	if (not checkShouldRunAtThisHour(check.active(), obs))
		return false;

	if (not isShip(obs) and
			not signatureMatchesExpectedParameters(CheckSignature(check.checksignature(), obs.stationID()), expectedParameters))
		return false;



	return true;
}

bool CheckRunner::shouldRunAnyChecks(const kvalobs::kvStationInfo & obs) const
{
	return obs.typeID() > 0; // not aggregated value
}


void CheckRunner::resetObservationDataFlags(db::DatabaseAccess::DataList & observationData, const db::DatabaseAccess::ParameterList & expectedParameters)
{
	for ( db::DatabaseAccess::DataList::iterator it = observationData.begin(); it != observationData.end(); ++ it )
	{
		kvalobs::kvControlInfo ci = it->controlinfo();

		// setting all flags but fagg(0) and fmis(6) to zero
		for ( int i = 1; i < 16; ++ i )
			if ( i != 6 )
				ci.set(i, 0);
		it->controlinfo(ci);
	}
}

void CheckRunner::resetCFailed(db::DatabaseAccess::DataList & observationData)
{
	for ( db::DatabaseAccess::DataList::iterator it = observationData.begin(); it != observationData.end(); ++ it )
		it->cfailed("");
}


bool CheckRunner::haveAnyHqcCorrectedElements(const db::DatabaseAccess::DataList & observationData) const
{
	for ( db::DatabaseAccess::DataList::const_iterator it = observationData.begin(); it != observationData.end(); ++ it )
		if ( kvalobs::hqc::hqc_touched(* it) )
			return true;
	return false;
}


}
