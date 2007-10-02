/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testKvGetData.cc,v 1.2.6.1 2007/09/27 09:02:47 paule Exp $                                                       

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
#include <kvQtApp.h>


using namespace std;
using namespace kvalobs;
using namespace kvservice;

int
main(int argn, char **argv)
{
  WhichDataHelper whichData;
  KvObsDataList   dataList;
  miutil::miTime  from(miutil::miTime::nowTime());
  KvQtApp   app(argn, argv, false);

  from.addDay(-1);

  COUT("Getting data for station: 18700." << endl
       << "From date: " << from << "  until now!" << endl);

  whichData.addStation(18700, from);

  //if(app.getKvData(dataList, whichData)){
  if(!KvQtApp::kvQApp->getKvData(dataList, whichData)){
    cerr << "Cant connect to kvalobs!";
    return 1;
  }

  for(CIKvObsDataList it=dataList.begin();
      it!=dataList.end();
      it++){
    cout << it->front().stationID() << " obstime: " << it->front().obstime() << "  " 
	 << it->size() << " parameter(s)" << endl;
  }

  CERR("testKvGetData ..... exit\n");

  return 0;
}
