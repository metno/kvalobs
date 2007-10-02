/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSubscriberImpl.cc,v 1.5.2.2 2007/09/27 09:02:46 paule Exp $                                                       

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
#include <dnmithread/mtcout.h>
#include <qthread.h>
#include <qapp.h>
#include "kvQtEvents.h"
#include "kvDataSubscriberImpl.h"
#include "kvservicetypes.h"
#include <puTools/miTime>
#include <string>

using namespace kvalobs;
using namespace std;

kvservice::priv::
DataSubscriber::DataSubscriber()
{
}
 
kvservice::priv::
DataSubscriber::~DataSubscriber()
{
}

void 
kvservice::priv::
DataSubscriber::callback(const CKvalObs::CService::ObsDataList& data)
{
  KvDataEvent   *dataEvent;
  KvObsDataList *dataList;
  KvDataList    tmpDataList; 
  KvObsDataListPtr dataListPtr;
  char          sensor;

  CERR("DataSubscriber::callback: called!\n");
	  
  if(data.length()<=0){
    CERR("DataSubscriber::callback: data.length()<=0!\n");
    return;
  }
	  
  try{
    dataList=new KvObsDataList();
  }
  catch(...){
    CERR("EXCEPTION: DataSubscriber::callback: return!\n");
    return;
  }
	
  dataListPtr.reset(dataList);
  
  
  for(CORBA::ULong i=0; i<data.length(); i++){
    tmpDataList.clear();

    for(CORBA::ULong k=0; k<data[i].dataList.length(); k++){
      if(strlen(data[i].dataList[k].sensor)>0)
	sensor=*data[i].dataList[k].sensor;
      else
	sensor=0;
      
      kvalobs::kvData myData(
	      data[i].dataList[k].stationID,
	      miutil::miTime(data[i].dataList[k].obstime),
	      data[i].dataList[k].original,
	      data[i].dataList[k].paramID,
	      miutil::miTime(data[i].dataList[k].tbtime),
	      data[i].dataList[k].typeID_,
	      sensor,
	      data[i].dataList[k].level,
	      data[i].dataList[k].corrected,
	      kvalobs::kvDataFlag(string(data[i].dataList[k].controlinfo)),
	      kvalobs::kvDataFlag(string(data[i].dataList[k].useinfo)),
	      string(data[i].dataList[k].cfailed));
      
      tmpDataList.push_back(myData);
    }

    dataListPtr->push_back(tmpDataList);
  }
  
  CERR("DataSubscriber::callback dataList->size()=" << 
       dataListPtr->size() << endl);
  
  
  try{
    dataEvent=new KvDataEvent(dataListPtr);
    QApplication::postEvent(qApp, dataEvent);
  }
  catch(...){
    return;
  }


}
