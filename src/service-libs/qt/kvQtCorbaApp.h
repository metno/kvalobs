/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQtCorbaApp.h,v 1.10.2.3 2007/09/27 09:02:47 paule Exp $                                                       

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
#ifndef __kvQtCorbaApp_h__
#define __kvQtCorbaApp_h__

#include <list>
#include <string>
#include <kvskel/datasource.hh>
#include <kvskel/kvService.hh>
#include <kvalobs/kvapp.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvReferenceStation.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvOperator.h>
#include "WhichDataHelper.h"
#include "kvservicetypes.h"
#include "kvDataSubscribeInfoHelper.h"

namespace kvservice{
  namespace priv{

      class KvQtCorbaApp: public KvApp
      {
	  CKvalObs::CService::kvService_var refManager;
	  CKvalObs::CService::kvDataNotifySubscriber_var refDataNotifySubs; 
	  CKvalObs::CService::kvDataSubscriber_var       refDataSubs; 
	  CKvalObs::CService::kvHintSubscriber_var       refKvHintSubs;

	  void unsubscribe_(const std::string &id);
	  std::list<std::string> subscriberid;
	  
	  CKvalObs::CService::DataIterator_ptr 
	    getDataIter(const WhichDataHelper &wd);
	  
	  CKvalObs::CService::ModelDataIterator_ptr 
	    getModelDataIter(const WhichDataHelper &wd);

	  bool getParams(CKvalObs::CService::ParamList *& params);
	  bool getStations(CKvalObs::CService::StationList *&stations);
	  bool getReferenceStations(CKvalObs::CService::Reference_stationList 
				    *&ref,
				    long stationid, int paramsetid);
	  bool getTypes(CKvalObs::CService::TypeList *& types);

	  bool getObsPgm(CKvalObs::CService::Obs_pgmList *& obsPgm,
			 const std::list<long> &stationList,
			 bool aUnion);

	  bool getOperator(CKvalObs::CService::OperatorList *& operators);

	bool getStationParam( CKvalObs::CService::Station_paramList *& stParam,
			      int stationid, int paramid, int day );

      protected:
	  CKvalObs::CDataSource::Data_ptr dataInput;
	  bool connectToKvInput(bool reConnect = false);


      public:
	  KvQtCorbaApp(int argn, char **argv, const char *options[0][2]=0);
	  ~KvQtCorbaApp();
	  
	  /**
	   *\exception throws LookUpException.
	   */  
	  CKvalObs::CService::kvService_ptr lookUpManager(bool forceNS,
							  bool & usedNS);
	  CKvalObs::CService::kvDataNotifySubscriber_ptr
	  getDataNotifySubscriber();
	  
	  void setDataNotifySubscriber(
	      CKvalObs::CService::kvDataNotifySubscriber_ptr ptr
	      )
	      { refDataNotifySubs=ptr;}

	  void setDataSubscriber(
	      CKvalObs::CService::kvDataSubscriber_ptr ptr
	      )
	      { refDataSubs=ptr;}

	  void setKvHintSubscriber(
	      CKvalObs::CService::kvHintSubscriber_ptr ptr
	      )
	      { refKvHintSubs=ptr;}


	  
	  std::string 
	    subscribeDataNotify(const KvDataSubscribeInfoHelper &info);

	  std::string 
	    subscribeData(const KvDataSubscribeInfoHelper &info);
	  
	  std::string 
	    subscribeKvHint();
	  
	  void unsubscribe(const std::string &subscribeid);
	  
	  void unsubscribeAll();

                       
	  bool getKvData(KvObsDataList &dataList,
			 const WhichDataHelper &wd);      
	  bool getKvParams(std::list<kvalobs::kvParam> &paramList);
	  bool getKvStations(std::list<kvalobs::kvStation> &stationList);
	  bool getKvModelData(std::list<kvalobs::kvModelData> &dataList,
			      const WhichDataHelper &wd);
	  bool getKvReferenceStations(int stationid, 
				      int paramid, 
				      std::list<kvalobs::kvReferenceStation> &refList); 
	  bool getKvTypes(std::list<kvalobs::kvTypes> &typeList);

	  bool getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
			   const std::list<long> &stationList,
			   bool aUnion);

	bool getKvOperator(std::list<kvalobs::kvOperator> &operatorList);
	  
	bool getKvStationParam( std::list<kvalobs::kvStationParam> &stParam,
				int stationid, int paramid = -1, int day = -1 );

	  const CKvalObs::CDataSource::Result_var
	    sendDataToKv(const char *data, const char *obsType);
      };
  }
}

#endif
