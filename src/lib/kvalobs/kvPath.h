/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvPath.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvPath_h__
#define __kvPath_h__

#include <string>
#include <vector>

namespace kvalobs {
enum PathQuery {
  prefix,
  pkglibdir,
  sysconfdir,
  libdir,
  bindir,
  datadir,
  localstatedir,
  logdir,
  rundir
};

std::string kvPath(PathQuery name, const std::string & system = "kvalobs");
}

/**
 * kvPath returns the path to the named directory.
 * 
 * name may be one of:
 * 
 *  - prefix        , eg PREFIX
 *  - pkglibdir     , eg PKGLIBDIR/kvalobs
 *  - sysconfdir    , eg SYSCONFDIR/etc/'system'
 *  - libdir        , eg LIBDIR/lib
 *  - bindir        , eg BINDIR/bin
 *  - datadir       , eg DATADIR/share/'system'
 *  - localstatedir , eg LOCALSTATEDIR/lib/'system'
 *  - logdir        , eg LOCALSTATEDIR/log/'system'
 *  - rundir        , eg LOCALSTATEDIR/run/'system'
 *
 *  PREFIX, PKGLIBDIR, SYSCONFDIR, LIBDIR, BINDIR, DATADIR and LOCALSTATEDIR
 *  is as they are set by configure.
 * 
 * @return The path to the 'name'. An empty string if name is not recogniced.
 * 
 */
std::string kvPath(const std::string &name, const std::string &system =
                       "kvalobs");

/**
 * setKvPathPrefix - kvPath will return according to:
 *
 *  - prefix        , prefix
 *  - pkglibdir     , eg $prefix/lib/kvalobs
 *  - sysconfdir    , eg $prefix/etc/'system'
 *  - libdir        , eg $prefix/lib
 *  - bindir        , eg $prefix/bin
 *  - datadir       , eg $prefix/share/'system'
 *  - localstatedir , eg $prefix/var/lib/'system'
 *  - logdir        , eg $prefix/var/log/'system'
 *  - rundir        , eg $prefix/var/run/'system'
 *
 *  setPrefix is mostly useful for unit testing and should probably not be used
 *  by programs.
 */
void setKvPathPrefix(const std::string &prefix);

/**
 * Get a list of all available string arguments to kvPath
 */
std::vector<std::string> availableKvPathIdentifiers();


#endif
