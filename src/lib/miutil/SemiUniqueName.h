/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: Data.h,v 1.2.6.2 2007/09/27 09:02:22 paule Exp $

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

#ifndef __SEMIUNIQUENAME_H__
#define __SEMIUNIQUENAME_H__

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>

namespace miutil {

class SemiUniqueName
{
   static boost::posix_time::ptime pt;
   static int seq;
   static  boost::mutex mutex;

public:
   SemiUniqueName();
   ~SemiUniqueName();

   static std::string
   uniqueName( const std::string &prefix, const char *endsWith="");
};

}

#endif
