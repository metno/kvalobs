/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: miloghelper.h,v 1.1.2.4 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_miloghelper_h__
#define __milog_miloghelper_h__

#include <sstream>
#include <milog/Logger.h>

namespace milog {
/**
 * \addtogroup milog
 * @{
 */

/**
 * \brief A convenient helper class to create log message
 * with a stream interface.
 */
class Log : public std::ostringstream {
  Logger &log;
  bool popContext;
  LogLevel logLevel;

 public:
  Log(LogLevel ll, const std::string &context = "")
      : log(milog::Logger::logger()),
        popContext(false),
        logLevel(ll) {
    if (!context.empty()) {
      log.push(context);
      popContext = true;
    }

  }
  Log(const std::string &id, LogLevel ll, const std::string &context = "")
      : log(milog::Logger::logger(id)),
        popContext(false),
        logLevel(ll) {
    if (!context.empty()) {
      log.push(context);
      popContext = true;
    }

  }

  virtual ~Log() {
    log.log(logLevel, str());

    if (popContext)
      log.pop();
  }
};

/**
 * \brief A convinent class to log milog::FATAL messages.
 */
class LogFatal : public Log {
 public:
  LogFatal(const std::string &context = "")
      : Log(FATAL, context) {
  }

  LogFatal(const std::string &id, const std::string &context)
      : Log(id, FATAL, context) {
  }
};

/**
 * \brief A convinent class to log milog::ERROR messages.
 */
class LogError : public Log {
 public:
  LogError(const std::string &context = "")
      : Log(ERROR, context) {
  }

  LogError(const std::string &id, const std::string &context)
      : Log(id, ERROR, context) {
  }
};

/**
 * \brief A convinent class to log milog::WARN messages.
 */
class LogWarn : public Log {
 public:
  LogWarn(const std::string &context = "")
      : Log(WARN, context) {
  }

  LogWarn(const std::string &id, const std::string &context)
      : Log(id, WARN, context) {
  }
};

/**
 * \brief A convinent class to log milog::INFO messages.
 */
class LogInfo : public Log {
 public:
  LogInfo(const std::string &context = "")
      : Log(INFO, context) {
  }

  LogInfo(const std::string &id, const std::string &context)
      : Log(id, INFO, context) {
  }
};

/**
 * \brief A convinent class to log milog::DEBUG messages.
 */
class LogDebug : public Log {
 public:
  LogDebug(const std::string &context = "")
      : Log(DEBUG, context) {
  }

  LogDebug(const std::string &id, const std::string &context)
      : Log(id, DEBUG, context) {
  }
};

/**
 * \brief A convinent class to log milog::DEBUG1 messages.
 */
class LogDebug1 : public Log {
 public:
  LogDebug1(const std::string &context = "")
      : Log(DEBUG1, context) {
  }

  LogDebug1(const std::string &id, const std::string &context)
      : Log(id, DEBUG1, context) {
  }
};

/**
 * \brief A convinent class to log milog::DEBUG2 messages.
 */
class LogDebug2 : public Log {
 public:
  LogDebug2(const std::string &context = "")
      : Log(DEBUG2, context) {
  }

  LogDebug2(const std::string &id, const std::string &context)
      : Log(id, DEBUG2, context) {
  }
};

/**
 * \brief A convinent class to log milog::DEBUG3 messages.
 */
class LogDebug3 : public Log {
 public:
  LogDebug3(const std::string &context = "")
      : Log(DEBUG3, context) {
  }

  LogDebug3(const std::string &id, const std::string &context)
      : Log(id, DEBUG3, context) {
  }
};

/**
 * \brief A convinent class to log milog::DEBUG4 messages.
 */
class LogDebug4 : public Log {
 public:
  LogDebug4(const std::string &context = "")
      : Log(DEBUG4, context) {
  }

  LogDebug4(const std::string &id, const std::string &context)
      : Log(id, DEBUG4, context) {
  }
};

/**
 * \brief A convinent class to log milog::DEBUG5 messages.
 */
class LogDebug5 : public Log {
 public:
  LogDebug5(const std::string &context = "")
      : Log(DEBUG5, context) {
  }

  LogDebug5(const std::string &id, const std::string &context)
      : Log(id, DEBUG5, context) {
  }
};

/**
 * \brief A convinent class to log milog::DEBUG messages.
 */
class LogDebug6 : public Log {
 public:
  LogDebug6(const std::string &context = "")
      : Log(DEBUG6, context) {
  }

  LogDebug6(const std::string &id, const std::string &context)
      : Log(id, DEBUG6, context) {
  }
};

/**
 * \brief A convinent class to maintain the context stack.
 *
 * On construction time a context is pushed on the context stack. The
 * detructor pop the context of the stack.
 */
class LogContext {
  Logger &log;
  bool popContext;

 public:
  LogContext(const std::string &context)
      : log(milog::Logger::logger()),
        popContext(false) {
    if (!context.empty()) {
      log.push(context);
      popContext = true;
    }

  }
  LogContext(const std::string &id, const std::string &context)
      : log(milog::Logger::logger(id)),
        popContext(false) {
    if (!context.empty()) {
      log.push(context);
      popContext = true;
    }
  }

  virtual ~LogContext() {
    if (popContext)
      log.pop();
  }
};

/**
 * \brief Helper class to set a new file temporarly as default logfile.
 * After an instance of the class goes out of scope the original logfile
 * is reset as default logfile.
 */
class SetResetDefaultLoggerHelper {
  bool reset;

  void init(const std::string &logfile,
            milog::LogLevel loglevel = milog::NOTSET,
            long sizeInBytes = 1048576, int nRotate = 1);

 public:
  SetResetDefaultLoggerHelper(const std::string &logfile,
                              milog::LogLevel loglevel = milog::NOTSET,
                              long sizeInBytes = 1048576, int nRotate = 1) {
    init(logfile, loglevel, sizeInBytes, nRotate);
  }

  SetResetDefaultLoggerHelper(const std::string &logfile, long sizeInBytes =
                                  1048576,
                              int nRotate = 1) {
    init(logfile, milog::NOTSET, sizeInBytes, nRotate);
  }

  ~SetResetDefaultLoggerHelper();
};

/** @} */

}

#endif
