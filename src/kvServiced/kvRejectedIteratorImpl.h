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
#ifndef __kvRejectedIteratorImpl_h__
#define __kvRejectedIteratorImpl_h__

#include <time.h>
#include <list>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <kvskel/kvService.hh>
#include <kvdb/kvdb.h>
#include <kvalobs/kvRejectdecode.h>
#include "ServiceApp.h"
#include "ReaperBase.h"

class RejectedIteratorImpl :
  public POA_CKvalObs::CService::RejectedIterator,
  public PortableServer::RefCountServantBase,
  public ReaperBase
{
  
  dnmi::db::Connection                         *dbCon;
  std::list<kvalobs::kvRejectdecode>           dataList;
  std::list<kvalobs::kvRejectdecode>::iterator dataIt;
  miutil::miTime                               currentEnd;
  miutil::miTime                               toTime;
  std::list<std::string>                       decodeList;
  ServiceApp                                   &app;

void
  fillData(std::list<kvalobs::kvRejectdecode> &toData,
	   const std::list<kvalobs::kvRejectdecode> &fromData,
	   std::list<kvalobs::kvRejectdecode>::iterator &fromDataIt);
  
  bool findData(std::list<kvalobs::kvRejectdecode> &data);
  
  
public:
  // standard constructor
  RejectedIteratorImpl(dnmi::db::Connection         *dbCon,
		       const miutil::miTime         &fromTime,
		       const miutil::miTime         &toTime_,
		       const std::list<std::string> &decodeList_,
		       ServiceApp                   &app_);
  virtual ~RejectedIteratorImpl();

                                                                               
  // methods corresponding to defined IDL attributes and operations
  void destroy();
  CORBA::Boolean 
    next(CKvalObs::CService::RejectdecodeList_out rejectedList_);
  
  virtual void addRef() { _add_ref(); }
  virtual void removeRef(){ _remove_ref(); }
};




#endif
