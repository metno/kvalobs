/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GetDataThread.cc,v 1.4.2.4 2007/09/27 09:02:22 paule Exp $                                                       

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
/* $Header: /var/lib/cvs/kvalobs/src/kvsynopd/GetDataThread.cc,v 1.4.2.4 2007/09/27 09:02:22 paule Exp $ */
#include <milog/milog.h>
#include <sstream>
#include <puTools/miTime>
#include "getDataReceiver.h"
#include "GetDataThread.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace milog;


GetData::
GetData(App                      &app_, 
		const miutil::miTime     &fromTime_,
		int                      wmono_,
		int                      hours_,
		dnmi::thread::CommandQue &que_):
  	app(app_), 
  	que(que_),
  	fromTime(fromTime_),
  	joinable_(new bool(false)),
  	wmono(wmono_),
  	hours(hours_)
{
}
  
void 
GetData::
operator()()
{
  dnmi::db::Connection     *con;
  miTime                   now(miTime::nowTime());
  miTime                   synopFromTime(now);

  if(hours>=0){
    synopFromTime.addHour(-24);
    //synopFromTime.addHour(-6);
    if(fromTime>=synopFromTime)
      synopFromTime=fromTime;
  }

  LogContext lContext("GetDataKv("+now.isoTime()+")"); 

  //Generate synop for maximum 7 hours back in time.

  LOGINFO("Started GetData thread!");
  IDLOGINFO("GetData", "Started GetData thread!");

  IDLOGINFO("GetData", 
	    "fromTime: " << fromTime << " hours: " << hours <<
	    " wmono: " << wmono << endl <<
	    " synopFromTime: " << synopFromTime);

  

  con=app.getNewDbConnection();

  if(!con){
    LOGERROR("Cant open a Db connection!");
    IDLOGERROR("GetData", "Cant open a Db connection!");
    *joinable_=true;
    return;
  }

  kvalobs::kvDbGate gate(con);
  gate.busytimeout(120);

  if(wmono<=0){
    reloadAll(gate, synopFromTime);
  }else{
    reloadOne(gate, synopFromTime);
  }

  app.releaseDbConnection(con);

  LOGINFO("Exit GetData thread!");
  IDLOGINFO("GetData", "Exit GetData thread!");
  *joinable_=true;
} 


void 
GetData::
reloadAll(kvalobs::kvDbGate    &gate,
	  const miutil::miTime &synopFromTime)
{
  kvservice::KvObsDataList dl;
  ostringstream            ost; 
  
  //Mark all station to be reloaded.
  App::StationList stationList=app.reloadCache(-1);

  for(App::IStationList it=stationList.begin();
      it!=stationList.end() && !app.shutdown();
      it++){
    StationInfo::TLongList idList=(*it)->stationID();
    kvservice::WhichDataHelper  which;

    ost.str("");
    
    for(StationInfo::ITLongList sit=idList.begin();
	sit!=idList.end(); sit++){
      ost << " " <<*sit;
      which.addStation(*sit, fromTime);
    }

    IDLOGINFO("GetData", 
	      "Started GetData: wmono=" << (*it)->wmono() << " stationids:" <<
	      ost.str() << " FromTime: "  << fromTime); 
  
    
    GetKvDataReceiver myDataReceiver(app, synopFromTime, que, gate, "GetData");
  
    if(!app.getKvData(myDataReceiver, which)){
      IDLOGERROR("GetData",
		 "Failed GetData: wmono=" << (*it)->wmono()
		 << " stationids:" << ost.str() << " FromTime: "  << fromTime);
    }
    
    IDLOGINFO("GetData",
	      "Success GetData: wmono=" << (*it)->wmono() << " stationids:" <<
	      ost.str() << " FromTime: "  << fromTime); 
  
    app.cacheReloaded((*it)->wmono());
  }
}

void 
GetData::
reloadOne(kvalobs::kvDbGate    &gate,
	  const miutil::miTime &synopFromTime)
{
  ostringstream                ost;
  App::StationList stationList=app.reloadCache(wmono);
  
  if(stationList.empty()){
    LOGWARN("No station information for wmono: " << wmono <<"!");
    IDLOGWARN("GetData", "No station information for wmono: " << wmono <<
	      "!");
    return;
  }

  StationInfo::TLongList idList=(*stationList.begin())->stationID();
  kvservice::WhichDataHelper  which;
  
  ost.str("");
  
  for(StationInfo::ITLongList sit=idList.begin();
      sit!=idList.end(); sit++){
    ost << " " <<*sit;
    which.addStation(*sit, fromTime);
  }
  
  IDLOGINFO("GetData",
	    "Started GetData: wmono=" << (*stationList.begin())->wmono() 
	    << " stationids:" << ost.str() << " FromTime: "  << fromTime); 
  
  
  GetKvDataReceiver myDataReceiver(app, synopFromTime, que, gate);
  
  if(!app.getKvData(myDataReceiver, which)){
    IDLOGERROR("GetData",
	       "Failed GetData: wmono=" << (*stationList.begin())->wmono()
	       << " stationids:" << ost.str() << " FromTime: "  << fromTime);
  }
  
  IDLOGINFO("GetData",
	    "Success GetData: wmono=" << (*stationList.begin())->wmono()
	    << " stationids:" << ost.str() << " FromTime: "  << fromTime); 
  
  app.cacheReloaded((*stationList.begin())->wmono());
  
}
