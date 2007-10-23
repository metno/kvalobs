/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvServiceImpl.h,v 1.5.2.3 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __kvServiceImpl_h__
#define __kvServiceImpl_h__

#include <set>
#include <kvskel/kvService.hh>
#include <kvskel/adminInterface.h>
#include <boost/thread/thread.hpp>
#include "ServiceApp.h"
#include <list>
#include <kvalobs/kvObsPgm.h>


class KvServiceImpl: public virtual POA_CKvalObs::CService::kvService,
		     public virtual micutil::AdminInterface,
		     public PortableServer::RefCountServantBase {
  
  ServiceApp &app;
  void addToObsPgmList(CKvalObs::CService::Obs_pgmList &pgmList, 
		       std::list<kvalobs::kvObsPgm>    &obsPgms,
		       bool                            aUnion);

  CKvalObs::CService::WhichDataList*
    createWhichDataListForAllStations(dnmi::db::Connection *dbCon, 
				      const CKvalObs::CService::WhichData &wData);


public:
  // standard constructor
  KvServiceImpl(ServiceApp &app_);
  virtual ~KvServiceImpl();

  
  char* subscribeDataNotify(const CKvalObs::CService::DataSubscribeInfo& info, 
			    CKvalObs::CService::kvDataNotifySubscriber_ptr sub);

  char* subscribeData(const CKvalObs::CService::DataSubscribeInfo& info,
		      CKvalObs::CService::kvDataSubscriber_ptr sub);
  
  char* subscribeKvHint(CKvalObs::CService::kvHintSubscriber_ptr sub);

  void unsubscribe(const char *subid);
  
  CORBA::Boolean 
    getData(const CKvalObs::CService::WhichDataList& whichData,
	    CKvalObs::CService::DataIterator_out it);

  CORBA::Boolean 
    getModelData(const CKvalObs::CService::WhichDataList& whichData,
		 CKvalObs::CService::ModelDataIterator_out it);

  CORBA::Boolean 
    getRejectdecode(const CKvalObs::CService::RejectDecodeInfo& decodeInfo_,
		    CKvalObs::CService::RejectedIterator_out it);

  CORBA::Boolean 
    getReferenceStation(CORBA::Long stationid, 
			CORBA::Short paramsetid, 
			CKvalObs::CService::Reference_stationList_out 
			refStationList);

  CORBA::Boolean 
    getStations(CKvalObs::CService::StationList_out stationList);

  CORBA::Boolean 
    getParams(CKvalObs::CService::ParamList_out paramList_);

  CORBA::Boolean 
    getObsPgm(CKvalObs::CService::Obs_pgmList_out obs_pgmList_, 
	      const CKvalObs::CService::StationIDList& stationIDList_, 
	      CORBA::Boolean aUnion);

  CORBA::Boolean getTypes(CKvalObs::CService::TypeList_out typeList_);
  CORBA::Boolean getOperator(CKvalObs::CService::OperatorList_out operatorList_);

  CORBA::Boolean getStationParam( CKvalObs::CService::Station_paramList_out spList,
				  CORBA::Long stationid, 
				  CORBA::Long paramid, 
				  CORBA::Long day );
};


#endif
