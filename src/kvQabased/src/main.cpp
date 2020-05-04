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

#include <sys/prctl.h>
#include <boost/lexical_cast.hpp>
#include <csignal>
#include <string>
#include "decodeutility/kvalobsdata.h"
#include "decodeutility/kvalobsdataserializer.h"
#include "fileutil/pidfileutil.h"
#include "kvsubscribe/KafkaProducer.h"
#include "kvsubscribe/DataSubscriber.h"
#include "milog/FLogStream.h"
#include "milog/milog.h"
#include "CheckRunner.h"
#include "Configuration.h"
#include "DataProcessor.h"
#include "Exception.h"
#include "LogFileCreator.h"
#include "NewDataListener.h"
#include "QaBaseApp.h"
#include "scriptcreate/KvalobsCheckScript.h"
#include "db/KvalobsDatabaseAccess.h"

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
 * \ref group_kafka "here".
 */

using namespace boost::program_options;

namespace {
void terminate(int /*signal*/) {
  qabase::NewDataListener::stopAll();
}

/**
 * Information about the current process instance (which part of the fork() are we)
 */
class ProcessStatus {
 public:
  ProcessStatus() :id_(0) {}
  explicit ProcessStatus(int id) :id_(id) {}

  /**
   * Is this the "original" instance?
   */
  bool isLeader() const {
    return id_ == 0;
  }
  int id() const {
    return id_;
  }
 private:
  int id_;
};

ProcessStatus spawnSubProcesses(unsigned totalWorkerCount) {
  for (int i = 1; i < totalWorkerCount; ++i)
    if (fork() == 0) {
      prctl(PR_SET_PDEATHSIG, SIGHUP);  // die on parent death
      return ProcessStatus(i);
    }
  return ProcessStatus(0);
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
      throw std::runtime_error("Is kvQabased already running? If not remove the pidfile: " + pidfile);
  }
  if (!dnmi::file::createPidFile(pidfile))
    throw std::runtime_error("Unable to create pidfile!");
}

void setupLogging(const qabase::Configuration& config, const ProcessStatus & ps) {
  if (!config.runLogFile().empty()) {
    std::ostringstream logFile;
    std::ostringstream kafkaLogFile;
    logFile << config.runLogFile();
    std::string tmp =config.runLogFile();
    std::string::size_type i=tmp.find_last_of(".");

    if( i != std::string::npos ) {
      tmp.erase(i);
    }
    
    kafkaLogFile << tmp << "_kafka.log";

    if (!config.haveObservationToCheck()) {
      logFile << '.' << ps.id();
      kafkaLogFile << "." << ps.id();
    }
          
    std::string logFileName = logFile.str();
    milog::FLogStream* fs = new milog::FLogStream(config.numberOfLogs(), config.logSize());
    fs->open(logFileName);
    fs->loglevel(config.logLevel());
    if (!milog::LogManager::createLogger("filelog", fs)) {
      LOGERROR("Unable to create logger for file " << logFileName);
      delete fs;
      return;
    }
    if (!milog::LogManager::setDefaultLogger("filelog"))
      LOGERROR("Unable to register file logger as default logger");

    //Kafka logger
    logFileName = kafkaLogFile.str();
    milog::FLogStream* kafkaFs = new milog::FLogStream(1, 128*1024*1024); //128Mb
    kafkaFs->open(logFileName);
    kafkaFs->loglevel(milog::INFO);
    if (!milog::LogManager::createLogger("kafka", kafkaFs)) {
      LOGERROR("Unable to create logger for file " << logFileName);
      delete fs;
      return;
    }
  }
}

}

int main(int argc, char ** argv) {
  kvalobs::serialize::KvalobsDataSerializer::defaultProducer = "kvqabase";
  try {
    qabase::Configuration config(argc, argv);
    if (not config.runNormally())
      return 0;

    ProcessStatus processStatus;
    if (!config.haveObservationToCheck())
      processStatus = spawnSubProcesses(config.processCount());
    setupLogging(config, processStatus);
    qabase::QaBaseApp app(argc, argv);

    LOGINFO("Log xml sendt to kafka: " << (config.logXml()?"true":"false"));
    LOGDEBUG("Using model data name " << config.modelDataName());

    qabase::DataProcessor::logXml = config.logXml();
    db::KvalobsDatabaseAccess::setModelDataName(config.modelDataName());

    if (config.haveObservationToCheck()) {
      auto checkRunner = qabase::CheckRunner::create(config.databaseConnectString());

      if (config.onlySpecificQcx())
        checkRunner->setQcxFilter(config.qcxFilter().begin(), config.qcxFilter().end());

      qabase::DataProcessor processor(checkRunner);
      processor.process(*config.observationToCheck());
    } else {
      if (processStatus.isLeader())
        createPidFile();

      signal(SIGINT, terminate);
      signal(SIGHUP, terminate);

      std::string dbConnect = qabase::QaBaseApp::createConnectString();
      LOGDEBUG("Connecting to database: " << dbConnect);
      auto db = std::make_shared<db::KvalobsDatabaseAccess>(dbConnect);
      auto checkRunner = std::make_shared<qabase::CheckRunner>(db);
      qabase::NewDataListener listener(db, config.selectForControlCount());
      listener.run();
    }
  } catch (std::exception & e) {
    LOGFATAL(e.what());
    return 1;
  }
}
