/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvpushApp.cc,v 1.1.2.3 2007/09/27 09:02:48 paule Exp $                                                       

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
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <puTools/miTime>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvWorkelement.h>
#include <miconfparser/confparser.h>
#include <kvskel/managerInput.hh>
#include <miutil/commastring.h>
#include <sstream>
#include <string>
#include "kvpushApp.h"

using namespace std;
using namespace miutil::conf;
using namespace dnmi::db;


KvPushApp::
KvPushApp(int argn, char **argv, const char *options[][2]):
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

KvPushApp::
~KvPushApp()
{
}

bool 
KvPushApp::
sendSignalToManager(int sid, int tid, const miutil::miTime &obstime)
{
	using namespace CKvalObs::CManager;
	
	ManagerInput_var ref;
	CKvalObs::StationInfoList st;
	
  	st.length(1);
	st[0].stationId=sid;
  	st[0].obstime=obstime.isoTime().c_str();
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
KvPushApp::
getNewDbConnection()
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
KvPushApp::
releaseDbConnection(dnmi::db::Connection *con)
{
	dbMgr.releaseConnection(con);
}

void 
KvPushApp::
getOpt(int argn, char **argv, Options &opt)
{
	int ch;
		
	while((ch=getopt(argn, argv, "hi:s:t:f:"))>-1){
		switch(ch){
			case 'h':
				opt.help=true;
				break;
			case 'i':
				readIntList(optarg, opt.typeids);
				break;
			case 's':
				readIntList(optarg, opt.stations);
				break;
			case 't':
				readDate(optarg, opt.totime);
				break;
			case 'f':
				readDate(optarg, opt.fromtime); 
				break;
			default:
				ostringstream ost;
				
				ost << "Unknown argument: " << ch <<".";
				throw GetOptEx(ost.str());			
		}
	}
	
	
}

void 
KvPushApp::
readIntList(const char *opt, Options::List &list)
{
	miutil::CommaString cstr(opt);
	string buf;
	int    n;
	
	list.clear();
	
	for(int i=0; i<cstr.size(); i++){
		if(!cstr.get(i, buf))
			continue;
		
		if(sscanf(buf.c_str(), "%i", &n)!=1){
			ostringstream ost;
			
			ost << "Not a number <" << buf << ">.";
			throw GetOptEx(ost.str());
		}
		
		list.push_back(n);
	}
	
}

void 
KvPushApp::
readDate(const char *opt, miutil::miTime &date)
{
	int year, mon, day, hour, min, sec;
	int n;
	
	n=sscanf(opt,"%d-%d-%dT%d:%d:%d", 
	         &year, &mon, &day, &hour, &min, &sec);
	         
	if(n<4){
		ostringstream ost;
		ost << "Invalid timespec: <" << opt << ">.";
		throw GetOptEx(ost.str());
	}
	         
	switch(n){
	case 6:         
		date=miutil::miTime(year, mon, day, hour, min, sec);
		break;
	case 5:         
		date=miutil::miTime(year, mon, day, hour, min);
		break;	          	
	case 4:         
		date=miutil::miTime(year, mon, day, hour);
		break;
	}
}

void
KvPushApp::
replace(std::string &src, const std::string &what, const std::string &with){
	std::string::size_type i;
	
	i=src.find(what);
	
	if(i==std::string::npos)
		return;
		
	src.replace(i, what.length(), with);
}


bool 
KvPushApp::
selectAllTypeids(const Options::List &stationList,  
   				  const miutil::miTime &fromtime,
					  const miutil::miTime &totime,
					  dnmi::db::Connection *con)
{
	ostringstream ost;
	bool ret=true;
	
	for(Options::CIList it=stationList.begin();
		  it!=stationList.end() && ret;
		  it++){
		ost.str(""); 	
		ost << "SELECT DISTINCT stationid, typeid, obstime FROM data " << endl
		 	 << "   WHERE stationid=" << *it << " AND " << endl
		 	 << "         obstime>='@fromtime@' AND obstime<='@totime@'";
		
		ret=selectData(ost.str(), fromtime, totime, con);
	}
	
	return ret;
}
								  
bool 
KvPushApp::
selectAllStations(const Options::List &typeidList,  
					   const miutil::miTime &fromtime,
					   const miutil::miTime &totime,
					   dnmi::db::Connection *con)
{
	ostringstream ost;
	bool ret=true;
	
	
	for(Options::CIList it=typeidList.begin();
		  it!=typeidList.end() && ret;
		  it++){
		ost.str(""); 	
		ost << "SELECT DISTINCT stationid, typeid, obstime FROM data " << endl
		 	 << "   WHERE typeid=" << *it << " AND " << endl
		 	 << "         obstime>='@fromtime@' AND obstime<='@totime@'";
		
		ret=selectData(ost.str(), fromtime, totime, con);
	}
	
	return ret;
}
								   
bool
KvPushApp:: 
selectFrom(const Options::List &stationidList,
           const Options::List &typeidList,  
   		  const miutil::miTime &fromtime,
	    	  const miutil::miTime &totime,
	    	  dnmi::db::Connection *con)
{
	ostringstream ost;
	bool ret=true;
	
	for(Options::CIList sit=stationidList.begin();
		  sit!=stationidList.end() && ret;
		  sit++){
		for(Options::CIList tit=typeidList.begin();
			 tit!=typeidList.end() && ret;
		  	 tit++){
		  	
			ost.str(""); 	
			ost << "SELECT DISTINCT stationid, typeid, obstime FROM data " << endl
		 	 	 << "   WHERE stationid=" << *sit << " AND typeid=" << *tit << " AND " << endl
		 	    << "         obstime>='@fromtime@' AND obstime<='@totime@'";
		
			ret=selectData(ost.str(), fromtime, totime, con);
		}
	}
	
	return ret;
}
						  
bool 
KvPushApp::
selectData(const std::string &query_,	
	  	     const miutil::miTime &fromtime_,
			  const miutil::miTime &totime_,
			  dnmi::db::Connection *con)
{
	miutil::miTime	fromtime(fromtime_);
	miutil::miTime totime(fromtime);
	string query;
	Result *dbRes;
	bool   ret=true;
	
	do{
		totime.addDay(1);
	
		if(totime>totime_)
			totime=totime_;
					
		query=query_;
	
		replace(query, "@fromtime@", fromtime.isoTime());	
		replace(query, "@totime@", totime.isoTime());	
	
		try{
			cout << "SQLquery[" << endl << query << endl << "]" << endl;
			dbRes=con->execQuery(query);
			
			if(!updateWorkque(dbRes, con)){
				ret=false;
			}
			
			delete dbRes;
			
			fromtime=totime;
			fromtime.addSec(1);		
		}
		catch(SQLException &ex){
			cerr << "Error: " << ex.what() << endl;
			ret=false;
		}
	}while(ret && totime<totime_);
	
	return ret;
}
						 
bool 
KvPushApp::
updateWorkque(dnmi::db::Result *res, dnmi::db::Connection *con)
{
	using namespace kvalobs;
	int nTry;
	bool retry;
	miutil::miTime now(miutil::miTime::nowTime());
	miutil::miTime undefTime;
	DataTblView data;
	
	while(res->hasNext()){
		nTry=0;
		retry=true;
		
		while(nTry<100 && retry){
			nTry++;

			try{
				DRow &row=res->next();
				data=DataTblView(row);
				retry=false;
			}
			catch(SQLBusy &ex){
				sleep(1);
				retry=true;
			}
			catch(...){
				throw;
			}
		}
		
		kvalobs::kvDbGate gate(con);
		
		gate.busytimeout(120); //2 minutter
		
		if(!gate.insert(kvWorkelement(data.stationID(),
												data.obstime(), 
										  		data.typeID(), 
										  		now, 
										  		10,
										  		now,
			                       		now, 
			                       		now, 
			                       		undefTime, 
			                       		undefTime), 
		                false)){
			if(gate.getError()!=kvalobs::kvDbGate::Duplicate){
				cerr << "ERROR: " << gate.getErrorStr() << endl;
				return false;
			}else{
				cout << "In process: stationid: " << data.stationID() << " typeid: " << data.typeID() 
			  << " obstime: " << data.obstime() << " to the workque." << endl;
			}  
		}else{	
			cout << "Added: stationid: " << data.stationID() << " typeid: " << data.typeID() 
				  << " obstime: " << data.obstime() << " to the workque." << endl;
		}  
	}
	return true;
}	

bool 
KvPushApp::
selectDataAndUpdateWorkque(const Options &opt)
{
	Connection     *con=getNewDbConnection();
	
	if(!con){
		cerr << "Cant connect to the database!" << endl;
		return false;
	}
	
	if(opt.typeids.empty())
		return selectAllTypeids(opt.stations, opt.fromtime, opt.totime, con);
	else if(opt.stations.empty())
		return selectAllStations(opt.typeids,opt.fromtime, opt.totime, con);
	else
		return selectFrom(opt.stations, opt.typeids, opt.fromtime, opt.totime, con);
}

std::ostream& 
operator<<(std::ostream &ost, const Options &opt)
{
	ost << "Station(s):";
	
	if(opt.stations.empty()){
		ost << " All" << endl;
	}else{
		for(Options::CIList it=opt.stations.begin();
			 it!=opt.stations.end();
			 it++)
			 ost << " " << *it;
		ost << endl;
	}
	
	ost << "Typeid(s):";
	
	if(opt.typeids.empty()){
		ost << " All" << endl;
	}else{
		for(Options::CIList it=opt.typeids.begin();
			 it!=opt.typeids.end();
			 it++)
			 ost << " " << *it;
		ost << endl;
	}
	
	ost << "Fromtime: " << opt.fromtime << endl;
	ost << "Totime:   " << opt.totime << endl;
	
	return ost;
}

