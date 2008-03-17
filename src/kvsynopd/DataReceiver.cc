/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReceiver.cc,v 1.14.2.20 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <list>
#include <milog/milog.h>
#include "Data.h"
#include <kvalobs/kvDbGate.h>
#include "DataReceiver.h"
#include "ValidData.h"
#include "tblSynop.h"
#include <kvalobs/kvPath.h>

using namespace std;
using namespace kvservice;
using namespace kvalobs;
using namespace milog;

DataReceiver::DataReceiver(App &app_,
			   dnmi::thread::CommandQue &inputQue_,
			   dnmi::thread::CommandQue &outputQue_):
    app(app_), 
	inputQue(inputQue_), 
	outputQue(outputQue_),
	con(0)
{
}

void 
DataReceiver::
operator()()
{
  	dnmi::thread::CommandBase *com;
  	DataEvent                 *event;
 
  	milog::LogContext context("DataReceiver");

  	while(!app.shutdown()){
    	com=inputQue.get(1);

    	if(!com)
      		continue;

    	if(!con){
      		con=app.getNewDbConnection();
      
      		if(!con){
        		LOGFATAL("Cant create a connection to the database!\n");
        		LOGERROR("Cant save data from <kvalobs>!");
        		delete event;
        		continue;
      		}
    	}
    
    	event=dynamic_cast<DataEvent*>(com);

    	if(!event){
      		//Is this an event from the deleycontroll thread or
      		//getDataThread. getDataThread is started at program startup
      		//to get "old" data from kvalobs. It is terminated when
      		//all "old" data is received.
      
      		ObsEvent *obsevent=dynamic_cast<ObsEvent*>(com);
      
      		if(!obsevent){
        		LOGERROR("Unexpected event!");
        		delete com;
        		continue;
      		}
      
      		doCheckReceivedData(obsevent);
      
      		continue;
    	}
   
    	newData(event->data());
    
    	delete event;
  	}
  
  	if(con)
    	app.releaseDbConnection(con);

}

void 
DataReceiver::doCheckReceivedData(ObsEvent *obsevent)
{
  	milog::LogContext context("delaycontrol");

  	setDefaultLogger(obsevent->stationInfo());

	if(!obsevent->stationInfo()->synopForTime(obsevent->obstime().hour())){
   		LOGINFO("Skip SYNOP for this hour: " << obsevent->obstime() << endl <<
				" wmono: " << obsevent->stationInfo()->wmono() );
		delete obsevent;
		Logger::resetDefaultLogger();
		return;
	}

  	if(!typeidReceived(*obsevent)){
    	LOGERROR("Cant get information about received stationid/typeid!"<<
	    		 "Deleting event: " << obsevent->stationInfo()->wmono() <<
	     		 " obstime: " << obsevent->obstime());
    	delete obsevent;
  	}else if(obsevent->nTypeidReceived()==0){
    	LOGWARN("No stationid/typeid received!"<<
	    		"Deleting event: " << obsevent->stationInfo()->wmono() <<
	    		" obstime: " << obsevent->obstime());
    	delete obsevent;
  	}
   
  	LOGDEBUG3("Resend the event on outputque (newObsQue)");

  	outputQue.postAndBrodcast(obsevent);

  	Logger::resetDefaultLogger();
}


void 
DataReceiver::newData(kvservice::KvObsDataListPtr data)
{
  	kvalobs::kvDbGate gate(con);
  	IKvObsDataList    it;
  	StationInfoPtr    station;
  	miutil::miTime    toTime;
  	miutil::miTime    fromTime;

  	//data, er en liste av lister <KvData> til observasjoner 
  	//med samme stationid, typeid og obstime.

  	gate.busytimeout(120);
  	milog::LogContext context("newdata");
    
  	toTime=miutil::miTime::nowTime();
  	fromTime=toTime;
  	toTime.addHour(3);
  	fromTime.addDay(-3);

  	LOGINFO("Accepting data in the time interval: " << fromTime << " - " << 
	  		toTime);

	for(it=data->begin();   
      	it!=data->end();    
      	it++){              

    	KvObsData::kvDataList::iterator dit=it->dataList().begin();
    	std::list<Data> dataList;

    	if(dit==it->dataList().end()){
      		LOGWARN("Data received from kvalobs: Unexpected, NO Data!");
      		continue;
    	}

    
    	station=app.findStationInfo(dit->stationID());
    
    	if(!station){
      		LOGWARN("NO StationInfo: " << dit->stationID());
      		continue;
    	}
  
    	LOGINFO("Data received from kvalobs: stationID: " 
	    		<< it->stationid() 
	    		<< " (" << station->wmono() << ")" << endl 
	    		<< "       obstime: " << dit->obstime() << endl
	    		<< "        typeid: " << dit->typeID()  << endl 
	    		<< "   #parameters: " << it->dataList().size() << endl);
    
    	if(!app.acceptAllTimes() &&
           (dit->obstime()<fromTime || dit->obstime()>toTime)){
      		LOGWARN("obstime to old or to early: " << dit->obstime() << endl <<
	      			"-- Valid interval: " << fromTime << " - " << toTime);
      
      		//No 'continue' here! We reevaluate after we have set up to log
      		//to a station specific file and write the message to the logfile
      		//before we continue.
      		//See  **1**
      
    	}
  
  
    	if(!station->hasTypeId(dit->typeID())){
      		LOGWARN("StationInfo: typeid: " <<dit->typeID() << " not used!");
      
      		//No 'continue' here! We reevaluate after we have set up to log
      		//to a station specific file and write the message to the logfile
      		//before we continue.
      		//See  **2**
    	}

    	//Setup to log to a station specific file.
    	setDefaultLogger(station);
    
    	//This is repeated here to get it out on the log file for the wmono.
    	LOGINFO("Data received from kvalobs: stationID: " 
	    		<< it->stationid()<< endl 
	    		<< "       obstime: " << dit->obstime() << endl
	    		<< "        typeid: " << dit->typeID()  << endl 
	    		<< "   #parameters: " << it->dataList().size() << endl
	    		<< "Accepting data in time interval: " << fromTime << " - " << 
	    		toTime);
    
    	//**1**
    	if(!app.acceptAllTimes() && 
            (dit->obstime()<fromTime || dit->obstime()>toTime)){
      		LOGWARN("obstime to old or to early: " << dit->obstime() << endl <<
	      			"-- Valid interval: " << fromTime << " - " << toTime);
      		Logger::resetDefaultLogger();
      		LOGWARN("obstime to old or to early: " << dit->obstime() << endl <<
	      			"-- Valid interval: " << fromTime << " - " << toTime);
      		continue;
    	}
    
    	//**2**
    	if(!station->hasTypeId(dit->typeID())){
      		LOGWARN("StationInfo: typeid: " <<dit->typeID() << " not used!");
      		Logger::resetDefaultLogger();
      		LOGWARN("StationInfo: typeid: " <<dit->typeID() << " not used!");
      		continue;
    	}

    	for(dit=it->dataList().begin(); dit!=it->dataList().end(); dit++){
       		if(kvdatacheck::validData(*dit)){
	 			dataList.push_back(Data(*dit));
       		}
    	}
   
    	dit=it->dataList().begin();
    
    	if(dataList.size()==0){
      		LOGDEBUG("No new data to insert in database:" << endl 
	       	      << "  stationid: " << dit->stationID() 
	       		  << " obstime: " <<  dit->obstime() 
	       		  << " typeid: " << dit->typeID() << endl); 
      		Logger::resetDefaultLogger();
      		LOGDEBUG("No new data to insert in database:" << endl 
	              << "  stationid: " << dit->stationID() 
	       		  << " obstime: " <<  dit->obstime() 
	       		  << " typeid: " << dit->typeID() << endl);
      		continue;
    	}

    	if(!gate.insert(dataList, true)){
      		LOGERROR("Cant insert data: \n  stationid: " << dit->stationID() 
      			  << " obstime: " << dit->obstime() 
	       		  << "  reason: " << gate.getErrorStr() << endl);
      		Logger::resetDefaultLogger();
      		LOGERROR("Cant insert data: \n  stationid: " << dit->stationID() 
	       		  << " obstime: " << dit->obstime() 
	       		  << "  reason: " << gate.getErrorStr() << endl);

      		continue;
    	}

		if(!station->synopForTime(dit->obstime().hour())){
			LOGINFO("Skip SYNOP for this hour: " << dit->obstime() << endl
				<<  " stationid: " << dit->stationID() << endl
				<<  " typeid: " << dit->typeID());
			Logger::resetDefaultLogger();
			LOGINFO("Skip SYNOP for this hour: " << dit->obstime() << endl
				<<  " stationid: " << dit->stationID() << endl
				<<  " typeid: " << dit->typeID());
			continue;
		}
    
    	std::list<Data>::iterator dataIt=dataList.begin();
    
    	ObsEvent *event;
    
    	ostringstream errs;
    
    	errs << "Inserted data # " << dataList.size() << " :" << endl 
	 		 << "  stationid: " << dataIt->stationID() 
	 		 << " obstime: " <<  dataIt->obstime() 
	  		 << " typeid: " << dataIt->typeID() << endl;

    	LOGINFO(errs.str()); 
      
    	try{
      		event=new ObsEvent(dataIt->obstime(),
							   station);
      
      		if(!typeidReceived(*event)){
      			errs << "FAILED: Cant get informatiom about (stationid/typeid) " <<
      					"received for station <" << event->stationInfo()->wmono() << 
      					" at obstime <" << event->obstime() << endl;
      			LOGWARN("FAILED: Cant get informatiom about (stationid/typeid) " <<
      	        		"received for station <" << event->stationInfo()->wmono() << 
      	        		" at obstime <" << event->obstime());
      		}else if(event->nTypeidReceived()==0){
      			errs << "No data received (stationid/typeid)!" << endl <<
      	        		"Station (wmono): " << event->stationInfo()->wmono() << 
      	        		" obstime: " << event->obstime() << endl;

        		LOGWARN("No data received (stationid/typeid)!" << endl << 
                		"Station (wmono): " << event->stationInfo()->wmono() << 
                		" obstime: " << event->obstime());
        		delete event;
      		}else{
				app.addObsEvent(event, outputQue);
      		}
    	}
    	catch(...){
      		errs << "NOMEM: cant send a ObsEvent!";
      		LOGERROR("NOMEM: cant send a ObsEvent!");
    	}
    
    	//if(app.isContinuesType(dataIt->typeID())){
      		//We dont need to regenerate SYNOP for typeid that
      		//are not continues in time.
      		prepareToProcessAnySynopBasedOnThisObs(dataIt->obstime(),
									     		   station);
    	//}
    
    	Logger::resetDefaultLogger();

    	LOGINFO(errs.str());
  	}
}

void
DataReceiver::
prepareToProcessAnySynopBasedOnThisObs(const miutil::miTime &obstime,
				       StationInfoPtr station)
{
  	std::list<TblSynop> synopData;
  	miutil::miTime      now(miutil::miTime::nowTime());
  	miutil::miTime      maxTime;
  	miutil::miTime      time=obstime;

  	maxTime=miutil::miTime(obstime.year(), obstime.month(), obstime.day(),
			 obstime.hour(), 0, 0);
  	maxTime.addHour(24);

  	milog::LogContext context("regenerate");

  	if(maxTime>now)
    	maxTime=miutil::miTime(now.year(), now.month(), now.day(), 
							   now.hour(), 0, 0);

  	int hour=obstime.hour();
  	int r=hour%3;
  
  	//Nermest SYNOP tid som er st�rre enn obstime;
  	if(r==0)
    	time.addHour(3);
  	else
    	time.addHour(3-r); 
 
  	while(time<=maxTime){
    	LOGINFO("Posibly regenerating SYNOP for: " <<
	    		"  wmono: " << station->wmono() << " obstime: " << time<<endl);
    
    	try{
      		ObsEvent *event=new ObsEvent(time, station, true);
      
      		if(!typeidReceived(*event)){
      			LOGWARN("FAILED: Cant get informatiom about (stationid/typeid) " <<
                		"received for station <"<< event->stationInfo()->wmono() << 
                		" at obstime <" << event->obstime());
        		delete event;
        		return;
      		}else if(event->nTypeidReceived()==0){
        		LOGWARN("No data received (stationid/typeid)!" << endl <<
                		"Station (wmono): " << event->stationInfo()->wmono() << 
                		" obstime: " << event->obstime());
        		delete event;
      		}else{
        		app.addObsEvent(event, outputQue);
      		}
    	}
    	catch(...){
      		LOGERROR("NOMEM: cant send a ObsEvent!");
    	}

    	time.addHour(3);
  	}

}

bool
DataReceiver::
typeidReceived(ObsEvent &event)
{
  	StationInfo::TLongList stations=event.stationInfo()->stationID();
  	StationInfo::TLongList::iterator it=stations.begin();
  	ostringstream ost;
  
  	if(!con)
    	return false;

  	if(it==stations.end())
    	return false;
  
  	ost << "SELECT distinct stationid,typeid FROM data WHERE " 
      	<< "obstime='" << event.obstime().isoTime() << "' AND "
      	<< "stationid IN (" << *it;
  
  	it++;
  	for(;it!=stations.end();it++){
    	ost << "," << *it;
  	}

  	ost << ")";

  	LOGDEBUG3("typeidReceived: query:" << endl << ost.str());

  	bool retry=true;
  	time_t     now;
  	time_t     timeout;
  	dnmi::db::Result *rs;
 
  	time(&now);
  	timeout=now+120; //2 minutter
  
 	while(retry && now<timeout){
    	retry=false;
    
   		try{
      		rs=0;
      		rs=con->execQuery(ost.str());
    	}
    	catch(dnmi::db::SQLBusy &ex){
      		time(&now);
      		retry=true;
    	}
    	catch(dnmi::db::SQLException &ex){
      		it=stations.begin();
      		ost.str("");

      		ost << event.stationInfo()->wmono()  << " ";
      		ost << *it;
     		it++;

      		for(;it!=stations.end();it++){
        		ost << "," << *it;
      		}
      
      		ost << ")";
 
      		LOGERROR("SQLException: cant read the stationid/typeid for " << endl <<
            		 "event: " << ost.str() << " obstime: " <<
               		 event.obstime() << endl << "Reason: " << con->lastError());
      		return false;
    	}
  	}

  	if(!rs)
    	return false;
  
  	string ssid;
  	string stid;

  	try{
    	ost.str("");
    	ost << "typeidReceived for WMO: " << event.stationInfo()->wmono() << 
        	   " obstime: " << event.obstime() << endl << " (stationid/typeid):";
      
    	while(rs->hasNext()){
      		dnmi::db::DRow &row=rs->next();
      
      		ssid=row[0];
      		stid=row[1];

      		if(ssid.empty() || stid.empty())
        		continue;

      		ost << " (" << ssid << "/" << stid << ")";
      		event.addTypeidReceived(atoi(ssid.c_str()), atoi(stid.c_str()));
    	}
    	delete rs;
    
    	LOGINFO(ost.str());
  	}
  	catch(...){
    	delete rs;

    	//We clear the list. An empty list means "accept all", when the data
    	//is used i SYNOP generation!
    	event.clearTypeidReceived();
    	return false;
  	}
  
  	return true;
}

void 
DataReceiver::setDefaultLogger(StationInfoPtr station)
{
  	try{
    	FLogStream *logs=new FLogStream(1, 204800); //200k
    	std::ostringstream ost;
    
    	ost << kvPath("localstatedir") << "/log/kvsynop/dr-" 
			<< station->wmono() << ".log";
    
    	if(logs->open(ost.str())){
      		Logger::setDefaultLogger(logs);
      		Logger::logger().logLevel(station->loglevel());
    	}else{
      		LOGERROR("Cant open the logfile <" << ost.str() << ">!");
      		delete logs;
    	}
  	}
  	catch(...){
    	LOGERROR("Cant create a logstream for wmono: " << 
	    		 station->wmono() );
  	}
}
