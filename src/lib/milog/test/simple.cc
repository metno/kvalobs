/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: simple.cc,v 1.4.6.1 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <milog/milog.h>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
using namespace milog;

int
main(int argn, char **argv)
{

  pid_t pid=getpid();

  FLogStream *fs=new FLogStream(new StdLayout());
  
  fs->open(std::string(argv[0])+".log");

  LogManager::addStream(fs);

  std::ostringstream ost;
  ost << "main(" << pid << ")";
  Logger::push(ost.str());

  Logger::push("warn");
  Logger::logger().warn("warn 1" );
  Logger::pop();

  Logger::push("error");
  Logger::logger().error("error 1");
  Logger::pop();

  Logger::push("info");
  Logger::logger().info("Info 1");
  Logger::pop();

  Logger::push("debug");
  Logger::logger().debug("Debug 1");
  Logger::pop();
  
  Logger::push("Fatal");
  Logger::logger().fatal("Fatal 1");
  Logger::pop();

  Logger::push("macros");

  { 
    LogContext ctx("warn");
    LOGWARN("warn");
  }

  { 
    LogContext ctx("error");
    LOGERROR("error");
  }

  { 
    LogContext ctx("info");
    LOGINFO("info");
  }

  { 
    LogContext ctx("debug");
    LOGDEBUG("debug");
  }

  Logger::pop();

  fs=new FLogStream(new StdLayout());
  
  fs->open("simpel-global.log");
  
  
  //Creates a Globale logger.
  LogManager::createLogger("file", fs);

  Logger::push("file(simple.log)");

  { 
    LogContext ctx("warn");
    IDLOGWARN("file","warn");
  }

  { 
    LogContext ctx("error");
    IDLOGERROR("file","error");
  }

  { 
    LogContext ctx("info");
    IDLOGINFO("file","info");
  }

  { 
    LogContext ctx("debug");
    IDLOGDEBUG("file","debug");
  }

  Logger::pop();

  //Creates a locale logger in the current thread.

  HtmlStream *html=new HtmlStream();
  
  html->open("simple.html");
 

  Logger::createLogger("html", html);

  Logger::push("html(simple.html)");

  { 
    LogContext ctx("warn");
    IDLOGWARN("html","warn");
  }

  { 
    LogContext ctx("error");
    IDLOGERROR("html","error");
  }

  { 
    LogContext ctx("info");
    IDLOGINFO("html","info");
  }

  { 
    LogContext ctx("debug");
    IDLOGDEBUG("html","debug");
  }

  Logger::pop();

  Logger::removeLogger("html");
  
}
