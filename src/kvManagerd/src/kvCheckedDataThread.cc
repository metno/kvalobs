/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvCheckedDataThread.cc,v 1.5.6.1 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <milog.h>
#include <kvCheckedDataThread.h>
#include <kvStationInfoCommand.h>

using namespace std;

  //idName brukes kun i logmeldinger og utskrift til skjerm.
KvCheckedDataThread::
KvCheckedDataThread(ManagerApp &app_, 
		    dnmi::thread::CommandQue &inputQue_)
  :app(app_), inputQue(inputQue_), dbCon(0)
{
}
	   

void
KvCheckedDataThread:: 
operator()()
{
  const int                   CON_IDLE_TIME=60;
  const int                   WAIT_ON_QUE_TIMEOUT=1;
  int                         conIdleTime=0;
  dnmi::thread::CommandBase   *cmd; 
  kvalobs::StationInfoCommand *newCmd;

  milog::LogContext logContext("CheckedDataThread");
  LOGINFO("KvCheckedDataThread: starting!\n");

  while(!app.shutdown()){
    cmd=inputQue.get(WAIT_ON_QUE_TIMEOUT);

    if(!cmd){
      conIdleTime+=WAIT_ON_QUE_TIMEOUT;
      
      if(conIdleTime>CON_IDLE_TIME){
	if(dbCon){
	  LOGDEBUG("Closing the database connection!");
	  app.releaseDbConnection(dbCon);
	  dbCon=0;
	}

	conIdleTime=0;
      }
      
      continue;
    }
    
    if(!dbCon){
      //Will try to create a new connection to the database, we will
      //not continue before a connection is created or the application
      //is shutdown.

      do{
	dbCon=app.getNewDbConnection();
	
	if(dbCon){
	  LOGDEBUG("New connection to the database created!");
	  break;
	}
	
	LOGINFO("Can't create a connection to the database, retry in 5 seconds ..");
	sleep(5);
      }while(!app.shutdown());

      if(!dbCon){
	//We have failed to create a new database connection and we have got a 
	//shutdown condition. We use continue to evaluate the outer while loop 
	//that in turn will end this thread.
	continue; 
      }
    }

    newCmd=dynamic_cast<kvalobs::StationInfoCommand*>(cmd);
    
    if(!newCmd){
      LOGERROR("Unknown Command." << endl);
      delete cmd;
      continue;
    }

  
    app.sendDataToKvService(newCmd->getStationInfo());
    
    delete newCmd;
    conIdleTime=0;
  }

  if(dbCon)
    app.releaseDbConnection(dbCon);
  
  LOGINFO("Terminated!\n");

}


