/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceSubscriber.cc,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <cstring>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvWorkelement.h>
#include <milog/milog.h>
#include "ServiceSubscriber.h"
#include "DataToSubscribers.h"


using namespace std;
using namespace kvalobs;
using namespace miutil;

ServiceSubscriber::
ServiceSubscriber(ServiceApp &app_,
		  dnmi::thread::CommandQue &que_)
  :app(app_), inputque(que_), dbCon(0)
{
}
  
ServiceSubscriber::
ServiceSubscriber(const ServiceSubscriber &s)
  :app(s.app), inputque(s.inputque), dbCon(s.dbCon)
{
}

ServiceSubscriber::
~ServiceSubscriber()
{
}


void 
ServiceSubscriber::
updateWorkelementServiceStart(const kvalobs::kvStationInfo &st,
			      					dnmi::db::Connection *con)
{
  	kvDbGate gate(con);
  	ostringstream ost;
  
  	ost << "UPDATE workque SET service_start='" 
       << miTime::nowTime() 
       << "' WHERE stationid=" << st.stationID() 
       << "  AND obstime='" << st.obstime().isoTime() 
       << "' AND typeid=" << st.typeID();
  

  	if(!gate.exec(ost.str())){
    	LOGERROR("DBERROR: Cant update workque!" << endl <<
	             "Reason: " << gate.getErrorStr());
   }
}

void 
ServiceSubscriber::
updateWorkelementServiceStop(const kvalobs::kvStationInfo &st,
			       				  dnmi::db::Connection *con)
{
  	kvDbGate gate(con);
  	ostringstream ost;
  	list<kvWorkelement> workList;

  	ost << "UPDATE workque SET service_stop='" 
       << miTime::nowTime() 
       << "' WHERE stationid=" << st.stationID() 
       << "  AND obstime='" << st.obstime().isoTime() 
       << "' AND typeid=" << st.typeID();
  

  	if(!gate.exec(ost.str())){
    	LOGERROR("DBERROR: Cant update workque!" << endl <<
	     			"Reason: " << gate.getErrorStr());
   	return;
  	}
}

void 
ServiceSubscriber::
callSubscribers(const kvalobs::kvStationInfo &si)
{
  	long stationID;
  	std::list<kvalobs::kvData>       dataList;
  	std::list<kvalobs::kvTextData>   textDataList;
  
  	if(!app.subscribers.hasSubscribers())
    	return;
  
  	if(!dbCon){
    	LOGERROR("callDataSubscribers: dbCon==0!");
    	return;
  	}

  	kvalobs::kvDbGate gate(dbCon);

  	if(!gate.select(dataList,
						 kvQueries::selectDataFromType(si.stationID(),
										 						 si.typeID(),
																 si.obstime()))){
    	dataList.clear();
  	}

  	if(!gate.select(textDataList,
		  				 kvQueries::selectDataFromType(si.stationID(),
						  										 si.typeID(),
																 si.obstime()))){
    	textDataList.clear();
  	}
  
  	if(dataList.empty() && textDataList.empty())
    	return;

  	DataToSubscribersPtr data2sub;
  
  	try{
    	data2sub.reset(new DataToSubscribers(dataList, 
					 									 textDataList, 
					 									 si
					 									 )
		   			  );
  	}
  	catch(...){
    	LOGERROR("NOMEM: callDataSubscribers!");
    	return;
  	}
  
  	LOGINFO("CALL Subscribers: stationID: "<< data2sub->stationid 
	  		  << " obstime: " << data2sub->obstime 
	  		  << " typeID: " << data2sub->typeid_);
  	app.subscribers.forAllSubscribers(data2sub);
}

 
void       
ServiceSubscriber::
operator()()
{
  	const                     int CON_IDLE_TIME=60;
  	const                     int WAIT_ON_QUE_TIMEOUT=1;
  	int                       conIdleTime=0;
  	DataReadyCommand          *stInfoCmd=0;
  	dnmi::thread::CommandBase *cmd=0;

  	milog::LogContext logContext("ServiceSubscriber");

  	while(!app.shutdown()){
    	cmd=inputque.get(WAIT_ON_QUE_TIMEOUT);
    
    	if(!cmd){
      	conIdleTime+=WAIT_ON_QUE_TIMEOUT;
      
      	if(conIdleTime>CON_IDLE_TIME){
				if(dbCon){
	  				LOGDEBUG("Closing the database connection!");
	  				app.releaseDbConnection(dbCon);
	  				dbCon=0;
				}
	
				conIdleTime=0;
      	}
      
      	continue;
    	}
    
    	if(!dbCon){
      	//Will try to create a new connection to the database, we will
      	//not continue before a connection is created or the application
      	//is shutdown.
      
      	do{
				dbCon=app.getNewDbConnection();
	
				if(dbCon){
	  				LOGDEBUG("Created a new connection to the database!");
	  				break;
				}
	
				LOGINFO("Can't create a connection to the database, retry in 5 seconds ..");
				sleep(5);
      	}while(!app.shutdown());
      
      	if(!dbCon){
				//We have failed to create a new database connection and we have got a 
				//shutdown condition. We use continue to evaluate the outer while loop 
				//that in turn will end this thread.
				continue; 
      	}
    	}
    
    	try{
      	stInfoCmd=dynamic_cast<DataReadyCommand*>(cmd);
      
      	if(!stInfoCmd){
				delete cmd;
				LOGERROR("Unexpected command!");
				continue;
      	}
    	}
    	catch(...){
      	LOGERROR("Exception: unexpected command!");
      	continue;
    	}
    
    	LOGDEBUG("DataReady received!");
    
    	conIdleTime=0;
    	kvalobs::IkvStationInfoList it=stInfoCmd->getStationInfo().begin();

    	for(;it!=stInfoCmd->getStationInfo().end(); it++){
      	updateWorkelementServiceStart(*it, dbCon);    
      	callSubscribers(*it);
      	updateWorkelementServiceStop(*it, dbCon);    
    	}

    	app.sendToManager(stInfoCmd->getStationInfo(), 
		      				stInfoCmd->getCallback());
    
   	delete stInfoCmd;
  	}
  
  	LOGINFO("ServiceSubscriber terminated!");
} 



#if 0
void 
DataNotifyFunc::func(KvDataNotifySubscriberPtr ptr)
{
  using namespace CKvalObs::CService;
  kvDataNotifySubscriber::WhatList wl;

  if(!checkStatusAndQc(ptr)){
    return;
  }


  if(!buildWhatList(wl)){
    LOGERROR("DataNotifyFunc::func: buildWhatList failed!\n");
    return;
  }

  try{
    CKvalObs::CService::kvDataNotifySubscriber_var ref=ptr->subscriber();
    
    ref->callback(wl);
    ptr->connection(true);
  }
  catch(CORBA::TRANSIENT &ex){
       ptr->connection(false, true);
       LOGERROR("EXCEPTION: (timeout?) Can't send <DataNotify> event to subscriber!" <<
		endl << "Subscriberid: " << ptr->subscriberid() << ">!");
  }
  catch(...){
    ptr->connection(false);
    LOGERROR("EXCEPTION: Can't send <DataNotify> event to subscriber!" <<
	     endl << "Subscriberid: " << ptr->subscriberid() << ">!");
  }
}

bool
DataNotifyFunc::buildWhatList(
      CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl
      )
{
  unsigned char        qcLevel=0x00;
  unsigned char        flag;
  char                 b[100];
  CORBA::Long          i=0;
  CORBA::Long          wli=0;
  //  kvalobs::CIkvParamInfoList it;

  wl.length(wli+1);
  wl[wli].stationID=stationInfo.stationID();
  wl[wli].typeID_=stationInfo.typeID();
  wl[wli].obsTime=stationInfo.obstime().isoTime().c_str();


  for(list<kvData>::const_iterator it=dataList.begin();
      it!=dataList.end(); 
      it++){
    flag=it->controlinfo().cflag(0);
    qcLevel |=flag;
  }
  
  i=0;

  if(qcLevel & QC1_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=CKvalObs::CService::QC1;
    i++;
  }

  if(qcLevel & QC2d_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=CKvalObs::CService::QC2d;
    i++;
  }

  if(qcLevel & QC2m_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=CKvalObs::CService::QC2m;
    i++;
  }

  if(qcLevel & HQC_mask){
    wl[wli].qc.length(i+1);
    wl[wli].qc[i]=CKvalObs::CService::HQC;
    i++;
  }

  return true;
}


#endif
