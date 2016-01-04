/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvPath.cc,v 1.1.6.3 2007/09/27 09:02:30 paule Exp $

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
#include <stdlib.h>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#include <kvalobs/kvPath.h>
#include <boost/assign/list_of.hpp>
#include <map>

namespace kvalobs {
namespace {
const std::map<std::string, PathQuery> pathNameTranslations =
    boost::assign::map_list_of("pkglibdir", pkglibdir)("sysconfdir", sysconfdir)(
        "libdir", libdir)("bindir", bindir)("datadir", datadir)("localstatedir",
                                                                localstatedir)(
        "logdir", logdir)("rundir", rundir)("prefix", prefix);
}

std::string kvPath(PathQuery name, const std::string & system) {
  std::string ret;

  switch (name) {
    case pkglibdir:
      ret = PKGLIBDIR;
      break;
    case sysconfdir:
      ret = SYSCONFDIR + std::string("/" + system);
      break;
    case libdir:
      ret = LIBDIR;
      break;
    case bindir:
      ret = BINDIR;
      break;
    case datadir:
      ret = DATADIR + std::string("/" + system);
      break;
    case localstatedir:
      ret = LOCALSTATEDIR + std::string("/lib/" + system);
      break;
    case logdir:
      ret = LOCALSTATEDIR + std::string("/log/" + system);
      break;
    case rundir:
      ret = LOCALSTATEDIR + std::string("/run/" + system);
      break;
    case prefix:
      ret = PREFIX;
      break;
  }

  boost::algorithm::replace_all(ret, "//", "/");

  return ret;
}
}

std::string kvPath(const std::string &name, const std::string &system) {
  std::map<std::string, kvalobs::PathQuery>::const_iterator find =
      kvalobs::pathNameTranslations.find(name);
  if (find == kvalobs::pathNameTranslations.end()) {
    std::cerr << "FATAL: The 'name' (" << name << ") is NOT recognised!\n";
    return "";
  } else {
    return kvPath(find->second, system);
  }
}
