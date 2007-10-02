/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDbGateTest.cc,v 1.1.6.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#include <string>
#include <iostream>
#include <db/dbdrivermgr.h>
#include "kvalobs/kvDbGate.h"
#include <list>

using namespace dnmi::db;
using namespace std;
using namespace kvalobs;
using namespace miutil;

typedef list<kvData> TkvData; 


int
main(int argn, char *argv[])
{
    string driver("../../db/obj/pgdriver.so");
    string drvId;
    //string constr("dbname=kvtest");
    string constr("dbname=kvalobs user=kvalobs password=kvalobs12");
    Connection *con;
    DriverManager dbmngr;
    

    
    if(!dbmngr.loadDriver(driver, drvId)){
	cerr << "Can't load driver <" << driver << ">\n";
	cerr << dbmngr.getErr() << endl;
	
	return 1;
    }else
	cerr << "Driver <" << drvId<< "> loaded!\n";

  
    
    con=dbmngr.connect(drvId, constr);

    if(!con){
	cerr << "Can't create connection to <" << drvId << ">\n";
	return 1;
    }
    cerr << "Connected to <" << drvId << ">\n";
    
    kvDbGate gate(*con);
    TkvData data;

    kvData d(10, miTime::nowTime().isoTime(), 3.14, 1,
	     miTime::nowTime().isoTime(), 13, 2, 1, 0, 
	     kvDataFlag(), kvDataFlag());

    gate.insert(d);

    try{
	gate.select(data, "");
    }
    catch(SQLException &ex){
	cerr << "Exception: " << ex.what() << endl;
    }
    catch(...){
      cerr << "Unknown exception: con->exec(ctbl) .....\n";
    }

    delete con;
    
    TkvData::iterator it=data.begin();

    for(;it!=data.end(); it++){
      cerr << *it << endl;
    }
      
}
