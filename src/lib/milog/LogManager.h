/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: LogManager.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_logmanager_h__
#define __milog_logmanager_h__

#include <milog/thread/Thread.h>
#include <milog/private/LoggerImpl.h>
#include <milog/private/types.h>
#include <milog/private/ThreadLogManager.h>

namespace milog {

/**
 * \addtogroup milog
 *
 *  @{
 */

class Logger;
class LogStream;

/**
 * \brief The LogManager maintains a global list of loggers.
 *
 * When a logger is added to this global list it can never be removed. 
 * Ie. it persist through out the life time of the program.
 */
class LogManager {

  LogManager(const LogManager &);
  LogManager& operator=(const LogManager &);

  friend class priv::ThreadLogManager;

  static milog::LogManager *logManager;
  static milog::thread::Mutex mutex_;
  milog::priv::TLoggerImplList loggers_;
  milog::priv::LoggerImpl *traceDefaultLogger;
  LogLevel logLevel_;
  bool enabled_;

  priv::LoggerImpl* getLogger(const std::string &id);

  LogManager();
  ~LogManager();

 public:
  static LogManager *instance();

  /**
   * \brief Add a logger to the global list of loggers. 
   *
   * If a logger with the same \em id exist, the function return false.
   * A logger that is added to the global list of loggers
   * persist to the program terminates. It cant be removed.
   *
   * \param id the name the logger shall be known as.
   * \param strm The LogStream to send log messages to.
   * \return true if the logger was created, false othewise.
   */
  static bool createLogger(const std::string &id, LogStream *strm);

  /**
   * Check if a logger with a given id exist.
   *
   * @param id The logger to check for.
   * @return true if an logger for the id exist and false otherwise.
   */
  static bool hasLogger(const std::string &id);

  /**
   * \brief Add a stream to the default logger.
   */
  static bool addStream(LogStream *strm);

  /**
   * \brief Add a stream to the logger 'id'.
   */
  static bool addStream(const std::string &id, LogStream *strm);

  /**
   * \brief Set the default logger at start up to the default logger.
   *
   * The default logger logs to std::err with the Layout set to
   * TraceLayut.
   */
  static bool resetDefaultLogger();

  /**
   * \brief Set the default logger to the logger associated with \em id.
   *
   * The logger must exist.
   *
   * \param id The id of the logger that we wan't as the default logger.
   * \return true false if the logger don't exist. True on success.
   */
  static bool setDefaultLogger(const std::string &id);

  static bool enabled();
  static void enabled(bool enabled_);

  static LogLevel loglevel();
  static void loglevel(LogLevel ll);
};

/** @} */
}

#endif
