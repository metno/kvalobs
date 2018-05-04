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
#include <puTools/miTime.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvWorkelement.h>
#include <miconfparser/confparser.h>
#include <kvskel/managerInput.hh>
#include <miutil/commastring.h>
#include <miutil/timeconvert.h>
#include <sstream>
#include <string>
#include <kvalobs/kvPath.h>
#include "kvpullApp.h"

using namespace std;
using namespace miutil::conf;
using namespace dnmi::db;

KvPullApp::KvPullApp(int argn, char **argv, const char *options[][2])
    : KvApp(argn, argv, options) {
  string confdir = kvPath("sysconfdir");
  string pkglibdir = kvPath("pkglibdir");
  string confile = confdir + "/kvpull.conf";

  cerr << "(MY) Reading conf: "  << confile << "\n";

  ConfSection *conf = miutil::conf::ConfParser::parse(confile);
  std::string dbdriver;

  if (!conf) {
    cerr << "Cant read the configuration file '" << confile << "'.\n\n";
    exit(1);
  }

  miutil::conf::ValElementList val = conf->getValue("database.dbdriver");

  if (val.size() == 1)
    dbdriver = val[0].valAsString();

  //Use postgresql as a last guess.
  if (dbdriver.empty()) {
    cerr << "No database driver specified in '" << confile << "'." << endl
         << endl;
    exit(1);
  }

  dbdriver = pkglibdir + "/db/" + dbdriver;

  if (!dnmi::db::DriverManager::loadDriver(dbdriver, dbDriverId)) {
    cerr << "Can't load driver <" << dbdriver << endl << dnmi::db::DriverManager::getErr() << endl
         << "Check if '" << dbdriver << "' exist." << endl << endl;

    exit(1);
  }

  cerr << "Db Driver <" << dbDriverId << "> loaded!\n";

  val = conf->getValue("database.dbconnect");

  if (val.size() == 1) {
    dbConnect = val[0].valAsString();
  } else {
    cerr << "No database.dbconnect specified in '" << confile << "'.\n\n";
    exit(1);
  }
}

KvPullApp::~KvPullApp() {
}

dnmi::db::Connection*
KvPullApp::getNewDbConnection() {
  Connection *con;

  con = dnmi::db::DriverManager::connect(dbDriverId, dbConnect);

  if (!con) {
    cerr << "Can't create a database connection  (" << dbDriverId << ")" << endl
         << "Connect string: <" << dbConnect << ">!" << endl;
    return 0;
  }

  cerr << "New database connection (" << dbDriverId << ") created!" << endl;
  return con;
}

void KvPullApp::releaseDbConnection(dnmi::db::Connection *con) {
  dnmi::db::DriverManager::releaseConnection(con);
}

void KvPullApp::getOpt(int argn, char **argv, Options &opt) {
  int ch;

  while ((ch = getopt(argn, argv, "hqi:s:t:f:")) > -1) {
    switch (ch) {
      case 'h':
        opt.help = true;
        break;
      case 'q':
        opt.doQa = true;
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

        ost << "Unknown argument: " << ch << ".";
        throw GetOptEx(ost.str());
    }
  }

}

void KvPullApp::readIntList(const char *opt, Options::List &list) {
  miutil::CommaString cstr(opt);
  string buf;
  int n;

  list.clear();

  for (int i = 0; i < cstr.size(); i++) {
    if (!cstr.get(i, buf))
      continue;

    if (sscanf(buf.c_str(), "%i", &n) != 1) {
      ostringstream ost;

      ost << "Not a number <" << buf << ">.";
      throw GetOptEx(ost.str());
    }

    list.push_back(n);
  }

}

void KvPullApp::readDate(const char *opt, miutil::miTime &date) {
  int year, mon, day, hour, min, sec;
  int n;

  n = sscanf(opt, "%d-%d-%dT%d:%d:%d", &year, &mon, &day, &hour, &min, &sec);

  if (n < 4) {
    ostringstream ost;
    ost << "Invalid timespec: <" << opt << ">.";
    throw GetOptEx(ost.str());
  }

  switch (n) {
    case 6:
      date = miutil::miTime(year, mon, day, hour, min, sec);
      break;
    case 5:
      date = miutil::miTime(year, mon, day, hour, min);
      break;
    case 4:
      date = miutil::miTime(year, mon, day, hour);
      break;
  }
}


std::ostream&
operator<<(std::ostream &ost, const Options &opt) {
  ost << "Station(s):";

  if (opt.stations.empty()) {
    ost << " All" << endl;
  } else {
    for (Options::CIList it = opt.stations.begin(); it != opt.stations.end();
        it++)
      ost << " " << *it;
    ost << endl;
  }

  ost << "Typeid(s):";

  if (opt.typeids.empty()) {
    ost << " All" << endl;
  } else {
    for (Options::CIList it = opt.typeids.begin(); it != opt.typeids.end();
        it++)
      ost << " " << *it;
    ost << endl;
  }

  ost << "Fromtime: " << opt.fromtime << endl;
  ost << "Totime:   " << opt.totime << endl;

  return ost;
}

