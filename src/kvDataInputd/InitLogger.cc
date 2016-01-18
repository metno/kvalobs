/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "lib/kvalobs/getLogInfo.h"
#include "lib/kvalobs/kvPath.h"
#include "lib/milog/milog.h"
#include "kvDataInputd/InitLogger.h"


using std::string;
using std::cerr;
using milog::LogManager;
using milog::StdErrStream;
using milog::LogLevel;
using milog::FLogStream;

void InitLogger(int argn, char **argv, const std::string &logname,
                miutil::conf::ConfSection *conf) {
  string filename;
  LogLevel traceLevel = milog::NOTSET;
  LogLevel logLevel = milog::NOTSET;
  int nRotate = 4;
  int fileSize = 102400;

  filename = kvPath("logdir") + "/" + logname + ".log";

  for (int i = 0; i < argn; i++) {
    if (strcmp("--tracelevel", argv[i]) == 0) {
      i++;

      if (i < argn) {
        traceLevel = loglevelFromString(argv[i]);
      }
    } else if (strcmp("--loglevel", argv[i]) == 0) {
      i++;

      if (i < argn) {
        logLevel = loglevelFromString(argv[i]);
      }
    }
  }

  getLogfileInfo(conf, "kvDataInputd", nRotate, fileSize);

  if (traceLevel == milog::NOTSET && conf)
    traceLevel = getTracelevelRecursivt(conf, logname, milog::DEBUG);

  if (logLevel == milog::NOTSET && conf)
    logLevel = getLoglevelRecursivt(conf, logname, milog::WARN);

  try {
    FLogStream *fs = new FLogStream(nRotate, fileSize);

    if (!fs->open(filename)) {
      cerr << "FATAL: Can't initialize the Logging system.\n";
      cerr << "------ Cant open the Logfile <" << filename << ">\n";
      delete fs;
      exit(1);
    }

    StdErrStream *trace = new StdErrStream();

    if (!LogManager::createLogger(logname, trace)) {
      cerr << "FATAL: Can't initialize the Logging system.\n";
      cerr << "------ Cant create logger\n";
      exit(1);
    }

    if (!LogManager::addStream(logname, fs)) {
      cerr << "FATAL: Can't initialize the Logging system.\n";
      cerr << "------ Cant add filelogging to the Logging system\n";
      exit(1);
    }

    trace->loglevel(traceLevel);
    fs->loglevel(logLevel);

    LogManager::setDefaultLogger(logname);
  } catch (...) {
    cerr << "FATAL: Can't initialize the Logging system.\n";
    cerr << "------ OUT OF MEMMORY!!!\n";
    exit(1);
  }

  cerr << "Logging to file <" << filename << ">!\n";
}
