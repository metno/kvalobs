/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: drivermgr_test.cc,v 1.7.2.2 2007/09/27 09:02:26 paule Exp $                                                       

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
#include "../include/db/dbdrivermgr.h"

using namespace dnmi::db;
using namespace std;

int
main(int argn, char *argv[])
{
    string driver("../../dbdrivers/obj/pgdriver.so");
    string drvId;
    //string constr("dbname=kvtest");
    string constr("host=localhost dbname=kvalobs user=kvalobs password=kvalobs12");
    string ctbl("CREATE TABLE test (name char(30), age integer)");
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
	cerr << "ERROR: " << dbmngr.getErr() << "\n!!!\n";
	return 1;
    }
    cerr << "Connected to <" << drvId << ">\n";
    
    try{
	con->exec(ctbl);
    }
    catch(SQLException &ex){
	cerr << "Exception: " << ex.what() << endl;
    }
    catch(...){
      cerr << "Unknown exception: con->exec(ctbl) .....\n";
    }

    
    try{
	con->exec("INSERT INTO test VALUES('B�rge' , '36')");
	con->exec("INSERT INTO test VALUES('Asle' , '36')");
	
	Result *res=con->execQuery("SELECT * FROM test");
	
	if(res){
	    std::cerr << "Size: " << res->size() << endl;
	    cerr << res->fieldName(0) << "       " << res->fieldName(1)<<"\n";
	    cerr << "================================================\n";
	    while(res->hasNext()){
		DRow &row=res->next();
		CIDRow it=row.begin();
		
		/*
		for(int i=0; i<row.size(); i++){
		    cerr << row[i] << "      ";
		}
		*/
		
		for(;it!=row.end(); it++)
		  cerr << *it << "      ";

		cerr << endl;
	    }

	    delete res;
	    res=0;
	}else
	  cerr << "No result set!\n";
	
    }
    catch(SQLException &ex){
	cerr << "Exception: " << ex.what()  << endl;
    }

    if(!dbmngr.releaseConnection(con)){
      cerr << "Cant release the connection!\n";
    }

}
