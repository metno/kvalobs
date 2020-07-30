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
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <mutex>
#include "boost/thread.hpp"
#include "lib/milog/milog.h"
#include "lib/miconfparser/miconfparser.h"
#include "lib/fileutil/pidfileutil.h"
#include "lib/kvalobs/kvPath.h"
#include "lib/decodeutility/kvalobsdataserializer.h"
#include "kvDataInputd/DataSrcApp.h"
#include "kvDataInputd/DecodeCommand.h"
#include "kvDataInputd/InitLogger.h"
#include "kvDataInputd/ObservationHandler.h"

using std::string;
using std::endl;

extern volatile sig_atomic_t sigTerm;
static void sig_term(int signal);
static void sig_abort(int signal);
static void setSigHandlers();
static void ignoreSigAbort();
static sigjmp_buf jmpbuf;
static volatile sig_atomic_t canjump;

namespace kvdatainput {
namespace decodecommand {
boost::thread_specific_ptr<kvalobs::decoder::RedirectInfo> ptrRedirect;
}
}

void loghHttpError(const std::string &msg) {
  IDLOGERROR("http_error", msg);
}

void loghHttpAccess(const std::string &msg) {
  IDLOGINFO("http_access", msg);
}

int main(int argn, char** argv) {
  kvalobs::serialize::KvalobsDataSerializer::defaultProducer = "kvinput";
  bool error;
  string pidfile;
  miutil::conf::ConfSection *theKvConf = KvBaseApp::getConfiguration();
  
  if( ! theKvConf ) {
    LOGFATAL("No configuration file!");
    exit(1);
  }
  
  int nWorkerThreads = 3;
  canjump = 0;
  InitLogger(argn, argv, "kvDataInputd", theKvConf);

  pidfile = KvBaseApp::createPidFileName("kvDataInputd");

  if (dnmi::file::isRunningPidFile(pidfile, error)) {
    if (error) {
      LOGFATAL(
          "An error occured while reading the pidfile:" << endl << pidfile << " remove the file if it exists and" << endl << "kvDataInputd is not running. " << "If it is running and there are problems. Kill kvDataInputd and" << endl << "restart it." << endl << endl);
      return 1;
    } else {
      LOGFATAL("Is kvDataInputd allready running?" << endl << "If not remove the pidfile: " << pidfile);
      return 1;
    }
  }

  setSigHandlers();
  KvBaseApp::createPidFile("kvDataInputd");
  DataSrcApp app(argn, argv, nWorkerThreads, theKvConf);
  ObservationHandler observationHandler(app, app.getRawQueue());

  if (!app.isOk()) {
    LOGFATAL("Problems with initializing of kvDataInputd!\n");
  }

  HttpConfig httpConfig = app.getHttpConfig();

  httpserver::webserver ws = httpserver::webserver(
      httpserver::create_webserver(httpConfig.port).max_threads(httpConfig.threads).log_error(loghHttpError).log_access(loghHttpAccess));
  ws.register_resource("/v1/observation", &observationHandler, false);

  ws.start(false);
  if (!ws.is_running()) {
    LOGFATAL("Cant start the http interface on port " << httpConfig.port << ", threads " << httpConfig.threads << ".");
    app.deletePidFile();
    return 1;
  }

  while (sigTerm == 0) {
    sleep(1);
  }

  // Hack to avoid that the webserver abort the application
  // on exit if a close on a socket fails.
  // We setup for a longjump back from the signalhandler
  // for SIGABRT.

  if (ws.is_running()) {
    ignoreSigAbort();
    if (sigsetjmp(jmpbuf, 1) == 0) {
      canjump = 1;
      ws.stop();
    }
  }

  app.shutdown();
  app.deletePidFile();
  return 0;
}

void sig_abort(int sig) {
  // Hack to avoid that the webserver abort the application
  // on exit if a close on a socket fails.
  if (canjump) {
    canjump = 0;
    siglongjmp(jmpbuf, 1);
  }
}

void ignoreSigAbort() {
  sigset_t oldmask;
  struct sigaction act, oldact;

  act.sa_handler = sig_abort;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (sigaction(SIGABRT, &act, &oldact) < 0) {
    LOGFATAL("ERROR: Can't install signal handler for SIGABRT\n");
    exit(1);
  }
}

void setSigHandlers() {
  sigset_t oldmask;
  struct sigaction act, oldact;

  act.sa_handler = sig_term;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (sigaction(SIGTERM, &act, &oldact) < 0) {
    LOGFATAL("ERROR: Can't install signal handler for SIGTERM\n");
    exit(1);
  }

  act.sa_handler = sig_term;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (sigaction(SIGINT, &act, &oldact) < 0) {
    LOGFATAL("ERROR: Can't install signal handler for SIGTERM\n");
    exit(1);
  }
}

void sig_term(int signal) {
  sigTerm = 1;
}
