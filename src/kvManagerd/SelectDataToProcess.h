/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: SelectDataToProcess.h,v 1.2.2.2 2007/09/27 09:02:35 paule Exp $                                                       

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
#ifndef __SelectDataToProcess_h__
#define __SelectDataToProcess_h__

#include <list>
#include <boost/shared_ptr.hpp>
#include "mgrApp.h"
#include <dnmithread/CommandQue.h>
#include <kvdb/kvdb.h>
#include <kvalobs/kvWorkelement.h>
#include <kvalobs/kvDbGate.h>

/**
 * SelectDataToProcess get messages from inputQue. When a message 
 * arrive we know that new messages is put into the table 'workque'.
 * We selects job from the workque to be prosecced. The data is sendt to 
 * the outputque to be processed.
 */
class SelectDataToProcess {
  ManagerApp &app;
  dnmi::thread::CommandQue &inputQue;
  dnmi::thread::CommandQue &outputQue;
  bool lowpri;
  /**
   * workInProcessQue brukes for � overv�ke antall jobber
   * som venter p� � bli sendt til qaBase. Vi pr�ver og
   * holde denne k�en p� en rimelig st�rrelse slik at systemet
   * ikke blir overbelastet.
   */
  dnmi::thread::CommandQue &workInProcessQue;
  dnmi::db::Connection *dbCon;

  boost::mutex mutex;
  int iRange;

  dnmi::db::Connection* newConnection();

  void processElements(std::list<kvalobs::kvWorkelement> &workList,
                       dnmi::db::Connection *con);
  bool processLowPriData(const boost::posix_time::ptime &baseTime);

 public:
  SelectDataToProcess(ManagerApp &app, dnmi::thread::CommandQue &inputQue_,
                      dnmi::thread::CommandQue &outputQue_,
                      dnmi::thread::CommandQue &workInProcessQue);
  SelectDataToProcess(const SelectDataToProcess &);
  ~SelectDataToProcess();

  void operator()();
};

#endif
