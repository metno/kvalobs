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

#include <errno.h>
#include "getprogname.h"

#ifdef  _GNU_SOURCE
  extern char *program_invocation_short_name;
  const char *miutil_getprogram_Progname_ = program_invocation_short_name;
#else
  extern char *__progname;
  const char *const char *miutil_getprogram_Progname_= __progname;
#endif


namespace miutil {
std::string
getProgramNameFromArgv(const char** argv)
{
  std::string cmd(argv[0]);

  auto i = cmd.find_last_of("/\\");

  if (i != std::string::npos) {
    cmd = cmd.substr(i + 1);
  }
  return cmd;
}

std::string
getProgramName()
{
  return miutil_getprogram_Progname_;
}

}
