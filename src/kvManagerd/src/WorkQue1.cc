/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: WorkQue1.cc,v 1.4.6.1 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <mtcout.h>
#include <WorkQue1.h>
#include <mgrApp.h>
#include <NewDataCommand.h>
#include <CommandQue.h>
#include <milog/milog.h>

using namespace std;
using namespace milog;

template<class CommandType>
WorkQue1<CommandType>::WorkQue1(ManagerApp &app_, 
				dnmi::thread::CommandQue &inputQue_,
				const std::string &idName_)
  :app(app_), inputQue(inputQue_), idName(idName_)
{
}

template <class CommandType> void 
WorkQue1<CommandType>::operator()()
{
  dnmi::thread::CommandBase *cmd; 
  CommandType *newCmd;

  LogContext context(idName);
  LOGINFO("Thread starting!");

  while(!app.shutdown()){
    cmd=inputQue.get();

    LOGDEBUG("Command received!");

    if(!cmd)
      continue;
    
    newCmd=dynamic_cast<CommandType*>(cmd);

    if(!newCmd){
      LOGDEBUG("Unknown Command.(inputQue)");
      delete cmd;
    }

    newCmd->execute();
    
    delete newCmd;
  }
  
  LOGINFO("Thread terminating!");
  
}

