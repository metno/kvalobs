/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceApp.h,v 1.1.2.3 2007/09/27 09:02:22 paule Exp $                                                       

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
#ifndef __ServiceApp_h__
#define __ServiceApp_h__

#include <kvalobs/kvapp.h>
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvStation.h>
#include <kvskel/qabase.hh>
#include <kvdb/dbdrivermgr.h>
#include "SubscriberData.h"

class ReaperBase;

class ServiceApp: public KvApp
{
	enum { MAX_CLIENTS=10 };
  	dnmi::db::DriverManager            dbMgr;
  	std::string                        dbDriver;
  	std::string                        dbConnect;
  	std::string                        dbDriverId;
  	bool                               orbIsDown;
  	std::list<ReaperBase*>             reaperObjList;
  	bool shutdown_;
  	boost::mutex mutex;
  	boost::mutex reaperMutex;

public:
  	SubscriberData subscribers;

  	ServiceApp(int argn, char **argv, 
	     		  const std::string &driver,
	     		  const std::string &connect,
	     		  const char *options[][2]=0);

  	virtual ~ServiceApp();
  
  	//inherited from KvApp
  	virtual bool isOk()const;

  	void doShutdown(){ shutdown_=true;}
  	bool shutdown();

  	bool sendToManager(kvalobs::kvStationInfoList &retList,
		     				 CKvalObs::CManager::CheckedInput_ptr callback);


	void loadAllPersistentSubscribers();
	void removeAllUndefinedSubscribers();
	
	
	/**
	 * Save the CORBA stringified ior to the database for
	 * the subscriber given with subsscriberid.
	 */
	bool savePsSubscriberSIOR(dnmi::db::Connection *dbCon,
									  const std::string &subscriberid,
									  const std::string &sior);
	

  	/**
    * Creates a new connection to the database. The caller must
    * call releaseDbConnection after use.
    */
  	dnmi::db::Connection *getNewDbConnection();
  	void                 releaseDbConnection(dnmi::db::Connection *con);

  	void addReaperObj(ReaperBase *rb);
  	void cleanUpReaperObj();
	void removeReaperObj(ReaperBase *rb);
	bool isMaxClientReached();

  	kvalobs::kvStation lookUpStation(const miutil::std::string &kvQuerie);

  	void notifyAllKvHintSubscribers(bool kvup=true);
};




#endif
