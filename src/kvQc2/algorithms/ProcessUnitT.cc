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
#include "BasicStatistics.h"
#include "Qc2App.h"
#include "Qc2Connection.h"
#include "Qc2D.h"
#include "ReadProgramOptions.h"
#include <sstream>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <puTools/miTime>
#include <memory>
#include <stdexcept>

#include "CheckedDataCommandBase.h"
#include "CheckedDataHelper.h"

//#include <qapplication.h>
//#include <qpushbutton.h>
//#include <qlcdnumber.h>
//#include <qfont.h>
//#include <qlayout.h>

//#include "PaperField.h"

#include "scone.h"



using namespace kvalobs;
using namespace std;
using namespace miutil;

int 
ProcessImpl::
ProcessUnitT( ReadProgramOptions params )
{
 /// Need to integrate multiple handling of different type ids OR resolve this issue
 /// by separate program that scan kvalobs database and identifies the value of
 /// each duplicate measurement to use ...
   int pid=params.pid;
   float LinInterpolated;
   float TanTaxInterpolated;
   miutil::miTime stime=params.UT0;
   miutil::miTime etime=params.UT1;
   std::string CIF=params.ControlInfoString;

  std::list<kvalobs::kvStation> StationList;
  std::list<int> StationIds;
  std::list<kvalobs::kvData> Qc2Data;
  std::list<kvalobs::kvData> Qc2SeriesData;
  std::list<kvalobs::kvData> Qc2InterpData;
  std::list<kvalobs::kvData> ReturnData;
  bool result;

  kvalobs::kvStationInfoList  stList;
  CheckedDataHelper checkedDataHelper(app);

  kvalobs::kvDbGate dbGate( &con );

  miutil::miTime ProcessTime;
  miutil::miTime XTime;
  miutil::miTime YTime;
  ProcessTime = etime;

  std::vector<kvalobs::kvData> Tseries;
  std::list<kvalobs::kvData> MaxT;
  std::list<kvalobs::kvData> MinT;


  GetStationList(StationList);  /// StationList is all the possible stations ... Check
  for (std::list<kvalobs::kvStation>::const_iterator sit=StationList.begin(); sit!=StationList.end(); ++ sit) {
     StationIds.push_back( sit->stationID() );
  } 

  while (ProcessTime >= stime) 
  {
     XTime=ProcessTime;
     XTime.addHour(-1);
     YTime=ProcessTime;
     YTime.addHour(1);
     Tseries.clear();

             try {
                result = dbGate.select(Qc2Data, kvQueries::selectMissingData(params.missing,pid,ProcessTime));
              }
              catch ( dnmi::db::SQLException & ex ) {
                IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
              }
              catch ( ... ) {
                IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
              }
              if(!Qc2Data.empty()) {
                   for (std::list<kvalobs::kvData>::const_iterator id = Qc2Data.begin(); id != Qc2Data.end(); ++id) {
                          result = dbGate.select(Qc2SeriesData, kvQueries::selectData(id->stationID(),pid,XTime,YTime));
                          for (std::list<kvalobs::kvData>::const_iterator is = Qc2SeriesData.begin(); is != Qc2SeriesData.end(); ++is) {
                             Tseries.push_back(*is);
                          }
                             if (Tseries.size()==3) {
                                   if (Tseries[0].original() > params.missing && Tseries[1].original()==params.missing && 
                                       Tseries[2].original() > params.missing){
                                       std::cout << "PHOTON FISH CAKE !!!!!!!!!!!!!" << std::endl;
                                       result = dbGate.select(MaxT, kvQueries::selectData(id->stationID(),215,YTime,YTime));
                                       result = dbGate.select(MinT, kvQueries::selectData(id->stationID(),213,YTime,YTime));
                                       if (MaxT.size()==1 && MinT.size()==1){

                                               LinInterpolated=0.5*(Tseries[0].original()+Tseries[2].original());
                                               TanTaxInterpolated=0.5*(MinT.begin()->original()+MaxT.begin()->original());
                                               std::cout << "originals[0]: " << Tseries[0].original() << std::endl;
                                               std::cout << "originals[1]: " << Tseries[1].original() << " Linear Interpolation: " << LinInterpolated << std::endl;
                                               std::cout << "Tan Tax Interpolation: " << TanTaxInterpolated << std::endl;
                                               std::cout << "originals[2]: " << Tseries[2].original() << std::endl;

                                               std::cout << "Min: " << MinT.begin()->original() << std::endl;
                                               std::cout << "Max: " << MaxT.begin()->original() << std::endl;
                                               std::cout << "-------------------------------------" << std::endl;

                                     result = dbGate.select(Qc2InterpData, kvQueries::selectData(StationIds,pid,Tseries[1].obstime(),
                                                                                                           Tseries[1].obstime() ));
                                     Qc2D GSW(Qc2InterpData,StationList,params);
                                     GSW.Qc2_interp(); 
                                     std::cout << Tseries[1].obstime()  <<" "        
                                               << Tseries[1].stationID()<<" "        
                                               << Tseries[1].corrected()<<" "        
                                               << Tseries[1].original() << " "         
                                               << GSW.corrected_[GSW.stindex[Tseries[1].stationID()]] << " "
                                               << "Nearest Neighbour: " << GSW.intp_[GSW.stindex[Tseries[1].stationID()]] << " "
                                               << GSW.stid_[GSW.stindex[Tseries[1].stationID()]] << std::endl;

                                       //std::cout << Tseries[kkk].original() << "," << GSW.intp_[GSW.stindex[Tseries[kkk].stationID()]] << ",";

                                       }
                                   }
                             }
                          }
                   }
                                     //result = dbGate.select(Qc2InterpData, kvQueries::selectData(StationIds,pid,Tseries[kkk].obstime(),
                                     //Qc2D GSW(Qc2InterpData,StationList,params);
                                     //GSW.Qc2_interp(); 
                Tseries.clear();
  ProcessTime.addHour(-1);
  }
return 0;
}

