/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSubscriberImpl.cc,v 1.3.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include <milog/milog.h>
#include "../kvevents.h"
#include "kvDataSubscriberImpl.h"
#include "../kvservicetypes.h"
#include <puTools/miTime>
#include <string>

using namespace kvalobs;
using namespace std;

kvservice::priv::
DataSubscriber::DataSubscriber(dnmi::thread::CommandQue &que_):que(que_)
{
}
 
kvservice::priv::
DataSubscriber::~DataSubscriber()
{
}

#include <sys/time.h>

void 
kvservice::priv::
DataSubscriber::callback(const CKvalObs::CService::ObsDataList& data)
{
  LOGDEBUG( "Callback" );

  struct timeval tv;
  struct timezone tz;
  int result = gettimeofday( & tv, & tz );

  DataEvent        *dataEvent;
  KvObsDataList    *dataList;
  KvObsDataListPtr dataListPtr;
  char             sensor;

  if(data.length()<=0){
    return;
  }
	  
  try{
    dataList=new KvObsDataList();
  }
  catch(...){
    LOGERROR("EXCEPTION (kvcpp): DataSubscriber::callback: return!\n");
    return;
  }
	
  dataListPtr.reset(dataList);
  
  
  for(CORBA::ULong i=0; i<data.length(); i++){
    dataListPtr->push_back( KvObsData( data[i] ) );
  }
  
  try{
    dataEvent=new DataEvent(dataListPtr);
    que.postAndBrodcast(dataEvent);
  }
  catch(...){
    return;
  }

  if ( result == -1 ) {
    LOGINFO( "Time to process: ERROR" );
    return;
  }   
  struct timeval tv2;
  result = gettimeofday( & tv2, & tz );
  if ( result != -1 ) {
    LOGINFO( "Time to process: " << tv2.tv_sec - tv.tv_sec << " seconds, "
	     << tv2.tv_usec - tv.tv_usec << " microseconds." );
  }
  else {
    LOGINFO( "Time to process: ERROR_" );
  }
}
