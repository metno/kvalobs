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

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <milog/milog.h>
#include <kvalobs/kvStationInfo.h>
#include <string>
#include <iosfwd>
#include <limits>

namespace boost {
namespace program_options {
class options_description;
}
}

namespace qabase {

/**
 * Parsing and storing command-line options.
 *
 * \todo integrate this with kvalobs default options.
 *
 * \ingroup group_control
 */
class Configuration {
 public:

  /**
   * Construct based on command-line arguments.
   *
   * @param argc Argument count
   * @param argv Argument values
   */
  Configuration(int & argc, char ** argv);
  ~Configuration();

  /**
   * Did any of the command-line arguments indicate that the user did not
   * want to run checks? This is true if the user gave command-line
   * arguments, such as --help.
   * @return False if program should terminate immediately, without error.
   *         Otherwise true.
   */
  bool runNormally() const {
    return runNormally_;
  }

  /**
   * Get database connection specification, for use when establishing
   * database connection.
   * @return Connection string for database.
   */
  std::string databaseConnectString() const;

  /**
   * Did the user specify a specific observation to check?
   * @return true if user only wants to check a single observation. False otherwise.
   */
  bool haveObservationToCheck() const {
    return observationToCheck_;
  }

  /**
   * Get the single observation to check
   * @return Observation the user wants to check, or NULL if
   * haveObservationToCheck() returns false.
   */
  const kvalobs::kvStationInfo * observationToCheck() const {
    return observationToCheck_;
  }

  /**
   * Has the user specified a single qcx check to run?
   * @return tru if user have specified a singe qcx
   */
  bool onlySpecificQcx() const {
    return not qcxFilter_.empty();
  }

  typedef std::vector<std::string> QcxFilter;

  /**
   * Get the list of qcx checks the user wants to run.
   * @return qcx list, or an empty list if onlySpecificQcx()==false.
   */
  const QcxFilter & qcxFilter() const {
    return qcxFilter_;
  }

  /**
   * Model data should be fetched from this source (name in kvalobs' model table)
   */
  const std::string & modelDataName() const {
    return modelDataName_;
  }

  const std::string & runLogFile() const {
    return runLogFile_;
  }

  /**
   * Get the desired logging level.
   * @return Selected logging level.
   */
  milog::LogLevel logLevel() const {
    return logLevel_;
  }

  /**
   * Get the base directory for creating script logs.
   * @return Base log directory.
   */
  const std::string & baseLogDir() const {
    return baseLogDir_;
  }

  unsigned processCount() const {
    return processCount_;
  }


  /**
   * @brief Max errors we accept when trying to send to kafka.
   * The process kill it self if the number of error exceed 
   * this value. 
   * 
   * This value has only effect if the process count is 1.
   * 
   * @return unsigned 
   */
  unsigned maxKafkaSendErrors() const {
    if( maxKafkaSendErrors_==0 || processCount_>1) {
      return std::numeric_limits<unsigned>::max();
    } 
    return maxKafkaSendErrors_;
  }

  bool kafkaEnabled() const;
  /**
   * The size of the logfile in bytes before it is created a backup file. 
   */
  int logSize() const {
    return logSize_*1024*1024;
  }

  /**
   * Number of backup logfiles to keep.
   */
  int numberOfLogs() const {
    return numberOfLogs_;
  }

  int selectForControlCount()const {
    return selectForControlCount_;
  }

  bool logXml()const {
    return logXml_;
  }

  int id() const {
    return id_;
  }

  /**
   * Print version information to the given stream.
   *
   * @param s The stream to write to
   * @return same stream as given in argument.
   */
  std::ostream & version(std::ostream & s) const;


  /**
   * Print help information to the given stream.
   *
   * @param s The stream to write to
   * @param options Option list to write help information about.
   * @return same stream as given in argument.
   */
  std::ostream & help(
      std::ostream & s,
      const boost::program_options::options_description & options) const;

 private:
  bool runNormally_;

  kvalobs::kvStationInfo * observationToCheck_;

  QcxFilter qcxFilter_;

  std::string modelDataName_;

  std::string runLogFile_;
  milog::LogLevel logLevel_;
  std::string baseLogDir_;
  bool        logXml_;

  std::string databaseName_;
  std::string host_;
  int port_;
  std::string user_;
  int logSize_;
  int numberOfLogs_;
  int selectForControlCount_;
  unsigned maxKafkaSendErrors_;

  unsigned processCount_;
  int id_;
  bool kafkaDisabled_;
 
};

}

#endif /* CONFIGURATION_H_ */
