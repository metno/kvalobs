/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: ConfInfo12.cc,v 1.1.2.2 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <list>
#include <string>
#include <ctype.h>
#include <stdlib.h>
#include <milog/milog.h>
#include "ConfInfo12.h"

using namespace std;
using namespace miutil::conf;

kvalobs::decoder::comobsdecoder::ConfInfo12::ConfInfo12() {
  for (int i = 0; i < 24; i++)
    tr_[i] = -1;

  tr_[6] = 2;
  tr_[18] = 2;
}

void kvalobs::decoder::comobsdecoder::ConfInfo12::parse(
    miutil::conf::ConfSection &conf) {
  miutil::conf::ConfSection *sect = conf.getSection("precipitation");

  if (!sect) {
    LOGWARN("No <precipitation> in the configuration file!");
    return;
  }

  doPrecip(*sect);
}

int kvalobs::decoder::comobsdecoder::ConfInfo12::tr(int hour) {
  if (hour < 0 || hour > 23)
    return -1;

  return tr_[hour];
}

void kvalobs::decoder::comobsdecoder::ConfInfo12::doPrecip(
    miutil::conf::ConfSection &sect) {
  list<string> keys = sect.getKeys();
  ValElementList val;
  string::size_type i;
  int n;
  int ntr;
  string sn;

  for (list<string>::iterator it = keys.begin(); it != keys.end(); it++) {
    i = it->find("tr");

    if (i == string::npos)
      continue;

    if (i != 0 || it->length() < 3)
      continue;

    sn = it->substr(2);

    string::size_type ii;

    for (ii = 0; ii < sn.length() && isdigit(sn[ii]); ii++)
      ;

    if (ii < sn.length())
      continue;

    n = atoi(sn.c_str());

    if (n < 0 || n > 23)
      continue;

    val = sect.getValue(*it);

    if (val.empty())
      continue;

    if (val[0].type() != INT)
      continue;

    ntr = static_cast<int>(val[0].valAsInt());

    if (ntr < 0 || ntr > 9)
      continue;

    tr_[n] = ntr;
  }
}
