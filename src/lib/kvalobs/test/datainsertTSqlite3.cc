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
#include <fileutil/file.h>
#include <kvalobs/DataInsertTransaction.h>
#include <kvalobs/kvDbGate.h>
#include <list>

using namespace dnmi::db;
using namespace dnmi::file;
using namespace std;
using namespace kvalobs;
using namespace miutil;

const char *schema = "CREATE TABLE data ("
    " stationid   INTEGER NOT NULL,"
    " obstime     TIMESTAMP NOT NULL,"
    " original    FLOAT NOT NULL,"
    " paramid      INTEGER NOT NULL,"
    " tbtime       TIMESTAMP NOT NULL,"
    " typeid       INTEGER NOT NULL,"
    " sensor       CHAR(1) DEFAULT '0',"
    " level     INTEGER DEFAULT 0,"
    " corrected   FLOAT NOT NULL,"
    " controlinfo CHAR(16) DEFAULT '0000000000000000',"
    " useinfo     CHAR(16) DEFAULT '0000000000000000',"
    " cfailed     TEXT DEFAULT NULL,      "
    " UNIQUE ( stationid, obstime, paramid, level, sensor, typeid )"
    " );"
    ""
    "CREATE INDEX data_obstime_index ON data (obstime);"
    "CREATE INDEX data_tbtime_index ON data (tbtime);";

typedef list<kvData> KvDataList;
typedef list<kvDbBase*> KvDbBaseList;

int main(int argn, char *argv[]) {
  string driverdir(DBDRIVERDIR);
  string driver(driverdir + "/sqlite3driver.so");
  string drvId;
  //string constr("dbname=kvtest");
  string constr("./testdb.sqlite");
  Connection *con;
  DriverManager dbmngr;
  File f("./testdb.sqlite");
  bool create = false;

  if (!dbmngr.loadDriver(driver, drvId)) {
    cerr << "Can't load driver <" << driver << ">\n";
    cerr << dbmngr.getErr() << endl;
    return 1;
  } else {
    cerr << "Driver <" << drvId << "> loaded!\n";
  }

  if (!f.ok())
    create = true;

  con = dbmngr.connect(drvId, constr);

  if (!con) {
    cerr << "Can't create connection to <" << drvId << ">\n";
    cerr << dbmngr.getErr() << endl;
    return 1;
  }

  cerr << "Connected to <" << drvId << ">\n";

  if (create) {
    cerr << "Must create db schema. ";

    try {
      con->exec(schema);
      cerr << "OK";
    } catch (...) {
      cerr << "Failed";
    }
    cerr << endl;
  }

  miTime now(miTime::nowTime());
  kvData d(10, now, 3.14, 1, now, 13, 2, 1, 0, kvControlInfo(), kvUseInfo(),
           "");

//   KvDbBaseList dbList;
  KvDataList dbList;

  dbList.push_back(d);
  DataInsertTransaction t1(dbList);

  try {
    DataInsertTransaction t1(dbList);
    con->perform(t1);
    cerr << "INSERT ok!" << endl;
  } catch (const std::exception &ex) {
    cerr << "Exception: " << ex.what() << endl;
    con->tryReconnect();
  }

  dbList.clear();
  dbList.push_back(
      kvData(101, now, 3.18, 1, now, 13, 2, 1, 3.14, kvControlInfo(),
             kvUseInfo(), ""));
  dbList.push_back(
      kvData(10, now, 3.18, 1, now, 13, 2, 1, 3.14, kvControlInfo(),
             kvUseInfo(), ""));

  try {
    DataInsertTransaction t1(dbList, DataInsertTransaction::INSERT_OR_UPDATE);
    con->perform(t1);
    cerr << "INSERT ok!" << endl;
  } catch (const std::exception &ex) {
    cerr << "Exception: " << ex.what() << endl;
    con->tryReconnect();
  }

  KvDataList data;
  kvDbGate gate(con);

//   if( !gate.insert( dbList, true ) ) {
//      cerr << gate.getErrorStr() << endl;
//   }
  ostringstream ost;

  ost << "WHERE stationid in (10, 101) AND typeid=13 AND obstime='"
      << now.isoTime() << "'";

  try {
    gate.select(data, ost.str());
  } catch (SQLException &ex) {
    cerr << "Exception: " << ex.what() << endl;
  } catch (...) {
    cerr << "Unknown exception: con->exec(ctbl) .....\n";
  }

  delete con;

  KvDataList::iterator it = data.begin();

  for (; it != data.end(); it++) {
    cerr << *it << endl;
  }

}
