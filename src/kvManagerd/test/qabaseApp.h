/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: qabaseApp.h,v 1.2.6.2 2007/09/27 09:02:36 paule Exp $                                                       

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
#ifndef __mgrApp_h__
#define __mgrApp_h__

#include <kvskel/managerInput.hh>
#include <kvalobs/kvStationInfo.h>
#include <dnmithread/CommandQue.h>
#include <kvalobs/kvapp.h>

class QaBaseApp : public KvApp {
  QaBaseApp();  //No implementation

  dnmi::thread::CommandQue inQue;
  bool shutdown_;
  CKvalObs::CManager::CheckedInput_var refManager;

 public:
  QaBaseApp(int argn, char **argv, const char *options[][2] = 0);
  virtual ~QaBaseApp();

  //inherited from KvApp
  virtual bool isOk() const;

  /**
   * shutdown, use this when the application shall terminate.
   */
  void shutdown() {
    shutdown_ = true;
  }

  /**
   * inShutdown, returns true when the application is in 
   * the terminating state. The calling thread shall do
   * finish its work and terminate.
   */
  bool inShutdown() const {
    return shutdown_;
  }

  dnmi::thread::CommandQue& getInQue() {
    return inQue;
  }

  /**
   * Used to return the result from the checks back to the kvManagerd.
   */
  bool sendToManager(kvalobs::kvStationInfoList &retList,
                     CKvalObs::CManager::CheckedInput_ptr manager);
};

#endif
