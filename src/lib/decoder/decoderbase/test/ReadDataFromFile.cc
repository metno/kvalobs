/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: ReadParamsFromFile.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $                                                       

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

#include "ReadDataFromFile.h"
#include <miutil/commastring.h>
#include <miutil/trimstr.h>
#include <milog/milog.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;
using namespace miutil;

bool ReadDataFromFile(const std::string &filename, std::string &obsType,
                      std::string &data) {
  ifstream fs;
  string line;
  int nLine = 0;
  ostringstream buf;

  obsType.clear();
  data.clear();

  fs.open(filename.c_str());

  if (!fs) {
    cerr << "ReadDataFromFile: Cant open file <" << filename << ">!" << endl;
    return false;
  }

  while (getline(fs, line)) {
    if (nLine == 0) {
      trimstr(line);
      if (line.empty())
        continue;
      nLine++;
      obsType = line;
      continue;
    }

    buf << line << "\n";
    nLine++;
  }

  fs.close();
  data = buf.str();

  return true;
}

