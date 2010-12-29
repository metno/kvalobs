/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceSubscriber.h,v 1.7.2.3 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __ServiceSubscriber_h__
#define __ServiceSubscriber_h__

#include <kvskel/commonStationInfo.hh>
#include <kvalobs/kvTextData.h>
#include <kvalobs/kvData.h>
#include <kvdb/kvdb.h>
#include "ServiceApp.h"
#include "kvDataNotifySubscriber.h"
#include "DataReadyCommand.h"

/**
 * One DataToSend record contains data for one obtime and stationid. 
 */
struct DataToSend{
private:

public:

  std::list<kvalobs::kvData>       dataList;
  std::list<kvalobs::kvTextData>   textDataList;
  long                             stationid;
  
  DataToSend(){}
  DataToSend(const std::list<kvalobs::kvData> &dataList_,
	     const std::list<kvalobs::kvTextData> &textDataList_,
	     long  stationid_)
    :dataList(dataList_), 
       textDataList(textDataList_),
       stationid(stationid_)
    {};
  
  DataToSend(const DataToSend &d)
    :dataList(d.dataList), textDataList(d.textDataList)
    {};
  

  DataToSend& operator=(const DataToSend &rhs){
    if(&rhs!=this){
      dataList=rhs.dataList;
      textDataList=rhs.textDataList;
    }
    return *this;
  }


};

typedef std::list<DataToSend>                   DataToSendList;
typedef std::list<DataToSend>::iterator        IDataToSendList;
typedef std::list<DataToSend>::const_iterator CIDataToSendList;


/**
 * This class is used to send data to all DataNotifySubscribers
 * that is registred.
 */
class DataNotifyFunc : public DataNotifySubscriberFuncBase
{
  const kvalobs::kvStationInfoExt     &stationInfo;
  const std::list<kvalobs::kvData> &dataList;
 
  bool fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag);
  bool buildWhatList(CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl);
  bool checkStatusAndQc(KvDataNotifySubscriberPtr ptr);

public:
  DataNotifyFunc(const kvalobs::kvStationInfoExt &sti,
		 const std::list<kvalobs::kvData> &dataList_)
    :stationInfo(sti), dataList(dataList_)
    {}
  void func(KvDataNotifySubscriberPtr ptr);
};

class DataFunc : public DataSubscriberFuncBase
{
  CKvalObs::CService::ObsDataList data;
  
  bool fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag);
  bool checkStatusAndQc(KvDataSubscriberPtr ptr);

public:
  DataFunc(const DataToSendList &dataList);

  void func(KvDataSubscriberPtr ptr);
};



class ServiceSubscriber{
  ServiceSubscriber& operator=(const ServiceSubscriber &);
  
  ServiceApp               &app;
  dnmi::thread::CommandQue &inputque;
  dnmi::db::Connection     *dbCon;

  void callDataNotifySubscribers(const kvalobs::kvStationInfoExt &stationInfo,
                                 const std::string &logid);
  void callDataSubscribers(const kvalobs::kvStationInfoExt &stationInfo,
                           const std::string &logid);
  void removeDeadConnections();
  void updateWorkelementServiceStart(const kvalobs::kvStationInfoExt &st,
				     dnmi::db::Connection *con,
                 const std::string &logid);
  void updateWorkelementServiceStop(const kvalobs::kvStationInfoExt &st,
				    dnmi::db::Connection *con,
                const std::string &logid);

 public:
  ServiceSubscriber(const ServiceSubscriber &s);

  ServiceSubscriber(ServiceApp &app_,
		    dnmi::thread::CommandQue &que_);
  ~ServiceSubscriber();

  void operator()();
};

#endif
