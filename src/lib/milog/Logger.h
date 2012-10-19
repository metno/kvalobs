/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Logger.h,v 1.1.2.3 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_logger_h__
#define __milog_logger_h__

#include <string>
#include <milog/milogtypes.h>
#include <milog/private/types.h>
#include <milog/private/ThreadLogManager.h>
#include <sstream>

namespace milog {

  namespace priv{
    class LoggerImpl;
  }

  /**
   * \addtogroup milog
   *
   * @{
   */

  class LogStream;

  /**
   * \brief The logger class.
   */
  class Logger{
    static Logger dummyLogger;
    milog::priv::LoggerImpl    *logger_;
    milog::priv::LoggerImpl    *thrLocaleLoggers_;
    LogLevel                   level_;
    bool                       enabled_;
    bool                       ownerOfLogger;



    friend class milog::priv::ThreadLogManager;
    
    Logger(milog::priv::LoggerImpl     *logger, 
	   LogLevel                    ll,
	   bool                        enabled);

    ///Creates an logger that logs to std::err.
    Logger(); 
    
    ///Creates an logger taht logs to logStream.
    Logger(LogStream *logStream);

   ~Logger();

  public:
    /**
     * \brief Register a LogStream in the current thread. 
     *
     * The logger will be known under the name \em id. The milog takes over 
     * the resonsibility of the LogStream \em logStrm. 
     * 
     * Every thread maintain its own context stack.
     * 
     * \note WARNING dont use the \em logStrm directly after it is registred
     * with setLogger. The logger is only known in the thread that called
     * Logger::setLogger. When the logger is not needed anymore it can
     * be removed with remveLogger.
     *
     * 
     */
    static bool createLogger(const std::string &id, LogStream *logStrm);
    

    /**
     * \brief Set a LogStream as the default logger in the current thread. 
     *
     * The milog takes over the resonsibility of the LogStream \em logStrm. 
     * 
     * Every thread maintain its own context stack.
     * 
     * \note WARNING dont use the \em logStrm directly after it is registred
     * with the Logger. The logger is only known in the thread that called
     * Logger::setLogger. When you want to reset the default logger
     * to the application default logger call resetDefaultLogger.
     */

    static bool setDefaultLogger(LogStream *logStrm);
    
    static void resetDefaultLogger();

    /**
     * \brief remove the logger identified with the \em id.
     */
    static void removeLogger(const std::string &id);
    
 	 ///Push a logstream to the default logger.
	 static  void    pushLogStream(LogStream *strm);
	    
	 ///Remove the log stream at the top of the default stack.
	 ///It will not remove loggers added to the globale
	 ///log manager.
	 static void    popLogStream();
    

    /**
     * \brief Get the global logger for the program. 
     */
    static Logger& logger();
    
    /**
     * \brief Get the logger identified with \em id.
     */
    static Logger& logger(const std::string &id);

    /**
     * \brief Set the loglevel.
     *
     * \param ll The new log level.
     * \return The old loglevel.
     */
    LogLevel logLevel(LogLevel ll);
    
    /**
     * \brief Get the loglevel.
     */
    LogLevel logLevel()const { return level_;}

    /**
     * \brief Log a message.
     *
     * The message is logged with the loglevel \em ll.
     *
     * \param ll The threshold value to use when logging the message \em msg.
     * \param msg The log message.
     */
    void log(LogLevel ll, const std::string &msg);
    
    /**
     * \brief An convenient function to log function.
     * 
     * Threshold value milog::FATAL.
     */
    void fatal(const std::string &msg){ log(FATAL, msg);}

    /**
     * \brief An convenient function to log function.
     * 
     * Threshold value milog::WARN.
     */
    void warn(const std::string &msg) { log(WARN, msg); }
    
    /**
     * \brief An convenient function to log function.
     * 
     * Threshold value milog::ERROR.
     */
    void error(const std::string &msg){ log(ERROR, msg);}
    
    /**
     * \brief An convenient function to log function.
     * 
     * Threshold value milog::INFO.
     */
    void info(const std::string &msg) { log(INFO, msg); }
    
    /**
     * \brief An convenient function to log function.
     * 
     * Threshold value milog::DEBUG.
     */
    void debug(const std::string &msg){ log(DEBUG, msg);}

    /**
     * \brief Get the context string.
     */
    static std::string getContextString();
    
    /**
     * \brief push a context string on the context stack.
     */
    static void push(const std::string &context);
    
    /**
     * \brief Pop the context stack, ie remove the top element from
     * the context stack.
     */
    static void pop();
    
    /**
     * \brief is the logger enabled.
     */
    bool enabled()const { return enabled_;}
  };
  
  /** @} */
};



#endif
