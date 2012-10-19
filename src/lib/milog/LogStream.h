/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: LogStream.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_logstream_h__
#define __milog_logstream_h__

#include <string>
#include <milog/milogtypes.h>

namespace milog{
  class Layout;

  /**
   * \addtogroup milog
   *
   * @{
   */

  /**
   * \brief A baseclass to use to implement log streams.
   * 
   * A log stream is the actual logging medium This can be files
   * sockets etc.
   *
   * Every loggstream must have an accosiated LayOut instance.
   *
   * You must never call the function in a LogStream directly. A LogStream 
   * must be accosiated with a logger before it can be used. All access
   * to the LogStream must go through a Logger instance.
   */
  class LogStream{
    
    LogStream(const LogStream &);
    LogStream& operator=(const LogStream &);

    Layout   *layout_;
    LogLevel loglevel_;

  protected:
    LogStream();
    virtual void write(const std::string &message)=0; 
    void layout(Layout *layout);

  public:
   
    LogStream(Layout *layout, LogLevel ll=milog::NOTSET);
    virtual ~LogStream();
    
    /**
     * \brief massage to log. 
     *
     * The message is first formated by the 
     * Layout::formatMessage(const std::string &msg,LogLevel ll,const std::string &context)
     * before the message is writen to the stream.
     *
     * \param msg The log mesaage.
     * \param ll The threshold value.
     * \param context The context string.
     */
    void message(const std::string &msg, 
		 LogLevel ll,
		 const std::string &context
		 );

    
    /**
     * \brief Get the log level threshold.
     */
    LogLevel loglevel()const{ return loglevel_;}

    /**
     * \brief Set the log level threshold.
     */
    void     loglevel(LogLevel ll){ loglevel_=ll;}
  };

  /** @} */
}



#endif
