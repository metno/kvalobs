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
#include <puTools/miTime.h>

using namespace CorbaHelper;
using namespace std;
using namespace CKvalObs::CService;
using namespace miutil;

int
main(int argn, char **argv)
{
	string        kvserviceName("/kv-conan/kvService");
	WhichDataList wd;
			
	CorbaApp      app(argn, argv);
	
	app.setNameservice("localhost");
	CORBA::Object_ptr tmpObj=app.getObjFromNS(kvserviceName);

	if(CORBA::is_nil(tmpObj)){
		cerr << "Cant locate CORBA object <" << kvserviceName << ">!" << endl;
		return 1;
	}
	
	kvServiceExt_var refKvService=kvServiceExt::_narrow(tmpObj);
	
	if(CORBA::is_nil(refKvService)){
		cerr << "Cant cast to <kvServiceExt>!";
		return 1;
	}
	
	Station_metadataList *stMeta;
	
	try {
	   string now = miTime::nowTime().isoTime();
	   cerr << "now: " << now << endl;
		if( !refKvService->getStationMetaData( stMeta, 18700, now.c_str(), "") ){
			cerr << "Failed: cant get data from <" << kvserviceName << ">!" << endl;
			return 1;
		}		
	} catch (...) {
		cerr << "Exception while getData..." << endl;
		return 1;		
	}
			
	try {
	
	   for (CORBA::Long i = 0; i < stMeta->length(); ++i) {
	      cerr << (*stMeta)[i].stationid << ","
	           << (*stMeta)[i].paramid << ","
	           << (*stMeta)[i].typeID_ << ","
	           << (*stMeta)[i].level << ","
	           << (*stMeta)[i].sensor << ","
	           << (*stMeta)[i].metadatatypename << ","
	           << (*stMeta)[i].metadata << ","
	           << "'" << (*stMeta)[i].metadataDescription << "'" << ","
	           << (*stMeta)[i].fromtime << ","
	           << (strlen((*stMeta)[i].totime)==0?"NULL": (*stMeta)[i].totime)
	           << endl;
	   }
	}
	catch (...) {
		cerr << "Exception while <next>!" << endl;
		
		return 1;
	}
	
	cerr << "Ok!" << endl;
}
