/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: guitestapp.cc,v 1.2.6.1 2007/09/27 09:02:47 paule Exp $                                                       

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
#include <guitestapp.h>
#include <kvParam.h>
#include <kvStation.h>

using namespace std;
using namespace kvservice;

typedef list<kvalobs::kvParam>                    ParamList;
typedef list<kvalobs::kvParam>::iterator         IParamList;
typedef list<kvalobs::kvParam>::const_iterator  CIParamList;

typedef list<kvalobs::kvStation>                    StationList;
typedef list<kvalobs::kvStation>::iterator         IStationList;
typedef list<kvalobs::kvStation>::const_iterator  CIStationList;

const int GUITestApp::maxData=1000;

GUITestApp::
GUITestApp(int argn, char **argv)
  :KvQtApp(argn, argv, true)
{
  {
    ParamList paramList;
  
    if(!getKvParams(paramList)){
      cerr << "Cant connect to kvalobs!";
      return;
    }

    for(IParamList it=paramList.begin();
	it!=paramList.end();
	it++){
      params[it->paramID()]=it->name();
    }
  }

  {
    StationList stationList;
  
    if(!getKvStations(stationList)){
      cerr << "Cant connect to kvalobs!";
      return;
    }

    for(IStationList it=stationList.begin();
	it!=stationList.end();
	it++){
      stations[it->stationID()]=it->name();
    }
  }

  subidKvHint=subscribeKvHint(this, 
			      SLOT(kvHint(bool)));

  if(subidKvHint.empty()){
    CERR("ERROR: KvDataView::KvDataView: cant subscribe on kvHint!\n");
  }else{
    CERR("INFO: KvDataView::KvDataView: subscribed on kvHint!\n"
	 << "  subscriber id (" << subidKvHint << ")!\n");
  }


  subidKvData=KvQtApp::kvQApp->subscribeData(
      KvDataSubscribeInfoHelper(), 
      this, 
      SLOT(newData(kvservice::KvObsDataListPtr)));
  
  if(subidKvData.empty()){
    CERR("ERROR: KvDataView::KvDataView: cant subscribe on data!\n");
  }else{
    CERR("INFO: KvDataView::KvDataView: subscribed on data!\n"
	 << "  subscriber id (" << subidKvData << ")!\n");
  }


}

GUITestApp::
~GUITestApp()
{
}


std::string
GUITestApp::
station(long sid)const
{
  CIIdMap it=stations.find(sid);

  if(it!=stations.end())
    return it->second;

  return string();
}

std::string 
GUITestApp::
param(long pid)const
{
  CIIdMap it=params.find(pid);

  if(it!=params.end())
    return it->second;

  return string();
}

void 
GUITestApp::
newData(kvservice::KvObsDataListPtr d)
{
  string name;

  for(CIKvObsDataList it=d->begin();
      it!=d->end();
      it++){
    if(it->size()>0){
      dataList.push_front(Data(*it, station(it->front().stationID()))); 
      
      if(dataList.size()>maxData)
	dataList.pop_back();
    }
  }

  emit newData(dataList);
}



void 
GUITestApp::
kvHint(bool commingUp)
{
  if(commingUp){
    CERR("KVALOBS: hint UP!\n");
  }else{
    CERR("KVALOBS: hint DOWN!\n");
  }
}
