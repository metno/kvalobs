/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvmgrcltApp.cc,v 1.1.2.3 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <stdlib.h>
#include <miutil/timeconvert.h>
#include <miconfparser/confparser.h>
#include <kvskel/managerInput.hh>
#include "kvmgrcltApp.h"

using namespace std;
using namespace miutil::conf;
using namespace dnmi::db;


KvMgrCltApp::
KvMgrCltApp(int argn, char **argv, const char *options[][2]):
	KvApp(argn, argv, options)
{
	ConfSection *conf=KvApp::getConfiguration();
	std::string dbdriver;
	
	if(!getenv("KVALOBS")){
		cerr << "The environment variable KVALOBS must be set!" << endl;
		exit(1);
	}
	
	string kvpath=getenv("KVALOBS");
	
	if(kvpath.empty()){
		cerr << "The environment variable KVALOBS is empty!" << endl;
		exit(1);
	}
	
	if(kvpath[kvpath.length()-1]!='/')
		kvpath+='/';
	
	if(!conf){
		cerr << "Cant read the configuration file <$KVALOBS/etc/kvalobs.conf>!";
		exit(1);
	}
	
	miutil::conf::ValElementList val=conf->getValue("database.dbdriver");
    
    if(val.size()==1)
      dbdriver=val[0].valAsString();
  

  	//Use postgresql as a last guess.
 	if(dbdriver.empty()){
 		cerr << "No database driver specified in <$KVALOBS/etc/kvalobs.conf>!";
 		exit(1);
 	}
 	
 	dbdriver=kvpath+"lib/db/"+dbdriver;
 	
 	if(!dbMgr.loadDriver(dbdriver, dbDriverId)){
  		cerr << "Can't load driver <" << dbdriver << endl 
	     	<< dbMgr.getErr() << endl 
	     	<< "Check if the driver is in the directory $KVALOBS/lib/db???";

    	exit(1);
  	}
  
  	cerr << "Db Driver <" << dbDriverId<< "> loaded!\n";
  
    val=conf->getValue("database.dbconnect");

    if(val.size()==1){
      	dbConnect=val[0].valAsString();
    }else{
    	cerr << "No database.dbconnect specified in $KVALOBS/etc/kvalobs.conf!" << endl;
    	exit(1);	
  	}
}

KvMgrCltApp::
~KvMgrCltApp()
{
}

bool 
KvMgrCltApp::
sendSignalToManager(int sid, int tid, const boost::posix_time::ptime &obstime)
{
	using namespace CKvalObs::CManager;
	
	ManagerInput_var ref;
	CKvalObs::StationInfoList st;
	
  	st.length(1);
	st[0].stationId=sid;
  	st[0].obstime=to_kvalobs_string(obstime).c_str();
  	st[0].typeId_=tid;
	
	
	try{
		CORBA::Object_var obj=getRefInNS("kvManagerInput");
		ref=ManagerInput::_narrow(obj);
		
		if(CORBA::is_nil(ref)){
      		cerr << "Can't find <kvManagerInput>" << endl;
      		return false;
    	}
    	
    	return ref->newData(st);
    }
	catch(CORBA::COMM_FAILURE& ex) {
    	cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
        	 << "object." << endl;
  	}
  	catch(CORBA::SystemException &ex) {
    	cerr << "Caught a CORBA::SystemException." << endl;
    	//cerr << "Reason: " << ex.what() << endl;
  	}
  	catch(CORBA::Exception&) {
    	cerr << "Caught CORBA::Exception." << endl;
  	}
  	catch(omniORB::fatalException& fe) {
    	cerr << "Caught omniORB::fatalException:" << endl;
    	cerr << "  file: " << fe.file() << endl;
    	cerr << "  line: " << fe.line() << endl;
    	cerr << "  mesg: " << fe.errmsg() << endl;
  	}
  	catch(...) {
    	cerr << "Caught unknown exception." << endl;
  	}
	
	return false;	
}



dnmi::db::Connection*
KvMgrCltApp::getNewDbConnection()
{
	Connection *con;
  
  	con=dbMgr.connect(dbDriverId, dbConnect);

  	if(!con){
    	cerr << "Can't create a database connection  (" 
	     	 << dbDriverId << ")" << endl << "Connect string: <" << dbConnect << ">!"
	     	 << endl;
    	return 0;
  	}

  	cerr << "New database connection (" << dbDriverId  << ") created!" << endl;
  	return con;
}

void
KvMgrCltApp::
releaseDbConnection(dnmi::db::Connection *con)
{
	dbMgr.releaseConnection(con);
}
