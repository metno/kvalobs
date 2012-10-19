/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: LoggerImpl.h,v 1.1.2.3 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_private_loggerimpl_h__
#define __milog_private_loggerimpl_h__

#include <milog/thread/Thread.h>
#include <milog/milogtypes.h>
#include <milog/private/types.h>
#include <milog/LogStream.h>

namespace milog{
  //  class LogStream;
 
    namespace priv{
	class LoggerImpl{
	    milog::thread::Mutex mutex_;
	    TLogStreamList       streamList;
	    
	public:
	    ///Creates an logger that logs to std::err.
	    LoggerImpl(); 
		
	    ///Creates an logger that logs to logStream.
	    LoggerImpl(LogStream *logStream);
    
	    ~LoggerImpl();

	    void log(const std::string &msg,
		     LogLevel ll, 
		     const std::string &context);
	
	    void addLogStream(LogStream *logStream);
	    
	    ///Pop the last added logsteram from the stream list.
	    ///Return the number of streams left.
	    int removeLogStream();
	    
	    
	};

    };
};



#endif
