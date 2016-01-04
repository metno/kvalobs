/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: CheckForMissingObsMessages.h,v 1.2.2.6 2007/09/27 09:02:34 paule Exp $                                                       

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
#ifndef __CheckForMissingObsMessages_h__
#define __CheckForMissingObsMessages_h__

#include "mgrApp.h"
#include <dnmithread/CommandQue.h>
#include <kvdb/kvdb.h>

class CheckForMissingObsMessages {
  ManagerApp &app;
  dnmi::thread::CommandQue &outputQue;
  boost::posix_time::ptime lastMissingRunTime_;

  void findMissingData(const boost::posix_time::ptime& runtime,
                       dnmi::db::Connection *con);

  boost::posix_time::ptime lastMissingRuntime(dnmi::db::Connection *con);
  void lastMissingRuntime(
      dnmi::db::Connection *con,
      const boost::posix_time::ptime &newLastMissingRuntime);

 public:
  CheckForMissingObsMessages(ManagerApp &app,
                             dnmi::thread::CommandQue &outputQue_);
  CheckForMissingObsMessages(const CheckForMissingObsMessages&);
  ~CheckForMissingObsMessages();

  void operator()();
};

#endif
