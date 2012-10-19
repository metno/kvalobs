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
#include <kvalobs/kvData.h>
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/DataInsertTransaction.h>
#include <kvalobs/kvDbGate.h>
#include <list>

using namespace dnmi::db;
using namespace std;
using namespace kvalobs;
using namespace miutil;

typedef list<kvData> KvDataList;
typedef list<kvDbBase*> KvDbBaseList;

int
main(int argn, char *argv[])
{
   string driverdir( DBDRIVERDIR );
   string driver( driverdir +"/pgdriver.so");
   string drvId;
   //string constr("dbname=kvtest");
   string constr("dbname=kvalobs user=kvalobs port=5434 host=localhost");
   Connection *con;
   DriverManager dbmngr;



   if(!dbmngr.loadDriver(driver, drvId)){
      cerr << "Can't load driver <" << driver << ">\n";
      cerr << dbmngr.getErr() << endl;
      return 1;
   }else {
      cerr << "Driver <" << drvId<< "> loaded!\n";
   }

   con=dbmngr.connect(drvId, constr);

   if(!con){
      cerr << "Can't create connection to <" << drvId << ">\n";
      cerr << dbmngr.getErr() << endl;
      return 1;
   }

   cerr << "Connected to <" << drvId << ">\n";


   miTime now( miTime::nowTime() );
   kvData d(10, now, 3.14, 1, now, 13, 2, 1, 0,
            kvControlInfo(), kvUseInfo(), "");

//   KvDbBaseList dbList;
   KvDataList dbList;

   dbList.push_back( d );
   DataInsertTransaction t1( dbList );

   try {
      DataInsertTransaction t1( dbList );
      con->perform( t1 );
      cerr << "INSERT ok!" << endl;
   }
   catch( const std::exception &ex ) {
      cerr << "Exception: " << ex.what() << endl;
      con->tryReconnect();
   }

   try {
      dbList.clear();
      dbList.push_back( kvData(10, now, 3.18, 1, now, 13, 2, 1, 3.14,
                                      kvControlInfo(), kvUseInfo(), "") );
      DataInsertTransaction t1( dbList, DataInsertTransaction::INSERT_OR_UPDATE );
      con->perform( t1 );
      cerr << "INSERT ok!" << endl;
   }
   catch( const std::exception &ex ) {
      cerr << "Exception: " << ex.what() << endl;
      con->tryReconnect();
   }

   KvDataList data;
   kvDbGate gate( con );

   ostringstream ost;

   ost << "WHERE stationid=10 AND typeid=13 AND obstime='"<<now.isoTime()<<"'";

   try{
      gate.select(data, ost.str() );
   }
   catch(SQLException &ex){
      cerr << "Exception: " << ex.what() << endl;
   }
   catch(...){
      cerr << "Unknown exception: con->exec(ctbl) .....\n";
   }

   delete con;

   KvDataList::iterator it=data.begin();

   for(;it!=data.end(); it++){
      cerr << *it << endl;
   }

}
