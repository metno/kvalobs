/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataNotifyCommandExt.cc,v 1.1.2.2 2007/09/27 09:02:21 paule Exp $                                                       

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
#include <puTools/miTime>
#include <milog/milog.h>
#include <list>
#include "DataNotifyCommandExt.h"

namespace kvskel=CKvalObs::CService;
using namespace kvalobs;
using namespace std;

void
DataNotifyCommandExt::
buildWhatList( kvskel::kvDataNotifySubscriberExt::WhatList &wl,
					DataToSubscribersPtr dp
      		 )
{
  unsigned char qcLevel=0x00;
  unsigned char flag;
  char           b[100];
  CORBA::Long    i=0;
  CORBA::Long    wli=0;
  //  kvalobs::CIkvParamInfoList it;

  wl.length(wli+1);
  wl[wli].stationID=dp->stationid;
  wl[wli].typeID_=dp->typeid_;
  wl[wli].obsTime=dp->obstime.isoTime().c_str();


  for(list<kvData>::const_iterator it=dp->dataList.begin();
      it!=dp->dataList.end(); 
      it++){
    flag=it->controlinfo().cflag(0);
    qcLevel |=flag;
  }
  
  i=0;

  if(qcLevel & KvDataSubscriberInfo::QC1_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=kvskel::QC1;
    i++;
  }

  if(qcLevel & KvDataSubscriberInfo::QC2d_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=kvskel::QC2d;
    i++;
  }

  if(qcLevel & KvDataSubscriberInfo::QC2m_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=kvskel::QC2m;
    i++;
  }

  if(qcLevel & KvDataSubscriberInfo::HQC_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=kvskel::HQC;
    i++;
  }
}




int 
DataNotifyCommandExt::
execute(KvDataSubscriberInfo &sinf)
{
  	using namespace CKvalObs::CService;
  	kvDataNotifySubscriberExt::WhatList wl;
	int ret;
	DataToSubscribersPtr dp=data();
	
 	
 	if(!sinf.thisStation(dp->stationid) ||
     	!sinf.checkStatusAndQc(dp)){
     	LOGDEBUG("DataNotifyCommandExt::excute: NOT interested in this dataset!" << endl
     				<< sinf);
     	
   	return 0;
  	}

  	buildWhatList(wl, dp);
    	
	try{
   	ret=0;
   	
   	if(CORBA::is_nil(subscriber))
   		return -1;
   	
   	if(!subscriber->callback(wl, subscriberid().c_str())){
    		ret=2;
    		LOGWARN("FAILED DataNotify: Subscriber dont accept more data.");
    	}else{
    		LOGINFO("SUCCESS DataNotify: CALL Stationid: " << dp->stationid << " typeid: " << 
	    	 		  dp->typeid_ << " obstime: " << dp->obstime);
    	}
   	
  	}
  	catch(CORBA::TRANSIENT &ex){
    	LOGERROR("EXCEPTION: (timeout?) Can't send <DataNotify> event to subscriber!" 
	     	 	 	<< endl << "Subscriberid: " << sinf.subscriberid() << ">!");
    	ret=1;
  	}
  	catch(...){
    	LOGERROR("EXCEPTION: Can't send <DataNotify> event to subscriber!" <<
	     			endl << "Subscriberid: " << sinf.subscriberid() << ">!");
    	ret=-1;
  	}

  	return ret;
}
