/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2016 met.no

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

#include <termios.h>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include "kvalobs/kvPath.h"
#include "ManagerApp.h"
#include "MissingRunner.h"
#include "WorkQueueMover.h"
#include "ObservationComplete.h"

namespace {
void stop(int signum) {
  TimedDatabaseTask::stop();
  ObservationComplete::stop();
}

void setStopSignals() {
  static struct sigaction handler;
  handler.sa_handler = stop;
  sigemptyset(&handler.sa_mask);
  handler.sa_flags = 0;
  sigaction(SIGINT, &handler, nullptr);
}

void setupLogging() {
  std::string logfile = kvalobs::kvPath(kvalobs::logdir) + "/kvManagerd.log";

  LOGINFO("Logging to " << logfile);

  milog::FLogStream* fs = new milog::FLogStream(9, 1024 * 1024);
  fs->open(logfile);
  if (!milog::LogManager::createLogger("filelog", fs)) {
    LOGERROR("Unable to create logger for file " << logfile);
    delete fs;
    return;
  }
  if (!milog::LogManager::setDefaultLogger("filelog"))
    LOGERROR("Unable to register file logger as default logger");
}
}  // namespace

bool log_to_stdout(int argc, char ** argv) {
  const std::string option = "--log-to-stdout";
  for (int i = 1; i < argc; ++i)
    if (option == argv[i])
      return true;
  return false;
}

int main(int argc, char ** argv) {
  try {
    if (! log_to_stdout(argc, argv))
      setupLogging();
    setStopSignals();

    ManagerApp app(argc, argv);
    ManagerApp::PidFile pidFile("kvManagerd");
    bool checkForMissingObs=app.checkForMissisngObs(false);
    std::string connectString = app.createConnectString();
    ObservationComplete observationComplete(connectString);
    MissingRunner missingRunner(connectString, checkForMissingObs);
    WorkQueueMover workQueueMover(connectString);

    LOGDEBUG("Starting manager");
    LOGINFO("Check for missing observations '" << (checkForMissingObs?"enabled":"disabled") << "'.");
    std::thread missingThread(missingRunner);
    std::thread moverThread(workQueueMover);
    observationComplete();
    missingThread.join();
    moverThread.join();
  } catch (std::exception & e) {
    LOGFATAL(e.what());
    return 1;
  }
}
