/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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
#include "ProcessImpl.h"
#include "algorithms/BasicStatistics.h"
#include "Qc2App.h"
#include "Qc2Connection.h"
#include "algorithms/Qc2D.h"
#include "ReadProgramOptions.h"
#include <sstream>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <puTools/miTime>
#include <memory>
#include <stdexcept>

#include "CheckedDataCommandBase.h"
#include "CheckedDataHelper.h"



using namespace kvalobs;
using namespace std;
using namespace miutil;

ProcessImpl::ProcessImpl( Qc2App &app_, dnmi::db::Connection & con_ )
    : app( app_ ), con( con_ )
{
}

void 
ProcessImpl::
GetStationList(std::list<kvalobs::kvStation>& StationList)
{

   bool result;
   kvalobs::kvDbGate dbGate( &con );

   try {
        result = dbGate.select( StationList, kvQueries::selectAllStations("stationid"), "station" );
     }
     catch ( dnmi::db::SQLException & ex ) {
        IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
     }
     catch ( ... ) {
        IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
     }
}

void 
ProcessImpl::
GetStationList(std::list<kvalobs::kvStation>& StationList, miutil::miTime ProcessTime)
{

   bool result;
   kvalobs::kvDbGate dbGate( &con );

   try {
        result = dbGate.select( StationList, kvQueries::selectAllStations("stationid"), "station" );
     }
     catch ( dnmi::db::SQLException & ex ) {
        IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
     }
     catch ( ... ) {
        IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
     }
}

int 
ProcessImpl::
select(ReadProgramOptions params)
{
     int AlgoCode = params.AlgoCode;

     std::cout << params.ControlInfoString << std::endl;
     std::cout << params.ControlInfoVector[0] << std::endl;
     std::cout << params.ControlInfoVector[1] << std::endl;


     switch (AlgoCode) {
     case 1:
         std::cout << "Case: " << AlgoCode << std::endl;
         //Redistribute(params.UT0, params.UT1, params.tid);
         Redistribute(params);
         break;
     case 2:
         std::cout << "Case: " << AlgoCode << std::endl;
         Variability(params);
         break;
     case 3:
         std::cout << "Case: " << AlgoCode << std::endl;
         Process4D(params);
         break;
     case 4:
         std::cout << "Case: " << AlgoCode << std::endl;
         ProcessUnitT(params);
         break;
     case 5:
         std::cout << "Case: " << AlgoCode << std::endl;
         break;
     default:
         std::cout << "No valid Algorithm Code Provided. Case: " << AlgoCode << std::endl;
         break;
     }
     return(0);
}

