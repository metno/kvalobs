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
#include <kvalobs/kvRejectdecode.h>
#include <milog/milog.h>
#include <miutil/trimstr.h>
#include "kvRejectedIteratorImpl.h"

using namespace std;
using namespace kvalobs;
using namespace CKvalObs::CService;
using namespace milog;

namespace{
  const int COUNT_MAX=25; //Max element to put in data.
  const int TIMESTEP =60;  //Timestemp in minutes

}

void
RejectedIteratorImpl::
fillData(std::list<kvalobs::kvRejectdecode> &toData,
	 const std::list<kvalobs::kvRejectdecode> &fromData,
	 std::list<kvalobs::kvRejectdecode>::iterator &fromDataIt)
{
	std::list<std::string>::iterator it;
	string decoder;	
	string mydecoder;
	string::size_type i;
	
  	for(; toData.size()<COUNT_MAX && 
	 	   fromDataIt!=fromData.end(); 
       	 fromDataIt++){
    
    	if(!decodeList.empty()){
      		for(it=decodeList.begin();
	  			it!=decodeList.end(); 
	  			it++){
				
				decoder=fromDataIt->decoder();

				i=decoder.find("/");
				
				if(i!=string::npos)
					mydecoder=decoder.substr(0, i);
				else
					mydecoder=decoder;
	  			
	  			miutil::trimstr(*it);
	  			miutil::trimstr(mydecoder);
	  			
	  			if(mydecoder==*it)
	  				break;
      		}
      
      		if(it==decodeList.end())
				continue;
    	}
    
    	toData.push_back(*fromDataIt);
  	}
}

bool 
RejectedIteratorImpl::
findData(std::list<kvalobs::kvRejectdecode> &data)
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

    ost << "WHERE tbtime>\'" << fromTime.isoTime() << "\' and " 
	<< "      tbtime<=\'" << currentEnd.isoTime() << "\' order by tbtime";

    dataList.clear();
    
    if(!gate.select(dataList, ost.str())){
      LOGERROR("DBERROR: " << gate.getErrorStr());
      return false;
    }

    dataIt=dataList.begin();
    fillData(data, dataList, dataIt);
    
  }

  return true;
}


RejectedIteratorImpl::
RejectedIteratorImpl(dnmi::db::Connection         *dbCon_,
		     const miutil::miTime         &fromTime,
		     const miutil::miTime         &toTime_,
		     const std::list<std::string> &decodeList_,
		     ServiceApp                   &app_)
  :dbCon(dbCon_), currentEnd(fromTime), toTime(toTime_),
   decodeList(decodeList_), dataIt(dataList.end()),
   app(app_)
{
}

RejectedIteratorImpl::
~RejectedIteratorImpl()
{
  LogContext context("service/RejectedIterator");
  LOGDEBUG("DTOR: called\n");
  
  if(dbCon)
    app.releaseDbConnection(dbCon);

}

void
RejectedIteratorImpl::
destroy()
{
  
  LogContext context("service/RejectedIterator");
  //CODE:
  // We must delete this instance of ModelDataIteratorImpl. We cant just 
  // call 'delete this'. We must also implement some mean of cleaning up
  // this instance if the client dont behave as expected or crash before
  // destroy is called.

  LOGDEBUG4("RejectedIteratorImpl::destroy: called!\n");
  deactivate();
  LOGDEBUG6("RejectedIteratorImpl::destroy: leaving!\n");
}


CORBA::Boolean 
RejectedIteratorImpl::
next(CKvalObs::CService::RejectdecodeList_out rejectedList)
{
	list<kvRejectdecode>           dataList;
	list<kvRejectdecode>::iterator it;
	bool active;

	IsRunningHelper running(*this, active );
  
	LogContext context("service/RejectedIterator");
	LOGDEBUG("RejectedIteratorImpl::next: called ... \n");
  
	//Check if we are deactivated. If so just return false.
	if( ! active ) {
		LOGDEBUG( "next: deactivated ( returning false)");
  		return false;
  	}
  
	rejectedList =new CKvalObs::CService::RejectdecodeList();
  
  

	if(!findData(dataList)){
		//An error occured.
		return false;
	}
 
	it=dataList.begin();

	if(it==dataList.end()){
		LOGERROR("All requested rejected data is sendt!");
		return false;
	}

	rejectedList->length(dataList.size());
  
	for(CORBA::Long datai=0;it!=dataList.end(); datai++, it++){
		(*rejectedList)[datai].message=it->message().c_str();
		(*rejectedList)[datai].tbtime=to_simple_string(it->tbtime()).c_str();
		(*rejectedList)[datai].decoder=it->decoder().c_str();
		(*rejectedList)[datai].comment=it->comment().c_str();
	    (*rejectedList)[datai].is_fixed=it->fixed();
	}
    
	LOGDEBUG("next: return " << rejectedList->length() << " elements!" <<endl);
    
	return true;
}

/*
 * TODO: Implement the cleanup, ie move the cleanup of the database object and whichData
 * from the deactivate method to this method. Remember to add a call to cleanup in 
 * the method ServiceApp::cleanUpReaperObj()
 */ 
void 
RejectedIteratorImpl::
cleanUp()
{
	app.releaseDbConnection(dbCon);
	dbCon=0;
}
