/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: LogStream.cc,v 1.8.6.2 2007/09/27 09:02:32 paule Exp $

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
#include <milog/Layout.h>
#include <milog/LogStream.h>

#include <iostream>

// using namespace milog;

milog::LogStream::LogStream(Layout *layout, LogLevel ll)
    : layout_(layout), loglevel_(ll) {}

milog::LogStream::LogStream() : layout_(0), loglevel_(milog::NOTSET) {}

void milog::LogStream::layout(Layout *layout) {
  if (layout_) {
    delete layout_;
  }

  layout_ = layout;
}

milog::LogStream::~LogStream() {
  if (layout_)
    delete layout_;
}

void milog::LogStream::message(const std::string &msg, LogLevel ll,
                               const std::string &context) {
  std::string fmsg;

  if (ll > loglevel_ || ll == INFO)
    return;

  if (layout_) {
    fmsg = layout_->formatMessage(msg, ll, context);
    write(fmsg);
  }
}
