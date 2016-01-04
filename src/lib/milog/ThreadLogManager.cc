/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: ThreadLogManager.cc,v 1.6.6.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <milog/private/ThreadLogManager.h>
#include <milog/LogManager.h>
#include <milog/private/types.h>
#include <milog/Logger.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace milog {
namespace priv {

namespace {
milog::thread::ThreadLocalDataHolder<ThreadLogManager> tLogMgr_(
    "__milog_key__");
}

ThreadLogManager::ThreadLogManager() {
}

ThreadLogManager::~ThreadLogManager() {
  ITLoggerList it = loggerList.begin();

  for (; it != loggerList.end(); it++)
    delete it->second;
}

ThreadLogManager*
ThreadLogManager::instance() {
  ThreadLogManager *tlm = tLogMgr_.get();

  if (!tlm) {
    try {
      tlm = new ThreadLogManager();
      tLogMgr_.reset(tlm);
    } catch (...) {
      return 0;
    }
  }

  return tlm;
}

Logger*
ThreadLogManager::getLogger(const std::string &id) {
  Logger *logger;

  ITLoggerList it = loggerList.find(id);

  if (it != loggerList.end()) {
    return it->second;
  }

  LogManager *lm = LogManager::instance();

  if (!lm) {
    return 0;
  }

  priv::LoggerImpl *logImpl = lm->getLogger(id);

  if (!logImpl) {
    return 0;
  }

  try {
    logger = new Logger(logImpl, LogManager::loglevel(), LogManager::enabled());
  } catch (...) {
    return 0;
  }

  loggerList[id] = logger;

  //cerr << "getLogger: new proxy for logger in LogManager!\n";

  return logger;
}

bool ThreadLogManager::setLogger(const std::string &id, Logger *logger) {
  if (!logger)
    return false;

  ITLoggerList it = loggerList.find(id);

  if (it != loggerList.end()) {
    delete it->second;
    it->second = logger;
  } else
    loggerList[id] = logger;

  return true;
}

void ThreadLogManager::removeLogger(const std::string &id) {
  ITLoggerList it = loggerList.find(id);

  if (it != loggerList.end()) {
    delete it->second;
    loggerList.erase(it);
  }
}

void ThreadLogManager::updateContextStr() {
  ostringstream ost;
  CITContextStack it = context_.begin();

  if (it != context_.end()) {
    ost << *it;
    it++;
  }

  for (; it != context_.end(); it++) {
    ost << "/" << *it;
  }

  contextStr_ = ost.str();
}

void ThreadLogManager::push(const std::string &context) {
  context_.push_back(context);

  updateContextStr();
}

void ThreadLogManager::pop() {
  context_.pop_back();

  updateContextStr();
}

}
}
