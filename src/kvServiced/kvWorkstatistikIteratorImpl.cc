/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvRejectedIteratorImpl.cc,v 1.2.2.4 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvWorkelement.h>
#include <milog/milog.h>
#include <miutil/timeconvert.h>
#include <miutil/trimstr.h>
#include "kvWorkstatistikIteratorImpl.h"

using namespace std;
using namespace kvalobs;
using namespace CKvalObs::CService;
using namespace milog;

namespace{
  const int COUNT_MAX=25; //Max element to put in data.
  const int TIMESTEP =60;  //Timestemp in minutes

}

void
WorkstatistikIteratorImpl::
fillData(std::list<kvalobs::kvWorkelement> &toData,
	 const std::list<kvalobs::kvWorkelement> &fromData,
	 std::list<kvalobs::kvWorkelement>::iterator &fromDataIt)
{
  	for(; toData.size() < COUNT_MAX &&
	 	   fromDataIt != fromData.end();
  	      fromDataIt++ ){
    	toData.push_back(*fromDataIt);
  	}
}

bool 
WorkstatistikIteratorImpl::
findData(std::list<kvalobs::kvWorkelement> &data)
{
  miutil::miTime fromTime;
  
  data.clear();
  fillData(data, dataList, dataIt);
  
  if(data.size()>=COUNT_MAX)
    return true;

  if(currentEnd>=toTime)
    return true;

  ostringstream ost;
  kvDbGate gate(dbCon);
  
  while(currentEnd<toTime && data.size()<COUNT_MAX){
    fromTime=currentEnd;
    currentEnd.addMin(TIMESTEP);

    if(currentEnd>toTime)
      currentEnd=toTime;
    
    ost.str("");

    ost << "WHERE " << timeType << ">\'" << fromTime.isoTime() << "\' and "
        << "      " << timeType << "<=\'" << currentEnd.isoTime() << "\' order by " << timeType;

    dataList.clear();
    
    if( ! gate.select( dataList, ost.str(), "workstatistik" ) ){
      LOGERROR("DBERROR: " << gate.getErrorStr());
      return false;
    }

    dataIt=dataList.begin();
    fillData(data, dataList, dataIt);
  }

  return true;
}


WorkstatistikIteratorImpl::
WorkstatistikIteratorImpl(
           dnmi::db::Connection  *dbCon_,
		     const miutil::miTime  &fromTime_,
		     const miutil::miTime  &toTime_,
		     WorkstatistikTimeType timeType_,
		     ServiceApp            &app_)
  :dbCon(dbCon_), currentEnd(fromTime_), toTime(toTime_),
   dataIt( dataList.end() ),
   app(app_)
{
   switch( timeType_ ) {
      case ObsTime: timeType="obstime"; break;
      case TbTime: timeType="tbtime"; break;
      case ProcessStartTime: timeType="process_start"; break;
      case QaStartTime: timeType="qa_start"; break;
      case QaStopTime: timeType="qa_stop"; break;
      case ServiceStartTime: timeType="service_start"; break;
      case ServiceStopTime: timeType="service_stop"; break;
      default:
         timeType="obstime";
   }

}

WorkstatistikIteratorImpl::
~WorkstatistikIteratorImpl()
{
  LogContext context("service/getWorkstatistikIterator");
  LOGDEBUG("DTOR: called\n");
  
  boost::mutex::scoped_lock lock( mutex );
  if(dbCon) {
    app.releaseDbConnection(dbCon);
    dbCon = 0;
  }

}

void
WorkstatistikIteratorImpl::
destroy()
{
  
  LogContext context("service/getWorkstatistikIterator");
  //CODE:
  // We must delete this instance of ModelDataIteratorImpl. We cant just 
  // call 'delete this'. We must also implement some mean of cleaning up
  // this instance if the client dont behave as expected or crash before
  // destroy is called.

  LOGDEBUG4("getWorkstatistikIterator::destroy: called!\n");
  deactivate();

  {
	  boost::mutex::scoped_lock lock( mutex );

	  if(dbCon) {
		  app.releaseDbConnection(dbCon);
	      dbCon = 0;
	  }
  }

  LOGDEBUG6("getWorkstatistikIterator::destroy: leaving!\n");
}


CORBA::Boolean 
WorkstatistikIteratorImpl::
next(CKvalObs::CService::WorkstatistikElemList_out wsList)
{
	list<kvWorkelement>           dataList;
	list<kvWorkelement>::iterator it;
	bool active;

	IsRunningHelper running(*this, active );
  
	boost::mutex::scoped_lock lock( mutex );

	LogContext context("service/getWorkstatistikIterator");
	LOGDEBUG("getWorkstatistikIterator::next: called ... \n");
  
	if( ! dbCon) {
		LOGERROR( "next:  No db connection (returning false)." );
		return false;
	}

	//Check if we are deactivated. If so just return false.
	if( ! active ) {
		LOGDEBUG( "next: deactivated ( returning false)");
  		return false;
  	}
  
	wsList =new CKvalObs::CService::WorkstatistikElemList();

	if( ! findData( dataList ) ){
		//An error occured.
		return false;
	}
 
	it=dataList.begin();

	if(it==dataList.end()){
		LOGERROR("All requested worstatistik is sendt!");
		return false;
	}

	wsList->length(dataList.size());
  
	for(CORBA::Long datai=0;it!=dataList.end(); datai++, it++){
	   (*wsList)[datai].stationID = it->stationID();
	   (*wsList)[datai].obstime = to_kvalobs_string(it->obstime()).c_str();
	   (*wsList)[datai].typeID_ = it->typeID();
	   (*wsList)[datai].tbtime= to_kvalobs_string(it->tbtime()).c_str();
	   (*wsList)[datai].priority = it->priority();
	   (*wsList)[datai].processStart = to_kvalobs_string(it->process_start()).c_str();
	   (*wsList)[datai].qaStart = to_kvalobs_string(it->qa_start()).c_str();
	   (*wsList)[datai].qaStop = to_kvalobs_string(it->qa_stop()).c_str();
	   (*wsList)[datai].serviceStart = to_kvalobs_string(it->service_start()).c_str();
	   (*wsList)[datai].serviceStop = to_kvalobs_string(it->service_stop()).c_str();
	}
    
	LOGDEBUG("next: return " << wsList->length() << " elements!" <<endl);
    
	return true;
}

/*
 * TODO: Implement the cleanup, ie move the cleanup of the database object and whichData
 * from the deactivate method to this method. Remember to add a call to cleanup in 
 * the method ServiceApp::cleanUpReaperObj()
 */ 
void 
WorkstatistikIteratorImpl::
cleanUp()
{
	boost::mutex::scoped_lock lock( mutex );

	if( dbCon ) {
		app.releaseDbConnection(dbCon);
		dbCon=0;
	}
}
