/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SendDataToQa.cc,v 1.2.2.1 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <iostream>
#include <sstream>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvWorkelement.h>
#include "SendDataToQa.h"
#include "mgrApp.h"
#include "NewDataCommand.h"


using namespace std;
using namespace miutil;
using namespace kvalobs;

SendDataToQa::
SendDataToQa(ManagerApp &app_, 
             dnmi::thread::CommandQue &inputQue_)
:app(app_), inputQue(inputQue_)
{
}

void 
SendDataToQa::operator()()
{
   const int WAIT_ON_QUE_TIMEOUT=1;
   const int WAIT_ON_QA_BASE_UP=60;

   dnmi::thread::CommandBase *cmd;
   NewDataCommand *newCmd;
   int qaDownTime=-1;

   milog::LogContext context("SendDataToQa");
   LOGINFO("Thread starting!");

   processStartedElements(WAIT_ON_QA_BASE_UP);

   while(!app.shutdown()){
      if(qaDownTime>-1){
         qaDownTime++;

         if(qaDownTime<WAIT_ON_QA_BASE_UP){
            sleep(1);
            continue;
         }else{
            qaDownTime=-1;
         }
      }

      cmd=inputQue.peek(WAIT_ON_QUE_TIMEOUT);

      if(!cmd ){
         continue;
      }

      newCmd=dynamic_cast<NewDataCommand*>(cmd);

      if(!newCmd){
         LOGERROR("Unknown Command." << endl);

         delete inputQue.remove(cmd);
         continue;
      }

      kvalobs::kvStationInfoList stationInfoList = newCmd->getStationInfo();

      for( kvalobs::IkvStationInfoList it = stationInfoList.begin();
           it != stationInfoList.end(); it++){
         if(sendData(*it)){
            qaDownTime=-1;
         }else{
            qaDownTime=0;
         }
      }

      if(qaDownTime<0)
         delete inputQue.remove(cmd);

   }

   LOGINFO("Thread terminated!\n");
}

bool
SendDataToQa::
sendData(const kvalobs::kvStationInfo &stationInfo)
{ 
   bool qaOk=true;
   bool bussy=true;

   while(qaOk && bussy && !app.shutdown()){
      qaOk=app.sendDataToQA(stationInfo, bussy);

      if(bussy){
         LOGDEBUG("sendData: kvQabased is BUSSY!");
         sleep(1);
      }
   }

   return qaOk;
}

void
SendDataToQa::
processStartedElements(int wait_on_qabase)
{
   ostringstream ost;
   int qaDownTime;
   list<kvWorkelement>  workList;
   dnmi::db::Connection *con=getNewDbConnection();

   if(!con)
      return;

   kvDbGate gate(con);

   ost << "WHERE process_start IS NOT NULL  AND qa_stop IS NULL";

   if(!gate.select(workList, ost.str(), "workque")){
      LOGERROR("Cant get 'workelement' from 'workque'!" <<
               "-- " << gate.getErrorStr());
      app.releaseDbConnection(con);
      exit(1);
      return;
   }

   for(list<kvWorkelement>::iterator it=workList.begin();
         it!=workList.end() && !app.shutdown();
         it++){

      qaDownTime=-1;

      while(!app.shutdown()){
         if(qaDownTime>-1){
            qaDownTime++;

            if(qaDownTime<wait_on_qabase){
               sleep(1);
               continue;
            }else{
               qaDownTime=-1;
            }
         }

         if(sendData(kvStationInfo(it->stationID(),
                                   it->obstime(),
                                   it->typeID()))){
            break;
         }else{
            qaDownTime=0;
         }
      }
   }

   app.releaseDbConnection(con);
}


dnmi::db::Connection*
SendDataToQa::
getNewDbConnection()
{
   dnmi::db::Connection *con;

   do{
      con=app.getNewDbConnection();

      if(con){
         LOGDEBUG("Created a new connection to the database!");
         return con;
      }

      LOGINFO("Can't create a connection to the database, retry in 5 seconds ..");
      sleep(5);
   }while(!app.shutdown());

   //We have failed to create a new database connection and we have got a
   //shutdown condition.
   return 0;
}

