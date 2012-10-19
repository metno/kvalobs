/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: OneTimeJob.h,v 1.2.2.2 2007/09/27 09:02:35 paule Exp $

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
#ifndef __OneTimeJob_h__
#define __OneTimeJob_h__

#include <puTools/miTime.h>
#include <kvdb/kvdb.h>
#include <kvalobs/kvStationInfo.h>
#include <dnmithread/CommandQue.h>

class ManagerApp;


class OneTimeJob
{
  friend class PreProcessWorker;

  ManagerApp                 *app_;
  dnmi::thread::CommandQue   *outputQue_;
  dnmi::db::Connection       *con_;

  void runJob(ManagerApp               *app,
	      dnmi::thread::CommandQue *outputQue,
	      dnmi::db::Connection     *con);



public:
    OneTimeJob():app_(0), outputQue_(0), con_(0){};
    virtual ~OneTimeJob(){}

    virtual const char* jobName()const =0;

    void postToQue(const kvalobs::kvStationInfo &info);

    virtual void doJob(dnmi::db::Connection &con)=0;
};



#endif
