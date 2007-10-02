/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvCorbaApp.h,v 1.6.2.3 2007/09/27 09:02:44 paule Exp $                                                       

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
#ifndef __kvCorbaApp_h__
#define __kvCorbaApp_h__

#include <string>
#include <kvskel/kvService.hh>
#include <kvalobs/kvapp.h>
#include <list>
#include "kvDataSubscribeInfoHelper.h"
#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvReferenceStation.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvOperator.h>
#include "WhichDataHelper.h"
#include "kvservicetypes.h"
#include "kvDataNotifySubscriberImpl.h"
#include "kvDataSubscriberImpl.h"
#include "kvHintSubscriberImpl.h"
#include <dnmithread/CommandQue.h>
#include "KvGetDataReceiver.h"

#include "RejectdecodeIterator.h"

namespace kvservice{
  namespace priv{
    
    using namespace CKvalObs::CService;
    using namespace kvservice::priv;


    typedef std::map<std::string,DataNotifySubscriber*> 
            DataNotifyList;
    typedef std::map<std::string,DataNotifySubscriber*>::iterator 
            IDataNotifyList;
    typedef std::map<std::string,DataNotifySubscriber*>::const_iterator 
            CIDataNotifyList; 

    typedef std::map<std::string, DataSubscriber*> 
            DataList;
    typedef std::map<std::string, DataSubscriber*>::iterator 
            IDataList;
    typedef std::map<std::string, DataSubscriber*>::const_iterator 
            CIDataList;

    typedef std::map<std::string, HintSubscriber*> 
            HintList;
    typedef std::map<std::string, HintSubscriber*>::iterator 
            IHintList;
    typedef std::map<std::string, HintSubscriber*>::const_iterator 
            CIHintList;

    typedef bool (*termfunc)();

    class KvCorbaApp : public CorbaHelper::CorbaApp
    {
      kvService_var  refService;
      DataNotifyList dataNotifySubs; 
      DataList       dataSubs; 
      HintList       hintSubs;
      std::string    nameserver;
      std::string    nameserverpath;

      omni_mutex mutex;

      void unsubscribe_(const std::string &id);
      
      DataIterator_ptr getDataIter(const WhichDataHelper &wd);
      
      ModelDataIterator_ptr getModelDataIter(const WhichDataHelper &wd);

      bool getRejectdecode( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, 
			    CKvalObs::CService::RejectedIterator_ptr &it ); 
      
      bool getParams(CKvalObs::CService::ParamList *& params);
      bool getStations(CKvalObs::CService::StationList *&stations);
      bool getReferenceStations(CKvalObs::CService::Reference_stationList 
				*&ref,
				    long stationid, int paramsetid);
      bool getTypes(CKvalObs::CService::TypeList *& types);
      bool getOperator(CKvalObs::CService::OperatorList *& operators);
      bool getObsPgm(CKvalObs::CService::Obs_pgmList *& obsPgm,
		     const std::list<long> &stationList,
		     bool aUnion);
      bool getStationParam( CKvalObs::CService::Station_paramList *& stParam,
			    int stationid, int paramid, int day );

      public:
      KvCorbaApp(int argn, 
		 char **argv, 
		 miutil::conf::ConfSection *conf,
		 const char *options[][2]=0);
      ~KvCorbaApp();

      std::string corbanameserver()const { return nameserver;}
      std::string kvpathInCorbaNameserver()const{ return nameserverpath;}
	  
      /**
       *\exception throws LookUpException.
       */  
      kvService_ptr lookUpManager(bool forceNS,
				  bool & usedNS);
   	  
      bool addDataNotifySubscriber(DataNotifySubscriber *ptr, 
				   const std::string &subid);
      
      bool addDataSubscriber(DataSubscriber *ptr, 
			     const std::string    &subid);

      bool addHintSubscriber(HintSubscriber *ptr,
			       const std::string &subid);
	  
      std::string 
      subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
			  dnmi::thread::CommandQue &que);

      std::string 
      subscribeData(const KvDataSubscribeInfoHelper &info,
		    dnmi::thread::CommandQue &que);
      
      std::string 
      subscribeKvHint(dnmi::thread::CommandQue &que);
	  
      void unsubscribe(const std::string &subscribeid);
      
      void unsubscribeAll();


      bool getKvData(KvGetDataReceiver &dataReceiver,
		     const WhichDataHelper &wd,
		     termfunc func);      
                       
      bool getKvData(KvObsDataList &dataList,
		     const WhichDataHelper &wd,
		     termfunc func);      
 
     bool getKvRejectdecode( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, 
			     CKvalObs::CService::RejectedIterator_ptr &it );

      bool getKvParams(std::list<kvalobs::kvParam> &paramList);

      bool getKvStations(std::list<kvalobs::kvStation> &stationList);

      bool getKvModelData(std::list<kvalobs::kvModelData> &dataList,
			  const WhichDataHelper &wd);

      bool getKvReferenceStations(int stationid, 
				  int paramid, 
				  std::list<kvalobs::kvReferenceStation> &refList); 

      bool getKvTypes(std::list<kvalobs::kvTypes> &typeList);

      bool getKvOperator(std::list<kvalobs::kvOperator> &operatorList);

      bool getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
		       const std::list<long> &stationList,
		       bool aUnion);

      bool getKvStationParam( std::list<kvalobs::kvStationParam> &stParam,
			      int stationid, int paramid = -1, int day = -1 );


    };
  }
}

#endif
