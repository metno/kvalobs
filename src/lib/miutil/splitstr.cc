/*
 wdb - weather and water data storage

 Copyright (C) 2007 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 E-mail: wdb@met.no

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA  02110-1301, USA
 */

#include <iostream>
#include <sstream>

#include "trimstr.h"
#include "splitstr.h"

using namespace std;

namespace {
int
count(const std::string &str, char sep, char stringProtector);
}

std::vector<std::string> miutil::splitstr(const std::string &str, char sep,
                                          char stringProtector) {
  ostringstream ost;
  char ch = 0;
  bool inString = false;
  int i = 0;
  int n = count(str, sep, stringProtector);
  vector<string> data;
  string elem;

  data.resize(n + 1);
  std::string::const_iterator it = str.begin();

  for (; it != str.end(); it++) {
    if (*it == stringProtector && ch != '\\') {
      ost << *it;
      ch = *it;
      inString = !inString;
      continue;
    }

    ch = *it;

    if (inString) {
      ost << *it;
      continue;
    }

    if (*it != sep) {
      ost << *it;
    } else {
      elem = ost.str();
      trimstr(elem);
      data[i] = elem;
      i++;
      ost.str("");
    }
  }

  elem = ost.str();
  trimstr(elem);
  data[i] = elem;

  return data;
}

namespace {
int count(const std::string &str, char sep, char stringProtector) {
  string::const_iterator it;
  char ch = 0;
  int cnt = 0;
  bool inString = false;

  for (it = str.begin(); it != str.end(); it++) {
    if (*it == stringProtector && ch != '\\')
      inString = !inString;
    else if (*it == sep && !inString)
      cnt++;

    ch = *it;
  }

  return cnt;
}
}
