/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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
#include <sstream>
#include "lib/miutil/gethostname.h"
#include "lib/miutil/getusername.h"
#include "creategroupid.h"
#include <errno.h>

#ifdef  _GNU_SOURCE
  extern char *program_invocation_short_name;
  const char *groupid_Progname_ = program_invocation_short_name;
#else
  extern char *__progname;
  const char *const char *groupid_Progname_= __progname;
#endif


namespace kvalobs {
namespace subscribe {

std::string createKafkaConsumerGroupId(const std::string &prefix_)
{
  std::string prefix;
  auto hostname=miutil::getHostname();
  auto username=miutil::getUsername();
  auto hostid=gethostid();

  if( !prefix_.empty() ) {
    prefix=prefix_+"_";
  } else if( groupid_Progname_ ) {
      prefix=groupid_Progname_;
  }

  std::ostringstream ost;

  ost << prefix  << hostname << "_" << username << "_" << std::hex << hostid;
  return ost.str();
}


}
}

