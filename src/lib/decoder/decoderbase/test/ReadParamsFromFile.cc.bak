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

#include "ReadParamsFromFile.h"
#include <miutil/commastring.h>
#include <miutil/trimstr.h>
#include <milog/milog.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;
using namespace miutil;

bool
ReadParamsFromFile(const std::string &filename, ParamList &paramList, bool hasHeader )
{
  ifstream fs;
  string   buf;
  string   sparamid;
  string   name;
  int skip=(hasHeader?1:0);

  paramList.clear();


  fs.open(filename.c_str());
  
  if(!fs){
    cerr << "Cant open file <" << filename << ">!" << endl;
    return false;
  }

  while( getline(fs, buf) ){
	  if( skip > 0 ) {
		  --skip;
		  continue;
	  }
    CommaString cStr(buf, '|');

    if(cStr.size()<2){
      cerr << "To few elements!" << endl;
      continue;
    }

    paramList.insert(Param(cStr[1], cStr[0].as<int>() ));
  } 

  fs.close();

  return true;
}

