/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SubscriberData.h,v 1.1.2.7 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __SubscriberData_h__
#define __SubscriberData_h__

#include <boost/thread/thread.hpp>
#include <set>
#include <map>
#include <stdio.h>
#include <list>
#include <vector>
#include <fstream>
#include <puTools/miTime>
#include <puTools/miString>
#include <kvskel/kvService.hh>
#include <dnmithread/Thread.h>
#include "SubscriberThread.h"
//#include "kvDataNotifySubscriber.h"
//#include "kvDataSubscriber.h"
#include "DataToSubscribers.h"
#include "DataCommand.h"
#include "DataNotifyCommand.h"

typedef SubscriberThread<DataCommand> KvDataSubscriber;

typedef SubscriberThread<DataNotifyCommand> KvDataNotifySubscriber;


class SubscriberData
{
  typedef enum{ DataSub, DataNotifySub, HintSub} TSubType;

  typedef std::map<std::string, 
		   dnmi::thread::Thread<KvDataSubscriber> > 
                   TDataSub;
  typedef std::map<std::string, 
		   dnmi::thread::Thread<KvDataSubscriber> >::iterator 
                   ITDataSub;
  typedef std::map<std::string,
		   dnmi::thread::Thread<KvDataNotifySubscriber> >
                   TDataNotifySub;
  typedef std::map<std::string,
                   dnmi::thread::Thread<KvDataNotifySubscriber> >::iterator
                   ITDataNotifySub;
  typedef std::map<std::string,
                   CKvalObs::CService::kvHintSubscriber_var>
                   THintSub;
  typedef std::map<std::string,
                   CKvalObs::CService::kvHintSubscriber_var>::iterator
                   ITHintSub;

  THintSub       hintSubs;
  TDataSub       dataSubs;
  TDataNotifySub dataNotifySubs;

  std::string createSubscriberid(TSubType st);

  bool removeDataNotifySubscriber(const std::string &subscriberid);
  bool removeDataSubscriber(const std::string &subscriberid);
  bool removeHintSubscriber(const std::string &subscriberid);

  bool writeSubscriberFile(const std::string &subscriberid, 
			   const KvDataSubscriberInfo *si,
			   const std::string &corbaref);


  bool readSubscriberFile(const std::string &fname);
  bool updateSubscriberFile(const std::string &subscriberid,
			    const miutil::miTime &timeForLastCall);
  bool removeSubscriberFile(const std::string &subscriberid); 


    
  boost::mutex mutex;
  std::string subPath; //The path to the directory where we shall save
                       //subscriber information. 
                       //Default: $KAVLOBS/var/kvalobs/manager/subscriber 

 public:
  SubscriberData();
  SubscriberData(const std::string &subscriberpath);
  ~SubscriberData();

  std::string createDataSubscriber(KvDataSubscriber *subscriber);
  std::string createDataNotifySubscriber(KvDataNotifySubscriber *subscriber);
  std::string createHintSubscriber(CKvalObs::CService::kvHintSubscriber_ptr s);

  void removeSubscriber(const std::string &subscriberid);

  /**
   * \return return true if we have data or datanotify subscribers!
   */
  bool hasSubscribers();

  void readAllSubscribersFromFile();
  void sendKvHintUp();
  void sendKvHintDown();
  void forAllSubscribers(DataToSubscribersPtr data2sub);
  void removeTerminatedSubscribers();
};

#endif
