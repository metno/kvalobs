/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvQcxInfo.cc,v 1.6.6.2 2007/09/27 09:02:30 paule Exp $

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
#include <kvalobs/kvQcxInfo.h>

using namespace std;

kvalobs::kvQcxInfo::~kvQcxInfo() {
}

std::string kvalobs::kvQcxInfo::toSend() const {
  ostringstream ost;
  ost << "(" << quoted(medium_qcx_) << "," << quoted(main_qcx_) << ","
      << controlpart_ << "," << quoted(comment_) << ")";
  return ost.str();
}

bool kvalobs::kvQcxInfo::set(const std::string& medium_qcx__,
                             const std::string& main_qcx__, int controlpart__,
                             const std::string& comment__) {
  medium_qcx_ = medium_qcx__;
  main_qcx_ = main_qcx__;
  controlpart_ = controlpart__;
  comment_ = comment__;

  sortBy_ = std::string(medium_qcx_);
  return true;
}

bool kvalobs::kvQcxInfo::set(const dnmi::db::DRow& r_) {
  dnmi::db::DRow &r = const_cast<dnmi::db::DRow&>(r_);
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];
      if (*it == "medium_qcx") {
        medium_qcx_ = buf;
      } else if (*it == "main_qcx") {
        main_qcx_ = buf;
      } else if (*it == "controlpart") {
        controlpart_ = atoi(buf.c_str());
      } else if (*it == "comment") {
        comment_ = buf;
      }
    } catch (...) {
      CERR("kvQcxInfo: exception ..... \n");
    }
  }
  sortBy_ = std::string(medium_qcx_);
  return true;
}

std::string kvalobs::kvQcxInfo::uniqueKey() const {
  ostringstream ost;

  ost << " WHERE  medium_qcx=" << quoted(medium_qcx_);

  return ost.str();
}
