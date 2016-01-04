/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: paramlist.cc,v 1.3.2.2 2007/09/27 09:02:31 paule Exp $

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
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <miutil/commastring.h>
#include <miutil/trimstr.h>
#include <milog/milog.h>
#include <kvalobs/paramlist.h>

using namespace std;
using namespace miutil;

std::string findParamIdInList(const ParamList &pl, int id) {
  CIParamList it = pl.begin();

  for (; it != pl.end(); it++) {
    if (it->id() == id)
      return it->kode();
  }

  return std::string();
}

bool findParamInList(const ParamList &pl, const std::string &name,
                     Param &param) {
  //In the find method only the name parametere of Param is
  //used so it does'nt matter what value id and isScalar has.
  CIParamList it = pl.find(Param(name, -1, false));

  if (it == pl.end())
    return false;

  param = *it;
  return true;
}

bool readParamsFromFile(const std::string &filename, ParamList &paramList) {
  ifstream fs;
  string buf;
  string sparamid;
  string name;
  bool isScalar;
  int lineno = 0;

  paramList.clear();
  fs.open(filename.c_str());

  if (!fs) {
    LOGERROR("Cant open param file <" << filename << ">!");
    return false;
  }

  while (getline(fs, buf)) {
    ++lineno;
    trimstr(buf);

    if (buf.empty() || buf[0] == '#')
      continue;

    CommaString cStr(buf, ',');

    if (cStr.size() < 3) {
      LOGERROR(
          "To few elements in param file <" << filename<< "> at line: " << lineno << ". Expecting 3, but found only " << cStr.size() << ".");
      return false;;
    }

    isScalar = true;

    if (cStr[2] == "f" || cStr[2] == "F")
      isScalar = false;

    paramList.insert(Param(cStr[1], cStr[0].as<int>(), isScalar));
  }

  fs.close();

  return true;
}

bool isParamListsEqual(const ParamList &oldList, const ParamList &newList) {
  if (oldList.size() != newList.size())
    return false;

  //The sets are sorted so if the sets are equal
  //the iterators should iterate through the same
  //elements.

  ParamList::const_iterator itOld = oldList.begin();
  ParamList::const_iterator itNew = newList.begin();

  for (; itNew != newList.end(); ++itNew, ++itOld) {
    if (itNew->kode() != itOld->kode() || itNew->id() != itOld->id()
        || itNew->isScalar() != itOld->isScalar()) {
      return false;
    }
  }
  return true;
}

std::ostream&
operator<<(std::ostream &os, const ParamList &p) {
  CIParamList it = p.begin();

  os << "ParamList:\n";

  for (; it != p.end(); it++)
    os << "  " << it->kode() << "  " << it->id() << " "
       << (it->isScalar() ? "true" : "false") << std::endl;

  return os;
}

