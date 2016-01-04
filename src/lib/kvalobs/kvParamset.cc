/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvParamset.cc,v 1.2.6.2 2007/09/27 09:02:30 paule Exp $

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
#include <map>
#include <kvalobs/kvParamset.h> 
#include <kvalobs/kvParamsetdef.h>

using namespace std;
using namespace miutil;

kvalobs::kvParamset::kvParamset() {

  for (int i = 0; i < nparamset; i++) {
    paramset t = paramset_[i];
    int paramsetid = t.paramsetid;
    char * set = t.set;
    std::string str = std::string(set);
    vector<std::string> vstr = str.split(str);
    m_str_paramset[paramsetid] = vstr;
    vector<int> vi;
    for (int k = 0; k < vstr.size(); k++) {
      int ll = atoi(vstr[k].c_str());
      vi.push_back(ll);
    }

    m_int_paramset[paramsetid] = vi;
    add_inverse(paramsetid, vi);
  }

}

void kvalobs::kvParamset::add_inverse(int paramsetid, vector<int> vi) {
  for (int i = 0; i < vi.size(); i++) {
    m_int_param[vi[i]].push_back(paramsetid);
  }
}

vector<int> kvalobs::kvParamset::get_param(int paramsetid) {
  return m_int_paramset[paramsetid];
}

vector<std::string> kvalobs::kvParamset::get_param_str(int paramsetid) {
  return m_str_paramset[paramsetid];
}

vector<int> kvalobs::kvParamset::get_paramset(int paramid) {
  //NOT IMPLEMENTED YET
  return m_int_param[paramid];
}

