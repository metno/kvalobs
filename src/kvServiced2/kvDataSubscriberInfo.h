/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvDataSubscriberInfo.h,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#ifndef __kvDataSubscriberInfo_h__
#define __kvDataSubscriberInfo_h__

#include <iostream>
#include <string>
#include <vector>
#include <kvskel/kvService.hh>
#include <dnmithread/CommandQue.h>
#include <kvdb/kvdb.h>
#include "kvSubscriberBase.h"
#include "DataToSubscribers.h"

class ServiceApp;
//class SubscriberCommandBase;

class KvDataSubscriberInfo : public KvSubscriberBase {

 public:
  const static unsigned char QC1_mask;
  const static unsigned char QC2d_mask;
  const static unsigned char QC2m_mask;
  const static unsigned char HQC_mask;

  enum {
    RETRY = 60,
    MAX_RETRY = 3600
  } RetryConst;
  //enum {RETRY=60, MAX_RETRY=120} RetryConst;

  typedef std::vector<int> TStations;
  typedef std::vector<int>::iterator ITStations;
  typedef std::vector<int>::const_iterator CITStations;

  typedef std::vector<CKvalObs::CService::QcId> TQc;
  typedef std::vector<CKvalObs::CService::QcId>::iterator ITQc;
  typedef std::vector<CKvalObs::CService::QcId>::const_iterator CITQc;

 private:
  KvDataSubscriberInfo();
  KvDataSubscriberInfo(const KvDataSubscriberInfo &info);
  KvDataSubscriberInfo& operator=(const KvDataSubscriberInfo &);

  //Definitions from kvService.hh (kvService.idl)
  //enum StatusId { All, OnlyFailed, OnlyOk };
  //enum QcId { QC1, QC2d, QC2m, HQC };
  CKvalObs::CService::StatusId status_;
  TQc qc_;
  TStations stations_;

 protected:
  bool terminate_;
  ServiceApp &app_;
  dnmi::thread::CommandQue inque_;

  //This is used for retry and timeout controll in communication 
  //with susbscribers.
  time_t retry;
  time_t maxRetry;
  time_t tick;

  void setretry();
  dnmi::thread::CommandBase* getDataFromQue(int timeout);
  bool dataQueEmpty() {
    return inque_.empty();
  }

 public:
  KvDataSubscriberInfo(CKvalObs::CService::StatusId status,
                       const KvDataSubscriberInfo::TQc &qc,
                       const KvDataSubscriberInfo::TStations &stations,
                       ServiceApp &app, const std::string &subscriberid,
                       bool persistent);

  KvDataSubscriberInfo(const CKvalObs::CService::DataSubscribeInfo &info,
                       ServiceApp &app, const std::string &subscriberid = "",
                       bool persistent = false);
  KvDataSubscriberInfo(bool persistent, const std::string &subscriberid,
                       ServiceApp &app);

  ~KvDataSubscriberInfo() {
  }

  ServiceApp& app() {
    return app_;
  }

  void terminate() {
    terminate_ = true;
  }
  bool terminated() const {
    return terminate_;
  }
  bool shutdown();
  void put(dnmi::thread::CommandBase *cmd);

  CKvalObs::CService::StatusId status() const {
    return status_;
  }

  bool qcAll() const {
    return qc_.size() == 0;
  }
  bool hasQc(CKvalObs::CService::QcId qc) const;
  TQc qc() const {
    return qc_;
  }
  ITQc qcbegin() {
    return qc_.begin();
  }
  CITQc qcbegin() const {
    return qc_.begin();
  }
  ITQc qcend() {
    return qc_.end();
  }
  CITQc qcend() const {
    return qc_.end();
  }

  bool thisStation(int stationid) const;

  ITStations stbegin() {
    return stations_.begin();
  }
  CITStations stbegin() const {
    return stations_.begin();
  }
  ITStations stend() {
    return stations_.end();
  }
  CITStations stend() const {
    return stations_.end();
  }

  bool checkStatusAndQc(DataToSubscribersPtr data);
  bool fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag);

  dnmi::db::Connection* getNewDbConnection();
  void releaseDbConnection(dnmi::db::Connection *con);

  bool savePsSubscriberSIOR(dnmi::db::Connection *dbCon,
                            const std::string &subscriberid,
                            const std::string &sior);

  std::string kvpath() const;

  friend std::ostream& operator<<(std::ostream& os,
                                  const KvDataSubscriberInfo &c);
};

std::ostream& operator<<(std::ostream& os, const KvDataSubscriberInfo &c);

#endif
