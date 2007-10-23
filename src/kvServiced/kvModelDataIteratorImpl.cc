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
#include "kvModelDataIteratorImpl.h"
#include <kvalobs/kvDbGate.h>
#include <list>
#include <kvalobs/kvModelData.h>

using namespace std;
using namespace kvalobs;
using namespace CKvalObs::CService;

ModelDataIteratorImpl::ModelDataIteratorImpl(dnmi::db::Connection *dbCon_,
				   WhichDataList *whichData_,
				   ServiceApp &app_)
  :dbCon(dbCon_), whichData(whichData_), iData(0), app(app_)
{
}

ModelDataIteratorImpl::~ModelDataIteratorImpl()
{
  
  CERR("DTOR: ModelDataIteratorImpl::~ModelDataIteratorImpl...\n");
  
  if(whichData)
    delete whichData;

  if(dbCon)
    app.releaseDbConnection(dbCon);

  CERR("DTOR: ModelDataIteratorImpl::~ModelDataIteratorImpl ... 1 ...\n");
}


void  
ModelDataIteratorImpl::destroy()
{
  //CODE:
  // We must delete this instance of ModelDataIteratorImpl. We cant just 
  // call 'delete this'. We must also implement some mean of cleaning up
  // this instance if the client dont behave as expected or crash before
  // destroy is called.

  CERR("ModelDataIteratorImpl::destroy: called!\n");
  app.releaseDbConnection(dbCon);
  delete whichData;

  whichData=0;
  dbCon=0;
  
  app.removeReaperObj(this);
  
  deactivate();
  CERR("ModelDataIteratorImpl::destroy: leaving!\n");
}

CORBA::Boolean  
ModelDataIteratorImpl::next(CKvalObs::CService::ModelDataList_out modelDataList)
{
  list<kvModelData>           dataList;
  list<kvModelData>::iterator it;
  miutil::miTime         thisTime;
  CORBA::Long            obsi=0;
  CORBA::Long            datai=0;
  char                   *sTmp;
  //ObsDataList          obsDataList;

  IsRunningHelper(*this);

  modelDataList =new CKvalObs::CService::ModelDataList();
  CERR("ModelDataIteratorImpl::next: called ... \n");
  
  do{
    if(iData>=whichData->length()){
      CERR("ModelDataIteratorImpl::next: End of data reached (return false)!\n");
      return false;
    }
    
    try{
      
      //Try to keep me alive!

      if(!findData(dataList, (*whichData)[iData])){
	CERR("ModelDataIteratorImpl::next: Cant find data (return false)!\n");
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
      CERR("ModelDataIteratorImpl::next: EXCEPTION: \n" << "   " << ex.what() << endl);
      return false;
    }
    catch(...){
      CERR("ModelDataIteratorImpl::next: UNKNOWN EXCEPTION: \n");
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
      CERR("ModelDataIteratorImpl::next: obsDataList[" << obsi-1 << "].dataList.length()="
	   << (*modelDataList)[obsi-1].dataList.length() << endl);
    }
   
    (*modelDataList)[obsi].dataList.length(datai+1);
    (*modelDataList)[obsi].dataList[datai].stationID=it->stationID(); 
    (*modelDataList)[obsi].dataList[datai].obstime=
                                  it->obstime().isoTime().c_str();
    (*modelDataList)[obsi].dataList[datai].paramID=it->paramID();
    (*modelDataList)[obsi].dataList[datai].level=it->level();
    (*modelDataList)[obsi].dataList[datai].modelID=it->modelID();
    (*modelDataList)[obsi].dataList[datai].original=it->original();  
     
    datai++;
    it++; //Move to the next data in dataList.
  }
    
  CERR("ModelDataIteratorImpl::next: obsDataList->length()=" 
       << modelDataList->length() << endl);


  return true;
}

bool
ModelDataIteratorImpl::findData(list<kvModelData> &data, 
			   const CKvalObs::CService::WhichData &wData)
{
  kvDbGate gate(dbCon);
  miutil::miTime stime(wData.fromObsTime);
  miutil::miTime etime(wData.toObsTime);

  if(stime.undef() || etime.undef()){
    if(stime.undef()){
      ostringstream os;
      os << "Inavlid time spec (fromObsTime): ";

      if(wData.fromObsTime)
	os <<  wData.fromObsTime;
      else
	os << "(NULL POINTER)";

      os << "!";
	 
      throw InvalidWhichData(os.str());
    }

    if(etime.undef()){
      etime=etime.nowTime();
    }
  }
     
  CERR("ModelDataIteratorImpl::findData: calling gate.select(data, .... \n");

  if(gate.select(data, kvQueries::selectModelData(wData.stationid, stime, etime))){
    CERR("ModelDataIteratorImpl::findData: nElements=" << data.size() 
	 << " (return true)\n");
    return true;
  }

  CERR("ModelDataIteratorImpl::findData: return false\n");
  return false;
}
  
  
