/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: testcommastring.cc,v 1.3.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#include "../include/miutil/commastring.h"

using namespace miutil;
using namespace std;

int main(int argn, char **argv) {
  CommaString str(",,,");
  CommaString strsep("@@@@@@", "@@");

  cerr << "Size (str)   : " << str.size() << endl;
  cerr << "Size (strsep): " << strsep.size() << endl;

  strsep.init("23 ,23, 234,f,,p");

  str[3] = "2";
  cerr << "strsep: " << strsep << endl;
  cerr << "str:    " << str << endl;

  strsep[3] = "k";
  strsep[4] = "b�";
  cerr << "strsep: " << strsep << endl;

  str.init("23 ,23, 234,f,,p");
  cerr << "Size: " << str.size() << endl;
  cerr << str << endl;
  str[3] = "k";

  cerr << str << endl;
  cerr << "Length: " << str.length() << endl;

  str.init("\"23 ,23, 234,f,,p\"");
  cerr << str << endl;
  cerr << "Size: " << str.size() << endl;
  cerr << "Length: " << str.length() << endl;

}
