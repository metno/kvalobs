/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SubscriberThread.h,v 1.1.2.4 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __SubscriberThread_h__
#define  __SubscriberThread_h__

#include <kvskel/kvService.hh>
#include <dnmithread/Thread.h>
#include <dnmithread/CommandQue.h>
#include <milog/milog.h>
#include "kvDataSubscriberInfo.h"

/**
 * SubscriberThread holds the information for a subcriber.
 *
 * cmdType must be a class derived from SubscriberCommandBase.
 */ 

class ServiceApp;

template <typename cmdType>
class SubscriberThread: 
  public KvDataSubscriberInfo, 
  public dnmi::thread::Runable
{

  typename cmdType::_var_type subscriber_;

 public:
  SubscriberThread(const CKvalObs::CService::DataSubscribeInfo &info,
		   typename cmdType::_ptr_type  subscriber,
		   ServiceApp &app)
    :KvDataSubscriberInfo(info, app), 
    subscriber_(subscriber)
    {
    }

  ~SubscriberThread(){
    LOGINFO("DELETE: SubscriberThread: subscriberid: " << subscriberid());
  }
  
  typename cmdType::_ptr_type
    subscriber()
    { 
      return cmdType::_interface::_duplicate(subscriber_);
    }

  int run();
};

#include "SubscriberThread.tcc"

#endif
