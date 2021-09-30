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

#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <memory>

#include "getusername.h"

using namespace std;

namespace miutil {


string getUsername(){
  auto euid=geteuid();
  passwd  myPwd;
  passwd  *pwRes;
  string username;

  auto bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);

  if( bufsize<0 ){
    bufsize=16384;
  }

  auto buf = unique_ptr<char>(new char[bufsize]);
  auto e = getpwuid_r(euid, &myPwd, buf.get(), bufsize, &pwRes);
  
  if ( pwRes ) {
    username=pwRes->pw_name;
  }

  return username;
}


}
