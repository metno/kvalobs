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

#include "ReadTypesFromFile.h"
#include <miutil/commastring.h>
#include <miutil/trimstr.h>
#include <milog/milog.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;
using namespace miutil;
using namespace kvalobs;

bool
ReadTypesFromFile(const std::string &filename, KvTypeList &typeList, bool hasHeader )
{
  ifstream fs;
  string   buf;
  string   sparamid;
  string   name;
  int      skipLine=(hasHeader?1:0);

  typeList.clear();
  fs.open(filename.c_str());
  
  if(!fs){
    cerr << "Cant open file <" << filename << ">!" << endl;
    return false;
  }

  while(getline(fs, buf)){
	  if( skipLine > 0 ) {
		  skipLine--;
		  continue;
	  }
    CommaString c(buf, '|');

    if(c.size()<7){
      cerr << "To few elements!" << endl;
      continue;
    }
//    kvTypes(int ty, std::string na, int earlyobs, int lateobs, std::string read,
//    			std::string obs, std::string co)
    typeList.push_back( kvTypes(c[0].as<int>(), c[1], c[2].as<int>(),c[3].as<int>(),c[4], c[5], c[6] ) );
  } 

  fs.close();

  return true;
}

