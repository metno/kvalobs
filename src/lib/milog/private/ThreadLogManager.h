/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: ThreadLogManager.h,v 1.1.2.3 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_priv_threadlogmanager_h__
#define __milog_priv_threadlogmanager_h__

#include <milog/private/types.h>
#include <milog/thread/Thread.h>

namespace milog {

class Logger;

namespace priv {

/**
 * ThreadLogManager takes care of logger referances in 
 * thread specific area. All Loggers here is proxys for real
 * loggers.
 */

class ThreadLogManager {
  milog::priv::TContextStack context_;
  std::string contextStr_;
  TLoggerList loggerList;

  void updateContextStr();

 public:
  ThreadLogManager();
  ~ThreadLogManager();

  Logger* getLogger(const std::string &id);
  bool setLogger(const std::string &id, Logger *logger);
  void removeLogger(const std::string &id);

  static ThreadLogManager* instance();

  void push(const std::string &context);
  void pop();

  std::string* getContextStr() {
    return &contextStr_;
  }
};
}
}

#endif
