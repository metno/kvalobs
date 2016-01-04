/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvRejectedIteratorImpl.h,v 1.2.2.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __WorkstatistikImpl_h__
#define __WorkstatistikImpl_h__

#include <time.h>
#include <list>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <kvskel/kvService.hh>
#include <kvdb/kvdb.h>
#include <kvalobs/kvWorkelement.h>
#include "ServiceApp.h"
#include "ReaperBase.h"

class WorkstatistikIteratorImpl :
    public POA_CKvalObs::CService::WorkstatistikIterator,
    public PortableServer::RefCountServantBase, public ReaperBase {
  mutable boost::mutex mutex;
  dnmi::db::Connection *dbCon;
  std::list<kvalobs::kvWorkelement> dataList;
  std::list<kvalobs::kvWorkelement>::iterator dataIt;
  miutil::miTime currentEnd;
  miutil::miTime toTime;
  std::string timeType;
  ServiceApp &app;

  void
  fillData(std::list<kvalobs::kvWorkelement> &toData,
           const std::list<kvalobs::kvWorkelement> &fromData,
           std::list<kvalobs::kvWorkelement>::iterator &fromDataIt);

  bool findData(std::list<kvalobs::kvWorkelement> &data);

 public:
  // standard constructor
  WorkstatistikIteratorImpl(dnmi::db::Connection *dbCon,
                            const miutil::miTime &fromTime,
                            const miutil::miTime &toTime_,
                            CKvalObs::CService::WorkstatistikTimeType timeType,
                            ServiceApp &app_);
  virtual ~WorkstatistikIteratorImpl();

  // methods corresponding to defined IDL attributes and operations
  void destroy();
  CORBA::Boolean
  next(CKvalObs::CService::WorkstatistikElemList_out dataList_);

  ///Overrided from reaperBase.
  virtual void cleanUp();

  virtual void addRef() {
    _add_ref();
  }
  virtual void removeRef() {
    _remove_ref();
  }
};

#endif
