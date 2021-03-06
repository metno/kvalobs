/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: ServiceApp.h,v 1.2.2.4 2007/09/27 09:02:39 paule Exp $                                                       

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

#include <milog/milog.h>
#include <kvalobs/kvapp.h>
#include <kvalobs/kvStationInfo.h>
#include <kvskel/qabase.hh>
#include "kvSubscriberCollection.h"
#include <kvalobs/kvStation.h>
#include <kvdb/dbdrivermgr.h>

class ReaperBase;

class ServiceApp : public KvApp {
  enum {
    MAX_CLIENTS = 10000
  };
  std::string dbDriver;
  std::string dbConnect;
  std::string dbDriverId;
  bool orbIsDown;
  std::list<ReaperBase*> reaperObjList;
  bool shutdown_;
  mutable boost::mutex mutex;
  mutable boost::mutex reaperMutex;
 public:
  KvSubscriberCollection subscribers;

  ServiceApp(int argn, char **argv, const std::string &driver,
             const std::string &connect, const char *options[][2] = 0);

  virtual ~ServiceApp();

  /**
   * \brief create a globale logger with id \a id.
   *
   * \param is An id to identify the logger.
   * \return true on success and false otherwise.
   */
  bool createGlobalLogger(std::string &id, milog::LogLevel ll = milog::NOTSET);

  //inherited from KvApp
  virtual bool isOk() const;

  void doShutdown() {
    shutdown_ = true;
  }
  bool shutdown();

  bool sendToManager(kvalobs::kvStationInfoList &retList,
                     CKvalObs::CManager::CheckedInput_ptr callback);

  /**
   * Creates a new connection to the database. The caller must
   * call releaseDbConnection after use.
   */
  dnmi::db::Connection *getNewDbConnection();
  void releaseDbConnection(dnmi::db::Connection *con);

  void addReaperObj(ReaperBase *rb);
  void removeReaperObj(ReaperBase *rb);
  void cleanUpReaperObj();

  bool isMaxClientReached();

  kvalobs::kvStation lookUpStation(const std::string &kvQuerie);

  void notifyAllKvHintSubscribers();
};

#endif
