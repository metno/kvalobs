/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Replay.cc,v 1.3.2.1 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <sstream>
#include <milog/milog.h>
#include "Replay.h"
#include "obsevent.h"

using namespace std;
using namespace kvalobs;
using namespace kvsynopd;

Replay::Replay(App &app_, 
	       dnmi::thread::CommandQue &inputQue)
  : app(app_), que(inputQue)
{
}
  
void 
Replay::operator()()
{
  dnmi::thread::CommandBase *com;
  ObsEvent                  *event;
  /*
    struct SynopData{
    long     stationid;
    string   termin;
    boolean  isOk;
    string   message;
    string   synop;
  };
  */
  milog::LogContext context("Replay");

  while(!app.shutdown()){
    com=que.get(1);

    if(!com)
      continue;
    
    event=dynamic_cast<ObsEvent*>(com);

    if(!event){
      LOGERROR("Unexpected event!");
      delete com;
      continue;
    }
    
    SynopData result;           

    result.stationid=event->stationInfo()->wmono();
    result.termin=CORBA::string_dup(miutil::miTime::nowTime().isoTime().c_str());
    result.isOk=event->isOk();
    result.message=CORBA::string_dup(event->msg().str().c_str());
    result.synop=CORBA::string_dup(event->synop().c_str());

    try{
      synopcb_var cb=event->callback();
      
      if(!CORBA::is_nil(cb)){
	cb->synop(result);
      }else{
	LOGDEBUG("NILREF: callback!");
      } 
    }
    catch(...){
      LOGERROR("Cant send replay to caller!");
    }

    delete event;
  }
} 


