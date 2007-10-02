/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testarrayop.cc,v 1.2.6.1 2007/09/27 09:02:23 paule Exp $                                                       

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
#include "SynopData.h"

using namespace miutil;
using namespace std;

int
main(int argn, char *argv[])
{
  SynopDataList dl;
  SynopDataList dl1;
  SynopData     sd;
  miTime        t(miTime::nowTime());
  
  
  cout << "------------------------------------" << endl;
  dl["2003-12-18 2:00:00"]=SynopData();
  dl["2003-12-18 3:00:00"]=SynopData();
  dl["2003-12-18 4:00:00"]=SynopData();
  dl["2003-12-18 5:00:00"]=SynopData();
  dl["2003-12-18 6:00:00"]=SynopData();
  
  for(CISynopDataList it=dl.begin();
      it!=dl.end(); 
      it++){
    cout << it->time() << endl;
  }

  cout << endl << endl;

  dl1.insert("2003-12-18 15:00:00",SynopData());
  dl1.insert("2003-12-18 10:00:00",SynopData());
  dl1.insert("2003-12-18 18:00:00",SynopData());
  dl1.insert("2003-12-18 9:00:00",SynopData());
  dl1.insert("2003-12-18 12:00:00",SynopData());
  
  for(CISynopDataList it=dl1.begin();
      it!=dl1.end(); 
      it++){
    cout << it->time() << endl;
  }


}
