/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: PreProcessMissingData.cc,v 1.3.2.6 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <stdlib.h>
#include "PreProcessMissingData.h"
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvTypes.h>
#include <milog/milog.h>
#include "PreProcessWorker.h"

using namespace kvalobs;
using namespace milog;
using namespace std;

PreProcessMissingData::
PreProcessMissingData()
{
}

PreProcessMissingData::
~PreProcessMissingData()
{
}

bool
PreProcessMissingData::
loadParamsAndTypes()
{
   miutil::miTime now( miutil::miTime::nowTime() );

   if( ! nextDbCheck.undef() && now < nextDbCheck &&
       !typeList.empty() && !paramList.empty() ) {
      return true;
   }

   std::list<kvalobs::kvTypes> tmpTypeList;
   std::list<kvalobs::kvParam> tmpParamList;

   kvDbGate dbGate( con_);
   std::string table;
   bool result;
   int  nOk=0;

   try{
      table = kvalobs::kvTypes().tableName();
      result = dbGate.select(tmpTypeList );

      if( result ) {
         nOk++;
         typeList = tmpTypeList;
      } else {
         LOGERROR("Failed to read the '" << table <<"' table from the database.\n" <<
                  "Reason: " << dbGate.getErrorStr());
      }

      table = kvalobs::kvParam().tableName();
      result = dbGate.select( tmpParamList );

      if( result ) {
         nOk++;
         paramList = tmpParamList;
      } else {
         LOGERROR("Failed to read the '" << table <<"' table from the database.\n" <<
                  "Reason: " << dbGate.getErrorStr());
      }
   }
   catch(dnmi::db::SQLException &ex){
         LOGERROR("Exception: Failed to read the '" << table <<"' table from the database.\n" <<
                  "Reason: " << ex.what());
   }
   catch(...){
      LOGERROR("Exception: Failed to read the '" << table <<"' table from the database.\n" <<
               "Reason: Unknown." );
   }

   if( nOk == 2 ) {
      now.addHour( 1 );
      nextDbCheck = miutil::miTime( now.year(), now.month(), now.day(), now.hour(), 0, 0);
   } else {
      now.addMin( 5 );
      nextDbCheck = now;
   }

   LOGDEBUG( "PreProcessMissingData: Next loading of 'types' and 'params' table: " + nextDbCheck.isoTime() );

   return nOk == 2;
}

bool
PreProcessMissingData::
paramIsMinute( int typeid_, int paramid )
{
   bool isMinParam = false;

   if( ! loadParamsAndTypes() ) {
      LOGERROR("PreProcessMissingData: The 'param' and 'types' table is not available.\n" <<
               "Can't decide if paramid (" << paramid << ") typeid (" << typeid_ << ") is minute data.");
      return false;
   }


   for( std::list<kvalobs::kvParam>::const_iterator it = paramList.begin();
        it != paramList.end(); ++it ) {
      if( it->paramID() == paramid ) {
         std::string name=it->name();
         //std::string::size_type i = name.find("_0");
         std::string::size_type i = name.find("RR_01");

         if( i != std::string::npos )
            isMinParam = true;
         break;
      }
   }

   for( std::list<kvalobs::kvTypes>::const_iterator it = typeList.begin();
         it != typeList.end(); ++it ) {
      if( it->typeID() == typeid_ ) {
         std::string c = it->obspgm();

         if( ! c.empty() && c[0]!='m' )
            isMinParam = false;

         break;
      }
   }


   return isMinParam;
}

void
PreProcessMissingData::
flagDataNotInObsPgm( const std::list<kvalobs::kvObsPgm> &obsPgm,
                     std::list<kvalobs::kvData> &data,
                     std::list<kvalobs::kvData> &wildObs
                   )
{
   bool hasThis;

   for( std::list<kvalobs::kvData>::iterator it = data.begin();
         it != data.end(); ++it ) {
      std::list<kvalobs::kvObsPgm>::const_iterator itop;

      hasThis=false;

      for( std::list<kvalobs::kvObsPgm>::const_iterator itop=obsPgm.begin();
            itop !=obsPgm.end(); itop++){
         // no missing-check for collector=TRUE
         //if (itop->collector())
         //   continue;

         int paramid= itop->paramID();
         int level  = itop->level();
         int sensor;
         int nr_sensor= itop->nr_sensor();

         if( it->paramID() == paramid &&
               it->sensor() < nr_sensor &&
               it->level() == level &&
               itop->isOn( it->obstime() ) ) {
            hasThis = true;
            break;
         }
      }

      if( ! hasThis ) {
         //It seems that this parameter is not defined in the
         //observation program.
         //TODO: Set a controll infoflag that identify this as a "wildobs".
         wildObs.push_back( *it );
      }
   }
}

bool
PreProcessMissingData::
isMissingMessage( const std::list<kvalobs::kvData> &datalist )const {
   if( datalist.empty() )
      return true;

   int fmis;
   for( std::list<kvalobs::kvData>::const_iterator it=datalist.begin();
         it != datalist.end(); ++it ) {
     fmis = it->controlinfo().MissingFlag();

     //Do we have a original value.
     if( it->original() != -32767 || fmis==0 || fmis == 2 || fmis == 4 )
        return false;
   }
   return true;
}

void 
PreProcessMissingData::
removeFromList( const kvalobs::kvData &data,
               std::list<kvalobs::kvData> &dataList )const
{
   for( std::list<kvalobs::kvData>::iterator it=dataList.begin();
        it != dataList.end();
        ++it ) {
      if( data.stationID() == it->stationID() &&
          data.typeID() == it->typeID() &&
          data.paramID() == it->paramID() &&
          data.level()== it->level() &&
          data.sensor() == it->sensor() ) {
         //Remove from the wildObsList
         dataList.erase( it );
         break;
      }
   }
}

void
PreProcessMissingData::doJob(long                 stationId, 
                             long                 typeId,
                             const miutil::miTime &obstime,
                             dnmi::db::Connection &con)
{
   std::ostringstream ost;
   bool  missingMessage;
   ost << "PreProcessMissingData(stationid=" << stationId << ")";
   LogContext ctxt(ost.str());
   ost.str("");

   if(typeId<0){
      LOGDEBUG1("Generated data is not checked for missing. typeId=" <<
                typeId);
      return;
   }else{
      LOGINFO("doJob STARTING typeId:" << typeId << " obstime:"
              << obstime   << std::endl);
   }
   // init database connection
   con_ = &con;
   kvDbGate dbGate( con_);
   bool     result;
   int      missingParamCount=0;
   int      paramObsPgmCount=0;  //Counts of params that should be in the observation

   // first fetch all observations matching stationId, obstime and typeId
   std::list<kvalobs::kvData> datalist;
   std::list<kvalobs::kvData> dataToSave;
   std::list<kvalobs::kvData> wildObsList;
   std::list<kvalobs::kvTypes> typeList;

   try{
      result = dbGate.select(datalist,
                             kvQueries::selectDataFromType(stationId,
                                                           typeId,
                                                           obstime));
   }
   catch(dnmi::db::SQLException &ex){
      LOGERROR("Exception: " << ex.what() << std::endl);
   }
   catch(...){
      LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
   }

   if (!result)
      return;

   missingMessage = isMissingMessage( datalist );

   // .. then fetch the observation program for this station
   std::list<kvalobs::kvObsPgm> obspgmlist;

   try{
      result = dbGate.select( obspgmlist,
                              kvQueries::selectObsPgm(stationId,
                                                      typeId,
                                                      obstime)
      );
   }
   catch(dnmi::db::SQLException &ex){
      LOGERROR("Exception: " << ex.what() << std::endl);
   }
   catch(...){
      LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
   }

   if (!result)
      return;

   flagDataNotInObsPgm( obspgmlist, datalist, wildObsList );

   boost::posix_time::ptime tbtime(boost::posix_time::microsec_clock::universal_time());

   // loop through obs_pgm and check if we have data for each
   // active parameter
   std::list<kvalobs::kvObsPgm>::const_iterator itop;

   for (itop=obspgmlist.begin(); itop!=obspgmlist.end(); itop++){
      // no missing-check for collector=TRUE
      if (itop->collector())
         continue;

      // check if this obspgm is active now..
      if (!itop->isOn(obstime))
         continue;

      paramObsPgmCount++;

      int paramid= itop->paramID();
      int level  = itop->level();
      int sensor, nr_sensor= itop->nr_sensor();
      bool countedThis=false;

      // loop through all sensors
      for (sensor=0; sensor<nr_sensor; sensor++){
         // check if we have this parameter
         std::list<kvalobs::kvData>::const_iterator itd;

         for (itd=datalist.begin(); itd!=datalist.end(); itd++){
            if (itd->paramID() == paramid &&
                  itd->level()   == level &&
                  itd->sensor()  == sensor &&
                  itd->typeID()  == typeId)
               break;
         }

         if (itd==datalist.end()){ //paramid not found
            if(!countedThis){
               missingParamCount++;
               countedThis=true;
            }


            // insert missing data
            kvControlInfo    controlinfo;
            kvUseInfo        useinfo;
            std::string failed;
            float            original  = -32767;
            float            corrected = -32767;
            controlinfo.MissingFlag(kvQCFlagTypes::status_orig_and_corr_missing);

            if( paramIsMinute( typeId, paramid ) ) {
               original  = 0.0;
               corrected = 0.0;
               controlinfo.MissingFlag(kvQCFlagTypes::status_ok );
            }

            kvData data(stationId,
                        to_ptime(obstime),
                        original,
                        paramid,
                        tbtime,
                        typeId,
                        sensor,
                        level,
                        corrected,
                        controlinfo,
                        useinfo,
                        failed);

            removeFromList( data, wildObsList );
            dataToSave.push_back( data );
         }
      }
   }

   if( ! dataToSave.empty() ) {
      try{
         result = dbGate.insert( dataToSave, false );
      }
      catch(dnmi::db::SQLException &ex){
         LOGERROR("Exception: " << ex.what() << std::endl);
      }
      catch(...){
         LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
      }
   }

#if 0
   if( ! wildObsList.empty() ) {
      try {
         dbGate.insert( wildObsList, true );
      }
      catch(dnmi::db::SQLException &ex){
         LOGERROR("Exception: " << ex.what() << std::endl);
      }
      catch(...){
         LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
      }
   }
#endif

   if(paramObsPgmCount>0){
      if(missingParamCount>0){
         LOGINFO("Missing " << missingParamCount << " parameters. Should be "
                 << paramObsPgmCount << " parameters for the station!");
      }else{
         LOGINFO("No missing parameters for the station!");
      }
   }else{
      LOGINFO("No parameters is expecting for the stations at this time!");
   }

   LOGINFO("doJob FINISHED" << std::endl);
}
