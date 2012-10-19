/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvdatatest.cc,v 1.1.6.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#include "kvalobs/kvData.h"

using namespace kvalobs;
using namespace std;
using namespace miutil;
int
main(int argn, char **argv)
{
  kvData d;
  miTime tbtime(miTime::nowTime());
  miTime obstime(2002, 4, 8, 12);
  string stmt;

   d=kvData(1024, obstime, 12.33f, 1023, tbtime, 23, 2, 100, 12.40f, kvControlInfo(),
            kvUseInfo(), ""); 

  cerr << d << endl << endl;
  
  
  cerr << d.toSend() << endl;

  cerr << "\n\n";

  try{
    cerr << "stationid:   " << d.stationID() << endl;
    cerr << "obstime:     " << d.obstime() << endl;
    cerr << "original:    " << d.original() << endl;
    cerr << "paramid:     " << d.paramID() << endl;
    cerr << "tbtime:      " << d.tbtime() << endl;
    cerr << "typeid:      " << d.typeID() << endl;
    cerr << "sensor:      " << d.sensor() << endl;
    cerr << "level:       " << d.level() << endl;
    cerr << "corrected:   " << d.corrected() << endl;
    cerr << "controlinfo: " << d.controlinfo() << endl;
    cerr << "useinfo:     " <<  d.useinfo() << endl;
  }
  catch(std::exception &ex){
    cerr << "EXCEPTION: " << ex.what() << endl;
    return 1;
  }

  return 0;
}
  
