/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvDataNotifySubscriberImpl.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include <milog/milog.h>
#include "../kvevents.h"
#include "kvDataNotifySubscriberImpl.h"

using namespace std;

namespace kvservice {
namespace priv {

DataNotifySubscriber::DataNotifySubscriber(dnmi::thread::CommandQue &que_)
    : que(que_) {
}

DataNotifySubscriber::~DataNotifySubscriber() {
}

void DataNotifySubscriber::callback(
    const CKvalObs::CService::kvDataNotifySubscriber::WhatList& what) {
  KvWhatList *whatList;
  DataNotifyEvent *dataNotifyEvent;
  KvWhatListPtr whatListPtr;

  LOGDEBUG("DataNotifySubscriber::callback: called!\n");

  if (what.length() <= 0) {
    LOGDEBUG("DataNotifySubscriber::callback: what.length()<=0!\n");
    return;
  }

  try {
    whatList = new KvWhatList();
    whatListPtr.reset(whatList);
  } catch (...) {
    LOGERROR("EXCEPTION: DataNotifySubscriber::callback: return!\n");
    return;
  }

  LOGDEBUG(
      "DataNotifySubscriber::callback what.length()=" << what.length() << endl);

  for (CORBA::ULong i = 0; i < what.length(); i++) {
    //CERR("DataNotifySubscriber::callback: what["<< i<<"].id.stationid=" 
    // <<what[i].id.stationid << endl); 
    whatListPtr->push_back(KvWhat(what[i]));
  }

  CERR(
      "DataNotifySubscriber::callback whatList->size()=" << whatListPtr->size() << endl);

  try {
    dataNotifyEvent = new DataNotifyEvent(whatListPtr);
    que.postAndBrodcast(dataNotifyEvent);
  } catch (...) {
    return;
  }
}

}
}

