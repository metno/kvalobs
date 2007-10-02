/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataNotifySubscriber.h,v 1.1.2.4 2007/09/27 09:02:40 paule Exp $                                                       

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
#ifndef __kvDataNotifySubscriber_ijk_h__
#define __kvDataNotifySubscriber_ijk_h__

//#include <kvskel/kvServiceCommon.hh>
#include <kvskel/kvService.hh>
#include <dnmithread/Thread.h>
#include <dnmithread/CommandQue.h>
#include "kvDataSubscriberInfo.h"
#include "DataCommand.h"
/**
 * KvDataSubscriber holds the information from a data subcriber.
 */ 

class ServiceApp;

class KvDataNotifySubscriber: 
  public KvDataSubscriberInfo, 
  public dnmi::thread::Runable
{
  CKvalObs::CService::kvDataNotifySubscriber_var subscriber_;

  void buildWhatList(CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl,
		     DataToSubscribersPtr d);


 public:
  KvDataNotifySubscriber(const CKvalObs::CService::DataSubscribeInfo &info,
		   CKvalObs::CService::kvDataNotifySubscriber_ptr subscriber,
		   ServiceApp &app)
    :KvDataSubscriberInfo(info, app), 
     subscriber_(subscriber)
    {
    }

  ~KvDataNotifySubscriber();

  
  CKvalObs::CService::kvDataNotifySubscriber_ptr
    subscriber()
    { 
      return CKvalObs::CService::kvDataNotifySubscriber::_duplicate(subscriber_);
    }


  void put(DataCommand *cmd);
  
  int run();
};

#endif
