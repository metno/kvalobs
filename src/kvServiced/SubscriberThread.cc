/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SubscriberThread.cc,v 1.1.2.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#include "SubscriberThread.h"
#include <dnmithread/CommandQue.h>
#include "ServiceApp.h"



#if 0
void 
SubscriberThread::
callDataNotifySubscribers(const kvalobs::kvStationInfo &stationInfo)
{
}

void 
SubscriberThread::
callDataSubscribers(const kvalobs::kvStationInfo &stationInfo)
{
}

void 
SubscriberThread::
removeDeadConnections()
{
}
#endif


SubscriberThread::
SubscriberThread(const SubscriberThread &s):
  app(s.app), inputquePtr(s.inputquePtr), join_(s.join_)
{
}

SubscriberThread::
SubscriberThread(ServiceApp &app_, dnmi::thread::CommandQue *que_):
  app(app_), inputquePtr(que_), join_(new bool(false))
{
}  

SubscriberThread::
~SubscriberThread()
{
}


bool 
SubscriberThread::
start()
{
  boost::thread *th;

  try{
    th=new boost::thread(*this);
  }catch(...){
    return false;
  }
  
  thread.reset(th);
}

bool 
SubscriberThread::
join()
{
  return *join_;
}


void 
SubscriberThread::
operator()()
{
}

