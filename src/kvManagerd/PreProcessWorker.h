/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: PreProcessWorker.h,v 1.2.2.2 2007/09/27 09:02:35 paule Exp $                                                       

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
#ifndef __preprocessworker_h__
#define __preprocessworker_h__

#include "PreProcessJob.h"
#include "OneTimeJob.h"
#include <list>
#include <boost/shared_ptr.hpp>
#include "mgrApp.h"
#include <dnmithread/CommandQue.h>
#include <kvdb/kvdb.h>
#include <kvalobs/kvStationInfoCommand.h>

typedef boost::shared_ptr<PreProcessJob> PreProcessJobPtr;
typedef std::list<PreProcessJobPtr> PreProcessJobList;
typedef std::list<PreProcessJobPtr>::iterator IPreProcessJobList;
typedef std::list<PreProcessJobPtr>::const_iterator CIPreProcessJobList;

typedef boost::shared_ptr<OneTimeJob> OneTimeJobPtr;
typedef std::list<OneTimeJobPtr> OneTimeJobList;
typedef std::list<OneTimeJobPtr>::iterator IOneTimeJobList;
typedef std::list<OneTimeJobPtr>::const_iterator CIOneTimeJobList;

class PreProcessWorker {
  boost::shared_ptr<PreProcessJobList> jobList;
  boost::shared_ptr<OneTimeJobList> oneTimeJobList;
  ManagerApp &app;
  dnmi::thread::CommandQue &inputQue;
  dnmi::thread::CommandQue &outputQue;

  boost::mutex mutex;

  void postDataToOutputQue(kvalobs::StationInfoCommand *infoCmd,
                           dnmi::db::Connection &con);

 public:
  PreProcessWorker(ManagerApp &app, dnmi::thread::CommandQue &inputQue_,
                   dnmi::thread::CommandQue &outputQue_);
  PreProcessWorker(const PreProcessWorker &);
  ~PreProcessWorker();

  void addOneTimeJob(OneTimeJob *job);
  void addJob(PreProcessJob *job);
  void removeJob(PreProcessJob *job);
  void operator()();
};

#endif
