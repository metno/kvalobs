/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: managerInputImpl.cc,v 1.2.2.2 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <dnmithread/mtcout.h>
#include <puTools/miTime.h>
#include "managerInputImpl.h"
#include <kvalobs/kvStationInfoCommand.h>
#include <milog/milog.h>

using namespace miutil;
using namespace std;
using namespace milog;

ManagerInputImpl::ManagerInputImpl(ManagerApp &app_,
				   dnmi::thread::CommandQue &que_)
  :app(app_), que(que_)
{
}

ManagerInputImpl::~ManagerInputImpl()
{
}


/**
 * newData receives data from the kvDataInputd and post it on the 'que'
 * the PreProcessWorker listen on the que.
 */
CORBA::Boolean 
ManagerInputImpl::newData(const CKvalObs::StationInfoList& infoList)
{
  kvalobs::StationInfoCommand *cmd=0;
  
  LogContext context("ManagerInputImpl::NewData");
  LOGDEBUG("New data from kvDataInputd!");
 
  try{
    cmd=new kvalobs::StationInfoCommand(infoList);
    que.postAndBrodcast(cmd);
    return true;
  }catch(...){
    if(cmd){
      LOGERROR("Can post the data to the que! (NOMEM?)");
      delete cmd;
    }else{
      LOGFATAL("OUT OF MEMORY!");
    }
    return false;
  }

}




