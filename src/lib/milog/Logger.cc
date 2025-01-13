/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: Logger.cc,v 1.6.6.4 2007/09/27 09:02:32 paule Exp $

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
#include <milog/Logger.h>
#include <milog/StdErrStream.h>
#include <milog/TraceLayout.h>
#include <milog/private/LoggerImpl.h>

#include <iostream>

using namespace milog::priv;

milog::Logger milog::Logger::dummyLogger(0);

milog::Logger::Logger(milog::priv::LoggerImpl *logger, LogLevel ll,
                      bool enabled)
    : logger_(logger), thrLocaleLoggers_(0), level_(ll), enabled_(enabled),
      ownerOfLogger(false) {}

milog::Logger::Logger()
    : thrLocaleLoggers_(0), ownerOfLogger(true), level_(NOTSET),
      enabled_(true) {
  StdErrStream *strm = 0;
  TraceLayout *layout = 0;

  try {
    layout = new TraceLayout();
    strm = new StdErrStream(layout);
    layout = 0; // StdStream is responsible for the layout now.
    logger_ = new milog::priv::LoggerImpl(strm);
    strm = 0; // LoggerImpl is responsible for the strm now.
  } catch (...) {
    delete layout;
    delete strm;
    delete logger_;
    logger_ = 0;
  }
}

milog::Logger::Logger(LogStream *logStream)
    : logger_(0), thrLocaleLoggers_(0), level_(NOTSET), enabled_(true),
      ownerOfLogger(true) {
  try {
    logger_ = new milog::priv::LoggerImpl(logStream);
  } catch (...) {
  }
}

milog::Logger::~Logger() {
  if (ownerOfLogger) {
    delete logger_;
  }

  delete thrLocaleLoggers_;
}

bool milog::Logger::setDefaultLogger(LogStream *logStrm) {
  return Logger::createLogger("__milog_default_logger__", logStrm);
}

void milog::Logger::resetDefaultLogger() {
  Logger::removeLogger("__milog_default_logger__");
}

bool milog::Logger::createLogger(const std::string &id, LogStream *ls) {
  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();

  if (!tlm)
    return false;

  try {
    Logger *log = new Logger(ls);
    return tlm->setLogger(id, log);
  } catch (...) {
    return false;
  }
}

void milog::Logger::removeLogger(const std::string &id) {
  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();

  if (!tlm)
    return;

  tlm->removeLogger(id);
}

void milog::Logger::pushLogStream(LogStream *strm) {
  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();

  if (!tlm)
    return;

  Logger *log = tlm->getLogger("__milog_default_logger__");

  if (!log)
    return;

  if (!log->thrLocaleLoggers_) {
    try {
      log->thrLocaleLoggers_ = new priv::LoggerImpl(strm);
    } catch (...) {
      return;
    }
  } else {
    log->thrLocaleLoggers_->addLogStream(strm);
  }
}

void milog::Logger::popLogStream() {
  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();

  if (!tlm)
    return;

  Logger *log = tlm->getLogger("__milog_default_logger__");

  if (!log)
    return;

  if (!log->thrLocaleLoggers_) {
    return;
  } else {
    if (log->thrLocaleLoggers_->removeLogStream() == 0) {
      delete log->thrLocaleLoggers_;
      log->thrLocaleLoggers_ = 0;
    }
  }
}

milog::Logger &milog::Logger::logger() {
  return logger("__milog_default_logger__");
}

milog::Logger &milog::Logger::logger(const std::string &id) {
  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();
  Logger *log;

  if (!tlm) {
    return dummyLogger;
  }

  log = tlm->getLogger(id);

  if (!log) {
    return dummyLogger;
  }

  return *log;
}

milog::LogLevel milog::Logger::logLevel(LogLevel ll) {
  LogLevel tmp = level_;
  level_ = ll;
  return tmp;
}

void milog::Logger::log(LogLevel ll, const std::string &msg) {
  if (!logger_)
    return;

  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();

  if (ll <= level_ || ll == milog::INFO) {
    if (tlm) {
      logger_->log(msg, ll, *tlm->getContextStr());

      if (thrLocaleLoggers_)
        thrLocaleLoggers_->log(msg, ll, *tlm->getContextStr());
    } else {
      logger_->log(msg, ll, "");
    }
  }
}

void milog::Logger::push(const std::string &context) {
  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();

  if (!tlm) {
    return;
  }

  tlm->push(context);
}

void milog::Logger::pop() {
  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();

  if (!tlm) {
    return;
  }

  tlm->pop();
}

std::string milog::Logger::getContextString() {
  priv::ThreadLogManager *tlm = priv::ThreadLogManager::instance();

  if (!tlm) {
    return std::string("");
  }

  return std::string(*tlm->getContextStr());
}
