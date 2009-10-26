/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: mgrApp.h,v 1.3.2.4 2007/09/27 09:02:35 paule Exp $                                                       

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
#ifndef __mgrApp_h__
#define __mgrApp_h__

#include <list>
#include <kvalobs/kvapp.h>
#include <kvalobs/kvStationInfo.h>
#include <kvskel/qabase.hh>
#include <kvalobs/kvStation.h>
#include <kvdb/dbdrivermgr.h>
#include <kvskel/kvService.hh>
#include "GenCacheElem.h"
#ifdef SMHI_LOG
#include <milog/milog.h>
#endif

class ManagerApp: public KvApp
{
  CKvalObs::CQaBase::QaBaseInput_var refQa;
  CKvalObs::CManager::CheckedInput_var refCheckedInput;
  CKvalObs::CManager::CheckedInput_var refServiceCheckedInput;
  CKvalObs::CService::DataReadyInput_var refKvServiceDataReady;
  dnmi::db::DriverManager            dbMgr;
  std::string                        dbDriver;
  std::string                        dbConnect;
  std::string                        dbDriverId;
  bool                               orbIsDown;

  /**
   * lookUpManager may throw exceptions.
   *
   * \exceptions LookUpException
   */
  CKvalObs::CQaBase::QaBaseInput_ptr lookUpQabase(bool forceNS, 
						  bool &usedNS);
  CKvalObs::CService::DataReadyInput_ptr lookUpKvService(bool forceNS, 
						     bool &usedNS);

  
  bool shutdown_;

  bool ok_; //a flag used to flag that we are ok. Used in AdminImpl::ping
  std::string status_; //The message to return in  AdminImpl::statusmessage
  bool checkForMissingObs_;
	
  GenCache genCache_;
	
  boost::mutex mutex;

 public:
  ManagerApp(int argn, char **argv, 
	     const std::string &driver,
	     const std::string &connect,
	     const char *options[][2]=0);

  virtual ~ManagerApp();
#ifdef SMHI_LOG
  milog::LogLevel     traceLevel;
  milog::LogLevel     logLevel;   
#endif
  //inherited from KvApp
  virtual bool isOk()const;

  void setCheckedInput(CKvalObs::CManager::CheckedInput_ptr ptr);
  void setServiceCheckedInput(CKvalObs::CManager::CheckedInput_ptr ptr);

  bool sendDataToQA(const kvalobs::kvStationInfo &info,
		    bool &bussy);
  bool sendDataToKvService(const kvalobs::kvStationInfoList &info_,
			   bool &bussy);


  bool checkForMissingObs()const { return checkForMissingObs_;}
  void checkForMissingObs(bool f){checkForMissingObs_=f;}

  /**
   * \brief Check if the stationid/typeid is generated "generated" data.
   *
   * Generated data is data that shuol have a negativ typeid.
   *
   * \param stationid the stationid to check.
   * \param typeid the typeid to check.
   * \return true if the stationid/typeid combination is generated data, false
   *              otherwise.
   */
  bool isGenerated(long stationid, long typeid_, dnmi::db::Connection *con);

  GenCache* genCache(){ return &genCache_;}

  void doShutdown(){ shutdown_=true;}
  bool shutdown();

  /**
   * Creates a new connection to the database. The caller must
   * call releaseDbConnection after use.
   */
  dnmi::db::Connection *getNewDbConnection();
  void                 releaseDbConnection(dnmi::db::Connection *con);

  kvalobs::kvStation lookUpStation(const miutil::miString &kvQuerie);

  void ok(bool f);
  bool ok()const;
  std::string status()const;
  void status(const std::string &m);
};




#endif
