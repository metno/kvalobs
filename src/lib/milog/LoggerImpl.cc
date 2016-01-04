/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: LoggerImpl.cc,v 1.6.6.4 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <milog/LogStream.h>
#include <milog/private/LoggerImpl.h>
#include <iostream>

using namespace std;

///Creates an logger that logs to std::err.
milog::priv::LoggerImpl::LoggerImpl() {
}

///Creates an logger that logs to logStream.
milog::priv::LoggerImpl::LoggerImpl(LogStream *logStream) {
  streamList.push_back(logStream);
}

milog::priv::LoggerImpl::~LoggerImpl() {
  if (!streamList.empty()) {
    ITLogStreamList it = streamList.begin();

    for (; it != streamList.end(); it++) {
      delete *it;
    }
  }
}

void milog::priv::LoggerImpl::log(const std::string &msg, LogLevel ll,
                                  const std::string &context) {
  milog::thread::ScopedLock lock(mutex_);

  if (!streamList.empty()) {
    ITLogStreamList it = streamList.begin();

    for (; it != streamList.end(); it++) {
      if (*it) {
        (*it)->message(msg, ll, context);
      }
    }
  }
}

void milog::priv::LoggerImpl::addLogStream(LogStream *logStream) {
  if (logStream) {
    milog::thread::ScopedLock lock(mutex_);
    streamList.push_back(logStream);
  }
}

int milog::priv::LoggerImpl::removeLogStream() {
  LogStream *stream;

  if (streamList.empty())
    return 0;

  stream = streamList.back();

  delete stream;

  streamList.pop_back();

  return streamList.size();
}

