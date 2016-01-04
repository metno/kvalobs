/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: DataNotifyCommand.h,v 1.1.2.2 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef __DataNotifyCommand_h__
#define __DataNotifyCommand_h__

#include <dnmithread/CommandQue.h>
#include <kvskel/kvDataNotifySubscriber.hh>
#include "SubscriberCommandBase.h"
#include "DataToSubscribers.h"

namespace kvskel = CKvalObs::CService;

class DataNotifyCommand : public SubscriberCommandBase<
    kvskel::kvDataNotifySubscriber> {

 protected:
  DataNotifyCommand(const DataNotifyCommand &);
  DataNotifyCommand& operator=(const DataNotifyCommand &);

  void buildWhatList(CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl,
                     DataToSubscribersPtr dp);

 public:
  DataNotifyCommand(DataToSubscribersPtr data2sub)
      : SubscriberCommandBase<kvskel::kvDataNotifySubscriber>(data2sub) {
  }

  ~DataNotifyCommand() {
  }
  ;

  //  int operator()(KvDataSubscriberInfo &sinf,
  //		 CKvalObs::CService::kvDataNotifySubscriber_ptr sub){
  //  subscriber=sub;
  //  return execute(sinf);
  //}

  int execute(KvDataSubscriberInfo &sinf);

};

#endif
