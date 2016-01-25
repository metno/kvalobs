/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvapp.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvbaseapp_h__
#define __kvbaseapp_h__

#include <exception>
#include <milog/milog.h>

namespace miutil {
namespace conf {
class ConfSection;
}
}

/**
 * \addtogroup kvinternalhelpers
 * @{
 */

/**
 * \brief This class encapsulate functions that is common for
 * most applications in kvalobs.
 */
class KvBaseApp {
  static std::string pidfile;
  static miutil::conf::ConfSection *conf;
  static std::string confFile;

 public:
  static milog::LogLevel globalLogLevel;
  bool setAppNameForDb;
  static std::string appName;  //Set the application name from ARGV[0].

  KvBaseApp(int argn, char **argv);
  virtual ~KvBaseApp();

  virtual void useMessage(std::ostream &os);
  void printUseMsgAndExit(int exitStatus);

  /**
   *  \brief get a pointer to the configuration data.
   *
   * \note  WARNING: dont delete the returned pointer.
   */
  static miutil::conf::ConfSection* getConfiguration();

  static milog::LogLevel getLogLevel(const std::string &section = "",
                                     miutil::conf::ConfSection *conf = 0);

  /**
   * \brief get the name of the configuration file that is used.
   *
   * The configuration file must be in $KVALOBS/etc path. Where
   * KVALOBS is a environment variable that must bes set.
   */
  static std::string getConfFile(const std::string &ifNotSetReturn =
      "kvalobs.conf");

  /**
   * \brief set the name of the configuration file.
   *
   * \note IMPORTENT: Call setConf before you instantiate the
   *    class if you want to use another configuration file than
   *       the filename to the application plus ".conf", ex kvDataInput.conf.
   * \verbatim
   Example

   class MyApp : public KvBaseApp{
   //Your stuff.
   }

   int
   main(int argn, char **argv){
   KvApp::setConfFile( my confile ); //MUST be before the instatiation of MyApp
   MyApp app();
   }
   \endverbatim
   */
  static void setConfFile(const std::string &filename);

  /**
   * Create a name for the pidfile. The name is on the form:
   *
   * progname-nodename.pid
   *
   * Where nodename is the name of the machine we are running on.
   *
   * @param progname the progname part of the pidfile name.
   * @return The name to use as the pidfile.
   */
  static std::string createPidFileName(const std::string &progname);
  static void createPidFile(const std::string &progname);
  static void deletePidFile();

  class PidFile {
   public:
    explicit PidFile(const std::string & progname);
    ~PidFile();
    PidFile(const PidFile & p) = delete;
    PidFile & operator=(const PidFile & p) = delete;
   private:
    std::string pidFile_;
  };

  /**
   * \brief creates a string that can be used to connect to the database.
   *
   * The function use the configuration file
   * $KVALOBS/etc/kvalobs.conf if it exist or it may get the different
   * values from the environment. It will always look at the configuration
   * file first.
   *
   * The environment variables that is used is:
   *
   *    KVDBUSER, the user we shall connect to the databse as.
   *              Default 'kvalobs'.
   *    KVDB,     the database to connect to. Default 'kvalobs'.
   *    PGHOST,   where is the databse. Default 'EMPTY STRING' this
   *              mean that we shall use the databse default.
   *    PGPORT,   at which port shall we connect to the database. Default
   *              'EMPTY STRING', this mean use the databse default.
   *
   * \param dbname, overides the above schema.
   * \param kvdbuser, overides the above schema.
   * \param host, overides the above schema.
   * \param port, overides the above schema.
   * \return the connection string.
   */

  static std::string createConnectString(const std::string &dbname = "",
                                         const std::string &kvdbuser = "",
                                         const std::string &host = "",
                                         const std::string &port = "");

};

/** @} */

#endif
