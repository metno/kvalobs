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
#include <limits.h>
#include <sstream>
#include "SemiUniqueName.h"

namespace pt=boost::posix_time;


using namespace std;

namespace miutil {

pt::ptime SemiUniqueName::pt;
int SemiUniqueName::seq(INT_MAX);
boost::mutex SemiUniqueName::mutex;


SemiUniqueName::
SemiUniqueName()
{
}

SemiUniqueName::
~SemiUniqueName()
{
}

std::string
SemiUniqueName::
uniqueName( const std::string &prefix, const char *endsWith )
{
   boost::mutex::scoped_lock lock( mutex );

   if( seq==INT_MAX) {
      seq=0;
      pt = pt::second_clock::universal_time();
   } else {
      pt::ptime now=pt::second_clock::universal_time();

      if( pt < now ) {
         pt = now;
         seq=0;
      } else {
         ++seq;
      }
   }

   ostringstream o;

   o << prefix << "-" << pt.date().year()
     << setfill('0') << setw(2) << pt.date().month().as_number()
     << setfill('0') << setw(2) << pt.date().day()
     << setfill('0') << setw(2) << pt.time_of_day().hours()
     << setfill('0') << setw(2) << pt.time_of_day().minutes()
     << setfill('0') << setw(2) << pt.time_of_day().seconds()
     << "-" << seq
     << endsWith;

   return o.str();
}

}
