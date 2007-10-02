/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SubscriberThread.h,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#define __SubscriberThread_h__

#include <boost/thread/mutex.hpp>
#include <kvskel/kvService.hh>
#include <dnmithread/Thread.h>
#include <dnmithread/CommandQue.h>
#include <milog/milog.h>
#include <db/db.h>
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
   boost::mutex mutex_;
            
   int runPersistent();
   dnmi::db::Connection* getDbConnection();
      
 	public:
 		SubscriberThread(
 				CKvalObs::CService::StatusId           status,
				const KvDataSubscriberInfo::TQc       &qc,
  				const KvDataSubscriberInfo::TStations &stations,
  				ServiceApp                             &app ,
		   	typename cmdType::_ptr_type           subscriber,
		      bool persistent=false,
		      const std::string &subscriberid="")
    	:KvDataSubscriberInfo(status, qc, stations, app, subscriberid, persistent), 
    	 subscriber_(cmdType::_interface::_duplicate(subscriber)) 
    	{
    	}
    	
  		SubscriberThread(const CKvalObs::CService::DataSubscribeInfo &info,
		   				  typename cmdType::_ptr_type  subscriber,
		                 ServiceApp &app,
		                 bool persistent=false,
		                 const std::string &subscriberid="")
    	:KvDataSubscriberInfo(info, app, subscriberid, persistent), 
    	 subscriber_(cmdType::_interface::_duplicate(subscriber)) 
    	{
    	}

	SubscriberThread(typename cmdType::_ptr_type  subscriber,
		              ServiceApp &app,
		              bool persistent=false,
		              const std::string &subscriberid="")
    	:KvDataSubscriberInfo(persistent, subscriberid, app), 
    	 subscriber_(cmdType::_interface::_duplicate(subscriber)) 
    	{
    	}


  	~SubscriberThread(){
   	 LOGINFO("DELETE: SubscriberThread: subscriberid: " << subscriberid());
  	}
  	
  	void setLogger(const std::string &name);
  
  	void
  	subscriber(typename cmdType::_ptr_type subscriber ){
  			boost::mutex::scoped_lock lock(mutex_);
  			subscriber_=cmdType::_interface::_duplicate(subscriber);
  	}
  	
  	typename cmdType::_ptr_type
   subscriber(){
   	 	boost::mutex::scoped_lock lock(mutex_);
      	return cmdType::_interface::_duplicate(subscriber_);
   }

	void tryToSendSavedData(bool &moredata);

	void saveDataInfo(cmdType *cmd, bool &moredata);

	bool sendPsData(cmdType &cmd, bool &moredata,
           			  dnmi::db::Connection *con);


  	int run();
  	
};

#include "SubscriberThread.tcc"

#endif
