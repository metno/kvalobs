/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SendDataToQaBase.cc,v 1.8.2.3 2007/09/27 09:02:38 paule Exp $                                                       

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
#include <corbahelper/corbaApp.h>
#include <dnmithread/mtcout.h>
#include "SendDataToQaBase.h"
#include "CheckedDataCommand.h"

using namespace miutil;
using namespace std;


void 
SendDataToQaBase::
operator()()
{
  const int WAIT_TIME=60;
  CheckedDataCommand     *cmd;
  dnmi::thread::CommandBase *tmpCmd;
  CKvalObs::StationInfo stInfo;
  micutil::KeyValList   args;  
  int nn=0;
  miTime time_start(stime_start);
  miTime time_stop(stime_stop);
  miTime time;
  miString stime, spos;
  bool   error=false;
  int    timeout;
  CORBA::Boolean bussy;

  if(!logpath.empty()){
    args.length(2);
    args[0].key=const_cast<const char*>("logpath");
    args[0].val=const_cast<const char*>(logpath.c_str());
    args[1].key=const_cast<const char*>("update_workque");
    args[1].val=const_cast<const char*>("false");
  }

  for (time=time_start; !error && time<= time_stop; time.addHour(1)){
    stime= time.isoTime();
    COUT("Running qaBase for time:" << stime << endl);
    
    for (int p=0; p<stations.size(); p++){
      spos= stations[p];
      COUT("   Running for station:" << spos << endl);
      createDummyData(stInfo, spos, stime);
      
      try{
	while(!refQaBase->newDataCB(stInfo, callback, args, bussy)){
	  if(!bussy)
	    break;

	  sleep(1);
	}

	nn++;
      }
      catch(...){
	COUT("CONNECT ERROR: cant send request to kvQaBased!!!\n");
	error=true;
      }

      if(error)
	break;

      timeout=0;

      while(timeout<WAIT_TIME){
	tmpCmd=inputque.get(1);
	
	if(!tmpCmd){
	  timeout++;
	  sleep(1);
	}else{
	  cmd=dynamic_cast<CheckedDataCommand*>(tmpCmd);
	  
	  if(!cmd){
	    COUT("INVALID COMMAND: received an unexpected command!!!\n\n");
	  }

	  break;
	}
      }
      
    }
  }
  
  COUT("Total number of messages sent to qabased:" << nn << endl);
  CorbaHelper::CorbaApp::getCorbaApp()->getOrb()->shutdown(0);
}



void 
SendDataToQaBase::
createDummyData(CKvalObs::StationInfo &st, 
		const std::string& spos, const std::string& stime)
{
  st.stationId= atoi(spos.c_str());
  st.obstime= stime.c_str();
  
  if(type.empty())
  	st.typeId_=0;
  else
	st.typeId_=atoi(type.c_str());
	
  CERR("Sendata: obstime: " << st.obstime << " stationid: " << st.stationId << " typeid: " << st.typeId_ << endl);
}

