/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: getdata.cc,v 1.1.2.2 2007/09/27 09:02:41 paule Exp $                                                       

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
#include <iostream>
#include <kvskel/kvService.hh>
#include <corbahelper/corbaApp.h>
#include <puTools/miTime>

using namespace CorbaHelper;
using namespace std;
using namespace CKvalObs::CService;
using namespace miutil;

main(int argn, char **argv)
{
	string        kvserviceName("/kvtest-conan/kvService");
	WhichDataList wd;
			
	CorbaApp      app(argn, argv);
	
	app.setNameservice("localhost");
	CORBA::Object_ptr tmpObj=app.getObjFromNS(kvserviceName);

	if(CORBA::is_nil(tmpObj)){
		cerr << "Cant locate CORBA object <" << kvserviceName << ">!" << endl;
		return 1;
	}
	
	kvService_var refKvService=kvService::_narrow(tmpObj);
	
	if(CORBA::is_nil(refKvService)){
		cerr << "Cant cast to <kvService>!";
		return 1;
	}
	
	wd.length(1);
	//wd[0].stationid=0;
	wd[0].stationid=18700;
	wd[0].status=All;
	wd[0].fromObsTime="2009-05-26 08:00:00";
	wd[0].toObsTime="";
	
	DataIterator_var dataIt;
	
	try {
		if(!refKvService->getData(wd, dataIt)){
			cerr << "Failed: cant get data from <" << kvserviceName << ">!" << endl;
			return 1;
		}		
	} catch (...) {
		cerr << "Exception while getData..." << endl;
		return 1;		
	}
			
	ObsDataList *obsData;
	try {
	
		bool destroy=true;
		while(dataIt->next(obsData)){
			for(CORBA::Long i=0; i<obsData->length(); i++){
				cerr << "[" << i << "] stationid: " << (*obsData)[i].stationid << endl;
				
				for (CORBA::Long ii = 0; ii < (*obsData)[i].dataList.length(); ++ii) {
					cerr << (*obsData)[i].dataList[ii].stationID << ","
						 << miTime((*obsData)[i].dataList[ii].obstime) << ","
						 << miTime((*obsData)[i].dataList[ii].tbtime) << ","
						 << (*obsData)[i].dataList[ii].typeID_ << ","
						 << (*obsData)[i].dataList[ii].paramID << endl;
				}
			
				cerr << "------- textdata ---------" << endl;
					
				for (CORBA::Long ii = 0; 
					 ii < (*obsData)[i].textDataList.length(); 
					 ++ii) {
					cerr << (*obsData)[i].textDataList[ii].stationID << ","
					     << miTime((*obsData)[i].textDataList[ii].obstime) << ","
					     << (*obsData)[i].textDataList[ii].typeID_ << ","
					     << (*obsData)[i].textDataList[ii].paramID << endl;
				}
				
				cerr << "=============================================" << endl;
			}
			//abort();
			//sleep( 65);
			cerr << "<<<<<<<<<<<<<<<< next >>>>>>>>>>>>>>>>" << endl;
			
		}
		
		dataIt->destroy();
	}
	catch (...) {
		cerr << "Exception while <next>!" << endl;
		
		try{
			dataIt->destroy();
			dataIt->next(obsData);
		}
		catch(...){
		}
		
		return 1;
	}
	
	cerr << "Ok!" << endl;
}
