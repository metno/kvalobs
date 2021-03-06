/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvCronString.cc,v 1.1.2.2 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "kvCronString.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace kvalobs;

kvalobs::CronString::CronString(const std::string& str)
    : str_(str) {
  unpackString();
}

namespace {
bool isNumber(std::string s) {
  try {
    boost::trim(s);
    boost::lexical_cast<int>(s);
    return true;
  } catch (boost::bad_lexical_cast &) {
    return false;
  }
}
}

void kvalobs::CronString::unpackString() {
  for (int i = 0; i < NT; i++)
    numbers[i].clear();
  isempty = true;

  boost::trim(str_);

  vector<std::string> v, v2;
  boost::split(v, str_, boost::algorithm::is_space(),
               boost::algorithm::token_compress_on);

  if (v.size() < NT) {
    return;
  }

  for (int i = 0; i < NT; i++) {
    if (v[i] != "*") {
      isempty = false;
      boost::split(v2, v[i], boost::algorithm::is_any_of(","),
                   boost::algorithm::token_compress_on);
      for (size_t j = 0; j < v2.size(); j++)
        if (isNumber(v2[j]))
          numbers[i].push_back(atoi(v2[j].c_str()));
    }
  }

}

void kvalobs::CronString::str(const std::string& str) {
  str_ = str;
  unpackString();
}

bool kvalobs::CronString::active(const boost::posix_time::ptime& t) {
  if (isempty)
    return true;

  vector<int>::iterator itr;

  int thistime[5];

  thistime[0] = t.time_of_day().minutes();
  thistime[1] = t.time_of_day().hours();
  thistime[2] = t.date().day();
  thistime[3] = t.date().month();
  thistime[4] = t.date().year();

  // check backwards (year,month,day,hour,minute)
  // note that using NT<5 enables skipping year,month,...
  for (int i = NT - 1; i >= 0; i--) {
    if (!numbers[i].empty()) {
      for (itr = numbers[i].begin(); itr != numbers[i].end(); itr++)
        if (*itr == thistime[i])
          break;
      if (itr == numbers[i].end())
        return false;
    }
  }

  return true;
}
