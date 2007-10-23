/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceSubscriber.h,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#ifndef __ServiceSubscriber_h__
#define __ServiceSubscriber_h__

#include <kvskel/commonStationInfo.hh>
#include <kvalobs/kvTextData.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStationInfo.h>
#include <kvdb/kvdb.h>
#include "ServiceApp.h"
//#include "kvDataNotifySubscriber.h"
#include "DataReadyCommand.h"


class ServiceSubscriber{
  ServiceSubscriber& operator=(const ServiceSubscriber &);
  
  ServiceApp               &app;
  dnmi::thread::CommandQue &inputque;
  dnmi::db::Connection     *dbCon;

  void callSubscribers(const kvalobs::kvStationInfo &stationInfo);
  void updateWorkelementServiceStart(const kvalobs::kvStationInfo &st,
				     dnmi::db::Connection *con);
  void updateWorkelementServiceStop(const kvalobs::kvStationInfo &st,
				    dnmi::db::Connection *con);

 public:
  ServiceSubscriber(const ServiceSubscriber &s);

  ServiceSubscriber(ServiceApp &app_,
		    dnmi::thread::CommandQue &que_);
  ~ServiceSubscriber();

  void operator()();
};

#endif
