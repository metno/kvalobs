/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: MissingObsCheckTest.cc,v 1.1.2.2 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <db/dbdrivermgr.h>
#include <kvalobs/kvDbGate.h>
#include <list>
#include <milog/milog.h>
#include <boost/bind.hpp>
#include "../MissingObsCheck.h"

using namespace std;
using namespace dnmi::db;
using namespace kvalobs;

void InitLogger(const std::string &logname);

class MyShutdown{
	bool ret;
public:
	MyShutdown(bool ret):ret(ret){}
	bool shutdown(){ return ret;}
};

int main(int argc, char** argv)
{
	int status;
	dnmi::thread::CommandQue que;
	GenCache                 genCache;
  	string driver = "../../../lib/db/sqlite3driver.so";
  	string driveID;
  	string constr("MissingObsCheckTest.db");
  	Connection    *con;
  	DriverManager manager;
	//miutil::miTime now(miutil::miTime::nowTime());
	miutil::miTime now("2006-09-08 12:30:00");
	miutil::miTime lastSearch(now);  
  	InitLogger("MissingObsCheckTest");
  
   lastSearch.addHour(-1);
   lastSearch.addMin(-1*35);
  	system(string("rm -f " +constr).c_str());
  	system(string("sqlite3 "+constr+"< ../../kvalobs_database/script.create").c_str());
  	status=system(string("sqlite3 "+constr+"< MissingObsCheckTest_import.sql").c_str());

	if(WEXITSTATUS(status)!=0){
		cerr << "Cant execute: 'sqlite3 "<< constr <<" < MissingObsCheckTest_import.sql'\n";
		return 1;
	}
  	if(!manager.loadDriver(driver, driveID)){
    	cerr << "Can't load driver <" << driver << ">" <<  endl 
	 		  << manager.getErr() << endl;
    	return 1;
  	}
    
  	con=manager.connect(driveID, constr);
  
  	if(!con){
    	cerr << "Can't create connection to <" << driveID << endl;
    	return false;
  	}

  	cerr << "Connected to <" << driveID << ">" << endl;

	MyShutdown app(false);

	MissingObsCheck missingCheck(*con, que, &genCache, boost::bind(&MyShutdown::shutdown, &app) );

	missingCheck.findMissingData(now, lastSearch);

	
  	manager.releaseConnection(con);
  	return true;

};

void
InitLogger(const std::string &logname){
   using namespace milog;
   
   string       filename;
   LogLevel     traceLevel=milog::DEBUG;
   LogLevel     logLevel=milog::DEBUG;
   FLogStream   *fs;
   StdErrStream *trace;

	filename=logname+".log";    

   try{
		fs=new FLogStream(4);
	
		if(!fs->open(filename)){
	    	std::cerr << "FATAL: Can't initialize the Logging system.\n";
	    	std::cerr << "------ Cant open the Logfile <" << filename << ">\n";
	    	delete fs;
	    	exit(1);
		}
	
		trace=new StdErrStream();
    
		if(!LogManager::createLogger(logname, trace)){
	    	std::cerr << "FATAL: Can't initialize the Logging system.\n";
	    	std::cerr << "------ Cant create logger\n";
	    	exit(1);
		}
	
		if(!LogManager::addStream(logname, fs)){
	    	std::cerr << "FATAL: Can't initialize the Logging system.\n";
	    	std::cerr << "------ Cant add filelogging to the Logging system\n";
	    	exit(1);
		}
	
		trace->loglevel(traceLevel);
		fs->loglevel(logLevel);
	
		LogManager::setDefaultLogger(logname);
    }
    catch(...){
		std::cerr << "FATAL: Can't initialize the Logging system.\n";
		std::cerr << "------ OUT OF MEMMORY!!!\n";
		exit(1);
    }

    std::cerr << "Logging to file <" << filename << ">!\n";
}
