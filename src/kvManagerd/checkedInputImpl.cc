/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: checkedInputImpl.cc,v 1.2.2.2 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <dnmithread/CommandQue.h>
#include <milog/milog.h>
#include "CheckedDataCommand.h"
#include "checkedInputImpl.h"
#include "mgrApp.h"

using namespace kvalobs;

CheckedInputImpl::~CheckedInputImpl() {
}

CheckedInputImpl::CheckedInputImpl(ManagerApp &app_,
                                   dnmi::thread::CommandQue &que_)
    : app(app_),
      que(que_) {
}

CORBA::Boolean CheckedInputImpl::checkedData(
    const CKvalObs::StationInfoList& data) {
  CheckedDataCommand *cmd;

  milog::LogContext logContext("CheckedInputImpl");

  LOGDEBUG("New data from kvQabased. Size=" << data.length());

  try {
    cmd = new CheckedDataCommand(data);
  } catch (...) {
    return false;
  }

  try {
    que.postAndBrodcast(cmd);
  } catch (...) {
    delete cmd;
    return false;
  }

  return true;
}
