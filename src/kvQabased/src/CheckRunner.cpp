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
#include <miutil/gettimeofday.h>
#include <miutil/msleep.h>
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

/**
* Start a transaction that will be rolled back upon object destruction,
* unless it has been explicitly committed first.
*/
class AutoRollbackTransaction
{
public:
   explicit AutoRollbackTransaction(db::DatabaseAccess & db) :
   db_(db), committed_(false)
   {
      db_.beginTransaction();
   }
   ~AutoRollbackTransaction()
   {
      if ( not committed_ )
      {
         try
         {
            db_.rollback();
         }
         catch ( std::exception & )
         {
            milog::LogContext context("transaction");
            LOGWARN("Error when attempting to rollback.");
         }
      }
   }
   void commit()
   {
      db_.commit();
      committed_ = true;
   }
private:
   db::DatabaseAccess & db_;
   bool committed_;
};

void
logTransaction( bool ok,
                double start,
                int shortRetries,
                int longRetries,
                int aborted,
                std::exception *ex=0)
{
   using namespace std;

   double duration( miutil::gettimeofday() - start );
   string extra(".");
   string sAborted;
   std::ostringstream sCommon;

   if( ex ) {
      dnmi::db::SQLException *sqlex=dynamic_cast<dnmi::db::SQLException*>(ex);

      if( sqlex ) {
         extra = string(" Excetion (") + typeid(*sqlex).name()+"): [" + sqlex->errorCode() + "]: " + sqlex->what() + ".";
      } else {
         extra = string(" Excetion (")+ typeid(*ex).name()+"): " + ex->what() + ".";
      }
   }

   if( aborted > 0 ) {
      std::ostringstream s;
      s << " #aborted: " << aborted;
      sAborted = s.str();
   }

   sCommon << fixed << setprecision(3) << duration <<
         " s #swait: " << shortRetries <<
         " #lwait: " << longRetries << sAborted <<  extra;

   if( ok ) {
      if( shortRetries == 0 && longRetries == 0 ) {
         IDLOGINFO( "transaction", "SUCCESS duration: " << sCommon.str() );
      } else {
         IDLOGWARN( "transaction", "SUCCESS duration: " << sCommon.str() )
      }
   } else {
      IDLOGERROR("transaction", "FAILED  duration: " << sCommon.str() );
      IDLOGERROR("failed", "FAILED duration: " << sCommon.str() );
   }
}
}

void CheckRunner::newObservation(const kvalobs::kvStationInfo & obs, std::ostream * scriptLog)
{
   const int shortSleep=100;
   const int longSleep=300;
   const int nRetry=3;
   double start;
   int nLongRetries=0;
   int nShortRetries=0;
   int aborted=0;

   std::ostringstream logContext;
   logContext << obs.obstime() << '/' << obs.typeID() << '/' << obs.stationID();
   milog::LogContext context(logContext.str());

   if ( not shouldRunAnyChecks(obs) )
   {
      LOGDEBUG("Will not run any checks on observation: " << obs);
      return;
   }

   LOGINFO("Checking " << obs);
   start = miutil::gettimeofday();

   // Will try up to nRetry*nRetry times in case of error
   try
   {
      for ( int k = 0; k < nRetry; ++ k )
      {
         if( k != 0 )
         {
            nLongRetries++;
            miutil::msleep( longSleep );
         }

         for ( int i = 0; i < nRetry ; ++ i )
         {
            if( i != 0 )
            {
               nShortRetries++;
               miutil::msleep( shortSleep );
            }

            try
            {
				for ( int i = 0; i < 256; ++ i )
				{
					try
					{
					   checkObservation(obs, scriptLog);
					   logTransaction( true, start, nShortRetries, nLongRetries, aborted );
					   return;
					}
					catch (dnmi::db::SQLSerializeError & )
					{
					   LOGWARN("Serialization error! Retrying");
					}
				}
				// Happens if we get more than 256 serialization errors in a row
				throw std::runtime_error("Serialization error!");
            }
            catch( const dnmi::db::SQLAborted &ex ) {
               LOGWARN("Aborted error! Retrying Reason: " << ex.what() );
               aborted++;
            }
         }
      }

      // final attempt:
      checkObservation(obs, scriptLog);
      logTransaction( true, start, nShortRetries, nLongRetries, aborted );
   }
   catch ( std::exception & e )
   {
      logTransaction( false, start, nShortRetries, nLongRetries, aborted, &e );
      //LOGERROR(e.what());
      throw;
   }
}

void CheckRunner::checkObservation(const kvalobs::kvStationInfo & obs, std::ostream * scriptLog)
{
   db::CachedDatabaseAccess cdb(db_, obs);
   db::DelayedSaveDatabaseAccess db(& cdb);
   AutoRollbackTransaction transaction(db);

   LOGDEBUG("Getting checks for observation");
   db::DatabaseAccess::CheckList checkList;
   db.getChecks(& checkList, obs);
   LOGDEBUG1("Received " << checkList.size() << " checks to run");


   LOGDEBUG("Getting list of expected parameters from station");
   db::DatabaseAccess::ParameterList expectedParameters;
   db.getParametersToCheck(& expectedParameters, obs);


   LOGDEBUG("Fetching observation data from database");
   std::set<std::string> parametersInData; // list of all parameters in observation data set
   db::DatabaseAccess::DataList observationData;
   for ( db::DatabaseAccess::ParameterList::const_iterator it = expectedParameters.begin(); it != expectedParameters.end(); ++ it )
   {
      db::DatabaseAccess::DataList d;
      db.getData(& d, obs, * it, 0);
      d.remove_if(std::not1(have_typeid(obs.typeID())));
      if ( not d.empty() )
      {
         parametersInData.insert(* it);
         observationData.insert(observationData.end(), d.begin(), d.end());
      }
   }
   //observationData.remove_if(std::not1(have_typeid(obs.typeID())));

   if ( haveAnyHqcCorrectedElements(observationData) )
   {
      LOGINFO("Observation is HQC-modified. Will not run tests on this");
      return;
   }

   if ( qcxFilter_.empty() )
   {
      resetObservationDataFlags(observationData);
      resetCFailed(observationData);
      db.write(observationData);
   }

   for ( db::DatabaseAccess::CheckList::const_iterator check = checkList.begin(); check != checkList.end(); ++ check )
   {
      std::string checkName = check->checkname();
      milog::LogContext context(checkName);
      try
      {
         bool hasAnyParametersRequiredByCheck = false;

         std::string signatureString = check->checksignature();
         CheckSignature signature(signatureString.c_str(), obs.stationID());
         const DataRequirement * obsRequirement = signature.obs();
         if ( obsRequirement )
         {
            for ( std::set<std::string>::const_iterator it = parametersInData.begin(); it != parametersInData.end(); ++ it )
               if ( obsRequirement->haveParameter(* it) )
               {
                  hasAnyParametersRequiredByCheck = true;
                  break;
               }
         }
         else
            hasAnyParametersRequiredByCheck = true;

         if ( hasAnyParametersRequiredByCheck and shouldRunCheck(obs, * check, expectedParameters) )
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

            LOGDEBUG("Check done. modification size " << modifications.size());

            db.write(modifications);
         }
         else
            LOGDEBUG1("Skipping check " << check->qcx());
      }
      catch ( std::bad_alloc & e )
      {
         LOGFATAL(e.what());
         throw;
      }
      catch ( NoErrorLogException & e )
      {
         LOGINFO(e.what());
      }
      catch( const dnmi::db::SQLException &ex )
      {
         //Must catch SQLException before std::exception, since
         //SQLException is derived from std::exception.
         //The exception is just rethrown.
         throw;
      }
      catch ( std::exception & e )
      {
         // errors that are not related to database are merely logged, and
         // we merely continue running next test
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

   transaction.commit();
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

   if ( (not isShip(obs))
		   //or expectedParameters.empty() )
         and not signatureMatchesExpectedParameters(CheckSignature(check.checksignature(), obs.stationID()), expectedParameters))
      return false;

   return true;
}

bool CheckRunner::shouldRunAnyChecks(const kvalobs::kvStationInfo & obs) const
{
   return obs.typeID() > 0; // not aggregated value
}


void CheckRunner::resetObservationDataFlags(db::DatabaseAccess::DataList & observationData)
{
   for ( db::DatabaseAccess::DataList::iterator it = observationData.begin(); it != observationData.end(); ++ it )
   {
      kvalobs::kvControlInfo oldCi = it->controlinfo();

      kvalobs::kvControlInfo newCi;

      // All flags should be 0, with a few exceptions:
      newCi.set(kvQCFlagTypes::f_fagg, oldCi.flag(kvQCFlagTypes::f_fagg));
      newCi.set(kvQCFlagTypes::f_fmis, oldCi.flag(kvQCFlagTypes::f_fmis));
      newCi.set(kvQCFlagTypes::f_fd, oldCi.flag(kvQCFlagTypes::f_fd));
      if ( oldCi.flag(kvQCFlagTypes::f_fpre) == 7 )
         newCi.set(kvQCFlagTypes::f_fpre, 7);

      it->controlinfo(newCi);

      kvalobs::kvUseInfo ui = it->useinfo();
      ui.setUseFlags(newCi);
      it->useinfo(ui);
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
