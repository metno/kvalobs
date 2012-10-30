/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SelectDataToProcess.cc,v 1.4.2.2 2007/09/27 09:02:35 paule Exp $                                                       

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
#include "SelectDataToProcess.h"
#include <milog/milog.h>
#include <kvalobs/kvStationInfoCommand.h>

using namespace std;
using namespace kvalobs;
using namespace miutil;

SelectDataToProcess::
SelectDataToProcess(ManagerApp               &app_,
                    dnmi::thread::CommandQue &inputQue_,
                    dnmi::thread::CommandQue &outputQue_,
                    dnmi::thread::CommandQue &workInProcessQue_)
:app(app_), inputQue(inputQue_), outputQue(outputQue_),
 workInProcessQue(workInProcessQue_), dbCon(0)
{
}

SelectDataToProcess::   
SelectDataToProcess(const SelectDataToProcess &p)
:app(p.app), inputQue(p.inputQue), outputQue(p.outputQue),
 workInProcessQue(p.workInProcessQue), dbCon(p.dbCon),iRange(1)
{
}

SelectDataToProcess::
~SelectDataToProcess()
{
}

void  
SelectDataToProcess::
processElements(std::list<kvalobs::kvWorkelement> &workList, 
                dnmi::db::Connection  *con)
{
   const int PROCESS_QUE_LIMIT=4;
   const int PROCESS_QUE_LOWPRI_LIMIT=2;
   kvDbGate      gate(con);
   ostringstream ost;
   int           limit;

   if(lowpri)
      limit=PROCESS_QUE_LOWPRI_LIMIT;
   else
      limit=PROCESS_QUE_LIMIT;

   list<kvWorkelement>::reverse_iterator it=workList.rbegin();

   while(it!=workList.rend() && !app.shutdown()){
      if(lowpri && !inputQue.empty()) //High pri data waiting?
         return;

      if((workInProcessQue.size()+outputQue.size())>limit){
         /*LOGDEBUG("processElements: to many jobbs in the the work line!" << endl
	       << "preprocessQue: " << outputQue.size() << endl
	       << "        qaQue: " << workInProcessQue.size());
          */
         //We dont push data to the outputQue if the data waiting
         //to be pushed to kvQabased is to big.
         sleep(1);
         continue;
      }

      StationInfoCommand *cmd;

      try{
         cmd=new StationInfoCommand();
      }
      catch(...){
         LOGERROR("MOMEM: Cant alocate <kvalobs::StationInfoCommand>!");
         return;
      }

      it->process_start(boost::posix_time::microsec_clock::universal_time());

      ost.str("");

      ost << "UPDATE workque SET process_start='"
            << to_simple_string(it->process_start())
            << "' WHERE stationid=" << it->stationID()
            << "  AND obstime='" << to_simple_string(it->obstime())
            << "' AND typeid=" << it->typeID();

      try{
         con->beginTransaction();
      }
      catch(dnmi::db::SQLException &ex){
         delete cmd;
         LOGERROR("processElements: DBERROR: cant start transaction!");
         it++;
         continue;
      }


      if(!gate.exec(ost.str())){
         LOGERROR("processElements:: Cant update table workque." <<
                  "-- Stationid: " << it->stationID() << endl <<
                  "--   obstime: " << it->obstime()  << endl <<
                  "--    typeid: " << it->typeID() << endl <<
                  "--  query: " << ost.str() << endl <<
                  "-- reason: " << gate.getErrorStr());
         try{
            con->rollBack();
         }
         catch(...){
         }

         delete cmd;
         return;
      }


      cmd->addStationInfo(kvStationInfo(it->stationID(),
                                        it->obstime(),
                                        it->typeID()));

      try{
         LOGDEBUG("Sending data to PreProcessWorker!");
         outputQue.postAndBrodcast(cmd);

         LOGDEBUG("processElements: started procesing of: " << endl <<
                  "-- Stationid: " << it->stationID() << endl <<
                  "-- obstime:   " << it->obstime() << endl <<
                  "-- typeid:    " << it->typeID() << endl <<
                  "-- priority:  " << it->priority());
      }
      catch(...){
         it->process_start(boost::posix_time::ptime()); //Set to NULL

         try{
            con->rollBack();
         }
         catch(...){
         }

         LOGERROR("Cant add the Command to the que. (NOMEM!)");
         delete cmd;
      }

      try{
         con->endTransaction();
      }
      catch(...){
         LOGERROR("processElemnts: DBERROR: cant commit (endTransaction)!");
      }

      it++;
   }

}


dnmi::db::Connection*
SelectDataToProcess::
newConnection()
{
   while(!dbCon && !app.shutdown()){
      dbCon=app.getNewDbConnection();

      if(dbCon){
         LOGDEBUG("Created a new connection to the database!");
      }else{
         LOGINFO("Can't create a db connection, retry in 5 seconds ...");
         sleep(5);
      }
   }

   return dbCon;
}

bool
SelectDataToProcess::
processLowPriData(const boost::posix_time::ptime &baseTime)
{
   const int DAY_LIMIT=7;
   const int LOW_PRI_LIMIT=10;
   boost::posix_time::ptime              lowerBound;
   boost::posix_time::ptime              upperBound;
   bool                moreData=true;
   ostringstream       ost;
   list<kvWorkelement> workList;

   dbCon=newConnection();

   if(!dbCon) //In shutdown
      return false;

   lowpri=true;
   kvDbGate gate(dbCon);

   milog::LogContext logContext("processLowPriData");

   if(iRange<DAY_LIMIT){
      lowerBound=baseTime;
      upperBound=baseTime;

      lowerBound -= boost::gregorian::days(iRange+1);
      upperBound -= boost::gregorian::days(iRange);

      LOGDEBUG("processLowPriData: iRange=" << iRange << endl <<
               "   baseTime: " << baseTime  << endl <<
               " lowerBound: " << lowerBound << endl <<
               " upperBound: " << upperBound );

      ost << "WHERE obstime>='" << to_simple_string(lowerBound) << "' AND "
            << "obstime<'" << to_simple_string(upperBound) << "' AND "
            << "process_start IS NULL "
            << "ORDER BY priority, stationid, typeid, "
            << "         obstime DESC "
            << "LIMIT " << LOW_PRI_LIMIT;


      if(!gate.select(workList, ost.str(), "workque")){
         LOGERROR("Cant get 'workelement' from 'workque'!" <<
                  "-- " << gate.getErrorStr());
         return false;
      }

      if(workList.size()<LOW_PRI_LIMIT)
         iRange++;
   }else{
      //process data older than DAY_LIMIT days.

      upperBound=baseTime;
      upperBound -= boost::gregorian::days(DAY_LIMIT);

      LOGDEBUG("processLowPriData: iRange<" << DAY_LIMIT << endl <<
               "   baseTime: " << baseTime  << endl <<
               " upperBound: " << upperBound );

      ost << "WHERE obstime<'"  << to_simple_string(upperBound) << "' AND "
            << "process_start IS NULL "
            << "ORDER BY priority, stationid, typeid, "
            << "         obstime DESC "
            << "LIMIT " << LOW_PRI_LIMIT;

      if(!gate.select(workList, ost.str(), "workque")){
         LOGERROR("Cant get 'workelement' from 'workque'!" <<
                  "-- " << gate.getErrorStr());
         return false;
      }

      if(workList.size()<LOW_PRI_LIMIT)
         moreData=false;
   }

   processElements(workList, dbCon);

   return moreData;
}

void 
SelectDataToProcess::
operator()()
{
   const int                 HIGH_PRI_LIMIT=5;
   const int                 CON_IDLE_TIME=60;
   const int                 WAIT_ON_QUE_TIMEOUT=1;
   int                       conIdleTime=0;
   StationInfoCommand        *infoCmd;
   dnmi::thread::CommandBase *cmd;
   list<kvWorkelement>       workList;
   ostringstream             ost;
   boost::posix_time::ptime                    baseTime;
   int                       nElements;
   bool                      moreData=true;
   bool                      starting=true;


   milog::LogContext logContext("SelectDataToProcess");

   LOGINFO("Starting!");

   while(!app.shutdown()){
      //    sleep(1);
      //continue;
      if(!starting && moreData && inputQue.empty()){
         cmd=0;
      }else{
         cmd=inputQue.get(WAIT_ON_QUE_TIMEOUT);
      }

      if(!cmd && !starting){
         if(!baseTime.is_not_a_date_time() && moreData){
            moreData=processLowPriData(baseTime);
            continue;
         }else{
            conIdleTime+=WAIT_ON_QUE_TIMEOUT;

            if(conIdleTime>CON_IDLE_TIME){
               if(dbCon){
                  LOGDEBUG("Closing the database connection!");
                  app.releaseDbConnection(dbCon);
                  dbCon=0;
               }

               conIdleTime=0;
            }

         }
      }

      if(!cmd && !starting)
         continue;

      //We dont need it anymore.
      if(cmd) //We may be in startup
         delete cmd;

      dbCon=newConnection();

      if(!dbCon){
         //We are in shutdown
         continue;
      }

      //Process data that is not older than one day
      starting=false;
      moreData=true;
      iRange=1;
      conIdleTime=0;
      baseTime=boost::posix_time::microsec_clock::universal_time();
      boost::posix_time::ptime  lowerBound;

      lowpri=false;
      lowerBound=baseTime;
      lowerBound -= boost::gregorian::days(1);

      ost.str("");
      ost << "WHERE obstime>='" << to_simple_string(lowerBound) << "' AND "
            << "process_start IS NULL "
            << "ORDER BY priority, obstime, tbtime, typeid DESC "
            << "LIMIT " << HIGH_PRI_LIMIT;


      LOGDEBUG("[" << ost.str() << "]");
      kvDbGate gate(dbCon);

      do{
         //Sletter alle elementene i k�en
         inputQue.clear();



         if(!gate.select(workList, ost.str(), "workque")){
            LOGERROR("Cant get 'workelement' from 'workque'!" <<
                     "-- " << gate.getErrorStr());

            nElements=0;
         }else{
            nElements=workList.size();
         }

         processElements(workList, dbCon);

      }while(nElements>0 && !app.shutdown());
   }

   if(dbCon)
      app.releaseDbConnection(dbCon);

   LOGINFO("Terminating!");

}


