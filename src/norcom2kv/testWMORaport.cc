/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testWMORaport.cc,v 1.1.6.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#include <iostream> 
#include <fstream>
#include "WMORaport.h"
#include <sstream>

using namespace std;

bool
readFile(const std::string &filename, std::string &content);

int
main(int argn, char **argv)
{
  WMORaport wmo;
  string    raport;

  if(argn<2){
    cout << "Bruk\n\ttestWMORaport filename\n\n";
    return 1;
  }

  if(!readFile(argv[1], raport)){
    cout << "Cant open file <" << argv[1] << ">\n\n";
    return 1;
  }

  //  cout << raport;
  
  if(!wmo.split(raport)){
    cerr << wmo.error() << endl;
  }
 
 cout << wmo << endl;
 cerr << wmo.error() << endl;
}


bool
readFile(const std::string &filename, std::string &content)
{
  fstream fist(filename.c_str());
  string buf;
  ostringstream ost;

  content.clear();

  if(!fist){
    return false;
  }
    
  while(getline(fist, buf)){
    ost << buf << endl;
  }
  
  content=ost.str();
  return true;
}
