/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2010 met.no

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

#include "QaBaseApp.h"
#include "NewDataListener.h"
#include "Configuration.h"
#include "CheckRunner.h"
#include "DataProcessor.h"
#include "scriptcreate/KvalobsCheckScript.h"
#include "Exception.h"
#include "LogFileCreator.h"
#include "db/KvalobsDatabaseAccess.h"
#include <decodeutility/kvalobsdata.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <kvsubscribe/KafkaProducer.h>
#include <kvsubscribe/DataSubscriber.h>
#include <milog/milog.h>
#include <milog/FLogStream.h>
#include <fileutil/pidfileutil.h>
#include <boost/lexical_cast.hpp>
#include <csignal>
#include <sys/prctl.h>

/**
 * \mainpage qabase - Kvalobs quality script manager
 *
 * This is the code documentation for qabase, which is responsible for running
 * checks on weather observations as they arrive at kvalobs. Note that there
 * are at least two other programs that run checks on kvalobs data - QC2 and
 * HQC. You can read about these elsewhere.
 *
 * The qabase code is logically divided into several sections.
 *
 *
 * \section section_program_control Controlling qabase
 *
 * The controlling class for qabase is qabase::CheckRunner. This class, along
 * with the main supporting classes for qabase is documented \ref group_control "here".
 *
 * \section section_script_running Script creation and running
 *
 * The main functionality of qabase is to create and run scripts for checking
 * observations. Script creation is documented \ref group_scriptcreate "here".
 * The \ref group_scriptrunner "Script runner module" provides functionality
 * for running scripts once they are generated.
 *
 *
 * \section section_database Database access
 *
 * qabase uses a number of classes and functions for getting data out of and
 * into the kvalobs database. These classes and functions provide direct
 * database access, caching and filters for selecting and massaging data in
 * ways that was hard or impossible to to with SQL queries. An overview of the
 * details may be found in the \ref group_db "Database documentation".
 *
 *
 * \section section_daemon_mode Daemon mode
 *
 * There are two ways to run qabase. One is to provide command-line arguments,
 * specifying a single observation to check, while the other way is to run
 * qabase as a daemon. Details about daemon mode can be found
 * \ref group_corba "here".
 */

using namespace boost::program_options;

namespace {
void terminate(int /*signal*/) {
  qabase::NewDataListener::stopAll();
}

void spawnSubProcesses(unsigned totalWorkerCount) {
  for (int i = 1; i < totalWorkerCount; ++i)
    if (fork() == 0) {
      prctl(PR_SET_PDEATHSIG, SIGHUP);  // die on parent death
      break;
    }
}

void createPidFile() {
  std::string pidfile = qabase::QaBaseApp::createPidFileName("kvQabased");
  LOGDEBUG("pidfile: " << pidfile);
  bool error;
  if (dnmi::file::isRunningPidFile(pidfile, error)) {
    if (error)
      throw std::runtime_error(
          "An error occured while reading the pidfile:" + pidfile
              + " remove the file if it exist and kvQabased is not running.  If it is running and there is problems. Kill kvQabased and restart it.");
    else
      throw std::runtime_error(
          "Is kvQabased already running? If not remove the pidfile: "
              + pidfile);
  }
  if (!dnmi::file::createPidFile(pidfile))
    throw std::runtime_error("Unable to create pidfile!");
}

void setupLogging(const qabase::Configuration& config) {
  milog::Logger::logger().logLevel(milog::INFO);
  std::unique_ptr<milog::LogStream> s;
  if (!config.runLogFile().empty()) {
    milog::FLogStream* fs = new milog::FLogStream(9, 1024 * 1024);
    fs->open(config.runLogFile());
    fs->loglevel(config.logLevel());
    bool ok = milog::LogManager::createLogger("filelog", fs);
    ok = milog::LogManager::setDefaultLogger("filelog");
    s.reset(fs);
  }
}
}

int main(int argc, char ** argv) {
  milog::LogContext context("qabase");

  try {
    qabase::Configuration config(argc, argv);
    qabase::QaBaseApp app(argc, argv);
    if (not config.runNormally())
      return 0;

    setupLogging(config);

    LOGDEBUG("Using model data name " << config.modelDataName());
    db::KvalobsDatabaseAccess::setModelDataName(config.modelDataName());

    if (config.haveObservationToCheck()) {
      auto checkRunner = qabase::CheckRunner::create(
          config.databaseConnectString());

      if (config.onlySpecificQcx())
        checkRunner->setQcxFilter(config.qcxFilter().begin(),
                                  config.qcxFilter().end());

      qabase::DataProcessor processor(checkRunner);
      processor.process(*config.observationToCheck());
    } else {
      createPidFile();
      spawnSubProcesses(4);

      signal(SIGINT, terminate);
      signal(SIGHUP, terminate);

      std::string dbConnect = qabase::QaBaseApp::createConnectString();
      LOGDEBUG("Connecting to database: " << dbConnect);
      auto db = std::make_shared < db::KvalobsDatabaseAccess > (dbConnect);
      auto checkRunner = std::make_shared < qabase::CheckRunner > (db);

      qabase::NewDataListener listener(db);
      listener.run();
    }
  } catch (std::exception & e) {
    LOGFATAL(e.what());
    return 1;
  }
}
