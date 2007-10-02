/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testenv.cc,v 1.1.6.1 2007/09/27 09:02:37 paule Exp $                                                       

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
#include <string>

using namespace std;

int
main(int argn, char **argv)
{
  string::size_type i, i2;
  string kvservers("abc def ghi\r");
  
  cout << kvservers << endl;
  
  i=kvservers.find_first_not_of(" \t\n\r");
  
  while(i!=string::npos){
    i2=kvservers.find_first_of(" \t\n\r", i);
    
    if(i2!=string::npos){
      //refDataMap[kvservers.subs(i, i2-i)]=Data::_nil();
      cout << kvservers.substr(i, i2-i) << " ";
      i=i2;
    }else{
      cout << kvservers.substr(i, kvservers.length()-i) << " ";
      i=kvservers.length();
    }
    
    i=kvservers.find_first_not_of(" \t\n\r", i);
  }

  cout << endl;
}
