/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: cmprspace.h,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $

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

#include "gethostname.h"
#include <unistd.h>

namespace miutil {

std::string
getHostname()
{
  char sHostName[256];
  std::string hostName;

  auto e = ::gethostname(sHostName, 256);
  if (e == 0) {
    sHostName[63] = '\0';
    hostName = std::string(sHostName);
    auto i = hostName.find_last_of(".");
    if (i != std::string::npos) {
      hostName = hostName.substr(i + 1);
    }
  }
  return hostName;
}

}
