/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: HtmlLayout.cc,v 1.2.6.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <sstream>
#include <milog/HtmlLayout.h>
#include <iostream>

namespace milog {
namespace priv {
HtmlLayout::HtmlLayout() {
}

HtmlLayout::~HtmlLayout() {
}

std::string HtmlLayout::formatMessage(const std::string &msg, LogLevel ll,
                                      const std::string &context) {
  bool font = true;
  std::stringstream os;

  if (msg.empty())
    return std::string("");

  switch (ll) {
    case FATAL:
      os << "<font color=red> ****** FATAL ******";
      break;
    case ERROR:
      os << "<font color=red>";
      break;
    case WARN:
      os << "<font color=blue>";
      break;
    default:
      font = false;
      break;
  }

  os << msg;

  if (font)
    os << "</font>";

  if (msg[msg.length() - 1] != '\n')
    os << "\n";

  return os.str();
}
}
}

