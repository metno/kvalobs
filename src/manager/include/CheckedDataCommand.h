/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CheckedDataCommand.h,v 1.7.6.1 2007/09/27 09:02:35 paule Exp $                                                       

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
#ifndef __CheckedDataCommand_h__
#define __CheckedDataCommand_h__

#include <commonStationInfo.hh>
#include <kvStationInfoCommand.h>
#include <mgrApp.h>
#include <kvDataNotifySubscriber.h>
#include <db.h>


/**
 * This class is used to send data to all DataNotifySubscribers
 * that is registred.
 */
class DataNotifyFunc : public DataNotifySubscriberFuncBase
{
  kvalobs::kvStationInfo &stationInfo;
 
  bool fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag);
  bool buildWhatList(CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl);
  bool checkStatusAndQc(KvDataNotifySubscriberPtr ptr);

public:
  DataNotifyFunc(kvalobs::kvStationInfo &sti):stationInfo(sti){}
  void func(KvDataNotifySubscriberPtr ptr);
};

class DataFunc : public DataSubscriberFuncBase
{
  std::list<kvalobs::kvData>                 dataList;
  
  bool fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag);
  bool checkStatusAndQc(KvDataSubscriberPtr ptr);
  bool fillDataList(CKvalObs::CService::ObsDataList &dl);

public:
  DataFunc(std::list<kvalobs::kvData> &dataList_):dataList(dataList_){}

  void func(KvDataSubscriberPtr ptr);
};



class CheckedDataCommand : public kvalobs::StationInfoCommand{
  CheckedDataCommand();
  CheckedDataCommand(const CheckedDataCommand &);
  CheckedDataCommand& operator=(const CheckedDataCommand &);
  
  ManagerApp           &app;
  dnmi::db::Connection *dbCon; //We shall not delete dbCon.
                               //It is owned by KvCheckedDataThread

  void callDataNotifySubscribers(kvalobs::kvStationInfoList &stationInfoList);
  void callDataSubscribers(kvalobs::kvStationInfoList &stationInfoList);

 public:
  CheckedDataCommand(const CKvalObs::StationInfoList &stInfo, 
		     ManagerApp &app_);
  ~CheckedDataCommand(){};

  void       setDbCon(dnmi::db::Connection *con){dbCon=con;}
  bool       executeImpl(); 

  void       debugInfo(std::iostream &info);
};

#endif
