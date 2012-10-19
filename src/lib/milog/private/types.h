/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: types.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_priv_types_h__
#define __milog_priv_types_h__

#include <list>
#include <string>
#include <map>

namespace milog{

    class Logger;
    class LogStream;

    namespace priv{
	class LoggerImpl;

	typedef std::list<std::string>                   TContextStack;
	typedef std::list<std::string>::iterator        ITContextStack;
	typedef std::list<std::string>::const_iterator CITContextStack;

	typedef std::map<std::string, Logger*>                   TLoggerList;
	typedef std::map<std::string, Logger*>::iterator        ITLoggerList;
	typedef std::map<std::string, Logger*>::const_iterator CITLoggerList;
	
	typedef std::map<std::string, LoggerImpl*>                   TLoggerImplList;
	typedef std::map<std::string, LoggerImpl*>::iterator        ITLoggerImplList;
	typedef std::map<std::string, LoggerImpl*>::const_iterator CITLoggerImplList;
    
	typedef std::list<LogStream*>                   TLogStreamList;
	typedef std::list<LogStream*>::iterator        ITLogStreamList;
	typedef std::list<LogStream*>::const_iterator CITLogStreamList;
}
}

#endif
