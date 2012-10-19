/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataCommand.h,v 1.1.2.2 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef __DataCommand_h__
#define __DataCommand_h__

#include <dnmithread/CommandQue.h>
#include <kvskel/kvDataSubscriber.hh>
#include "SubscriberCommandBase.h"
#include "DataToSubscribers.h"

namespace kvskel=CKvalObs::CService;

class DataCommand : 
  public SubscriberCommandBase<kvskel::kvDataSubscriber>
{
 protected:
  DataCommand();
  DataCommand(const DataCommand &);
  DataCommand& operator=(const DataCommand &);

  class DataHelper{
    DataHelper();
    DataHelper(const DataHelper &);
    DataHelper& operator=(const DataHelper &);
    
  public:
    DataHelper(long sid, int tid, miutil::miTime obst):
      stationid(sid), typeid_(tid), obstime(obst){}
    
    kvskel::ObsDataList data;
    long stationid;          //stationid to the data.
    int  typeid_;            //typeid to the data.
    miutil::miTime obstime;  //obstime to the data.
  };

  //kvskel::kvDataSubscriber_ptr subscriber;

  DataHelper* obsDataList(DataToSubscribersPtr dp);

 public:
  DataCommand(DataToSubscribersPtr data2sub)
    :SubscriberCommandBase<kvskel::kvDataSubscriber>(data2sub){}

  ~DataCommand(){};


  //  int operator()(KvDataSubscriberInfo &sinf,
  //		 CKvalObs::CService::kvDataSubscriber_ptr sub){
  //  subscriber=sub;
  //  return execute(sinf);
  //}

  int execute(KvDataSubscriberInfo &sinf);
};


#endif
