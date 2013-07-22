/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvModelDataIteratorImpl.cc,v 1.1.6.4 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <miutil/timeconvert.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvModelData.h>
#include "kvModelDataIteratorImpl.h"
#include "toStringHelper.h"

using namespace std;
using namespace kvalobs;
using namespace CKvalObs::CService;
using namespace milog;

ModelDataIteratorImpl::ModelDataIteratorImpl(dnmi::db::Connection *dbCon_,
				   WhichDataList *whichData_,
				   ServiceApp &app_)
  :dbCon(dbCon_), whichData(whichData_), iData(0), app(app_)
{
}

ModelDataIteratorImpl::~ModelDataIteratorImpl()
{
  
	LOGDEBUG("DTOR: ModelDataIteratorImpl::~ModelDataIteratorImpl...\n");
  
	if(whichData)
		delete whichData;
	{
		boost::mutex::scoped_lock lock( mutex );
		if(dbCon) {
			app.releaseDbConnection(dbCon);
			dbCon = 0;
		}
	}
	
	LOGDEBUG("DTOR: ModelDataIteratorImpl::~ModelDataIteratorImpl ... 1 ...\n");
}


void  
ModelDataIteratorImpl::destroy()
{
	// We just deactivate the object here. The cleanup thread will release the resources
	// and remove it from the reaperObjList.
	
	LOGDEBUG("ModelDataIteratorImpl::destroy: called!\n");
	deactivate();

	{
		boost::mutex::scoped_lock lock( mutex );
		if(dbCon) {
			app.releaseDbConnection(dbCon);
			dbCon = 0;
		}
	}
	LOGDEBUG("ModelDataIteratorImpl::destroy: leaving!\n");
}

CORBA::Boolean  
ModelDataIteratorImpl::next(CKvalObs::CService::ModelDataList_out modelDataList)
{
	list<kvModelData>           dataList;
	list<kvModelData>::iterator it;
	boost::posix_time::ptime thisTime;
	CORBA::Long            obsi=0;
	CORBA::Long            datai=0;
	char                   *sTmp;
	bool                   active;
  //ObsDataList          obsDataList;

	LogContext context("service/ModelDataIterator");
	IsRunningHelper(*this, active );

	boost::mutex::scoped_lock lock( mutex );

	LOGDEBUG("ModelDataIteratorImpl::next: called ... \n");
  
	if( ! dbCon ) {
		LOGERROR( "next:  No db connection (returning false)." << endl << toString(*whichData));
		return false;
	}

	//Check if we are deactivated. If so just return false.
	if( ! active ) {
		LOGWARN( "next: deactivated ( returning false)." << endl << toString(*whichData));
		return false;
	} 
  
	modelDataList =new CKvalObs::CService::ModelDataList();
	
	do{
		if(iData>=whichData->length()){
			LOGDEBUG("ModelDataIteratorImpl::next: End of data reached (return false)!\n" << endl << toString( *whichData ));
			return false;
		}
    
		try{
			if(!findData(dataList, (*whichData)[iData])){
				LOGWARN("ModelDataIteratorImpl::next: Cant find data (return false)!\n");
				//CODE:
				//We have a problem with the connection to the database.
				//We return false. false is used to mark the end of stream. So
				//the caller sees this as an end of stream, that is wrong. We 
				//may add an exception here later to tell the caller that we
				//had problems. Anyway, the caller must react the same and
				//call destroy on the iterator.
				return false;
			}
		}
		catch(InvalidWhichData &ex){
			LOGERROR("ModelDataIteratorImpl::next: EXCEPTION: \n" << "   " << ex.what() << endl << toString( *whichData ));
			return false;
		}
		catch(...){
			LOGERROR("ModelDataIteratorImpl::next: UNKNOWN EXCEPTION." << endl << toString( *whichData ));
			return false;
		}
     
		iData++;
	}while(dataList.empty());

	it=dataList.begin();
  
	if(it!=dataList.end()){
		modelDataList->length(obsi+1);
		thisTime=it->obstime();
	}

	while(it!=dataList.end()){
		if(it->obstime()!=thisTime){
			//New record
			obsi++;
			datai=0;
			thisTime=it->obstime();
			modelDataList->length(obsi+1);
			LOGDEBUG( "ModelDataIteratorImpl::next: obsDataList[" << obsi-1 << "].dataList.length()="
					    << (*modelDataList)[obsi-1].dataList.length() << endl);
		}
   
		(*modelDataList)[obsi].dataList.length(datai+1);
		(*modelDataList)[obsi].dataList[datai].stationID=it->stationID(); 
		(*modelDataList)[obsi].dataList[datai].obstime=
                                  to_kvalobs_string_without_decimal_secound(it->obstime()).c_str();
		(*modelDataList)[obsi].dataList[datai].paramID=it->paramID();
		(*modelDataList)[obsi].dataList[datai].level=it->level();
		(*modelDataList)[obsi].dataList[datai].modelID=it->modelID();
		(*modelDataList)[obsi].dataList[datai].original=it->original();  
     
		datai++;
		it++; //Move to the next data in dataList.
	}
    
	LOGDEBUG( "ModelDataIteratorImpl::next: obsDataList->length()=" 
             << modelDataList->length() << endl);

	return true;
}

bool
ModelDataIteratorImpl::findData(list<kvModelData> &data, 
			   const CKvalObs::CService::WhichData &wData)
{
	kvDbGate gate(dbCon);
	boost::posix_time::ptime stime = boost::posix_time::time_from_string_nothrow((const char *) wData.fromObsTime);
	boost::posix_time::ptime etime = boost::posix_time::time_from_string_nothrow((const char *) wData.toObsTime);

	if(stime.is_not_a_date_time() || etime.is_not_a_date_time()){
		if(stime.is_not_a_date_time()){
			ostringstream os;
			os << "Inavlid time spec (fromObsTime): ";

			if(wData.fromObsTime)
				os <<  wData.fromObsTime;
			else
				os << "(NULL POINTER)";

			os << "!";
	 
			throw InvalidWhichData(os.str());
		}

		if(etime.is_not_a_date_time()){
			etime=boost::posix_time::second_clock::universal_time();
		}
	}
     
	LOGDEBUG("ModelDataIteratorImpl::findData: calling gate.select(data, .... \n");

	if( gate.select(data, kvQueries::selectModelData(wData.stationid, stime, etime))){
		LOGDEBUG( "ModelDataIteratorImpl::findData: nElements=" << data.size() 
				    << " (return true)\n");
		return true;
	}

	LOGDEBUG("ModelDataIteratorImpl::findData: return false\n");
	return false;
}
  
/*
 * TODO: Implement the cleanup, ie move the cleanup of the database object and whichData
 * from the deactivate method to this method. Remember to add a call to cleanup in 
 * the method ServiceApp::cleanUpReaperObj()
 */ 
void 
ModelDataIteratorImpl::
cleanUp()
{
	boost::mutex::scoped_lock lock( mutex );

	if(dbCon)
		app.releaseDbConnection(dbCon);

	delete whichData;

	whichData=0;
	dbCon=0;
}
  

