/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id$                                                       

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

#include <stdlib.h>
#include <fstream>
#include <milog/milog.h>
#include <miconfparser/miconfparser.h>
#include <fileutil/pidfileutil.h>
#include "App.h"
#include <kvalobs/kvPath.h>

using namespace std;

int main(int argn, char **argv) {
  string pidfile;
  string confile;
  bool pidfileError;

  confile = kvalobs::kvPath(kvalobs::sysconfdir) + "/kv2kvDataInputd.conf";
  miutil::conf::ConfSection *conf = miutil::conf::ConfParser::parse(confile);

  if (!conf) {
    cerr << "Failed to read the configuration file <" << confile << ">."
         << endl;
    string logfile = kvalobs::kvPath(kvalobs::logdir) + "/kv2kvDataInputd.log";
    ofstream log(logfile.c_str(), ios_base::out | ios_base::app);

    if (log) {
      log << "Failed to read the configuration file <" << confile << ">."
          << endl;
      log.close();
    }
  }

  App::createLogger(conf);

  pidfile = dnmi::file::createPidFileName(kvalobs::kvPath(kvalobs::rundir),
                                          "kv2kvDataInputd");

  if (dnmi::file::isRunningPidFile(pidfile, pidfileError)) {
    if (pidfileError) {
      LOGFATAL(
          "An error occured while reading the pidfile:" << endl << pidfile << " remove the file if it exist and" << endl << "kv2kvDataInputd is not running. If it is running and" << endl << "there is problems. Kill kv2kvDataInputd and" << endl << "restart it." << endl << endl);
      return 1;
    } else {
      LOGFATAL(
          "Is kv2kvDataInputd allready running?" << endl << "If not remove the pidfile: " << pidfile << endl << endl);
      return 1;
    }
  }

  //The pidfile is automatically removed when `thePidfile` goes
  //out of scope, ie when the main function end.
  dnmi::file::PidFileHelper thePidfile(pidfile);

  App app(argn, argv, conf);
  app.run();
}
