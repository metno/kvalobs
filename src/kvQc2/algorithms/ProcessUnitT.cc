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

#include "ProcessControl.h"
#include "CheckedDataCommandBase.h"
#include "CheckedDataHelper.h"

#include "scone.h"
#include "tround.h"

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
  LOGINFO("ProcessUnitT");
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

  ProcessControl CheckFlags;
  kvalobs::kvControlInfo fixflags;

  kvalobs::kvStationInfoList  stList;
  CheckedDataHelper checkedDataHelper(app);

  kvalobs::kvDbGate dbGate( &con );

  miutil::miTime ProcessTime;
  miutil::miTime XTime;
  miutil::miTime YTime;
  ProcessTime = etime;

  //std::cout << "ETIME TIME STAMP: " << etime << std::endl;
  //std::cout << "STIME TIME STAMP: " << stime << std::endl;
  //std::cout << "ProcessTIME TIME STAMP: " << ProcessTime << std::endl;

  miutil::miTime fixtime; /// fixtime here for tests

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
                          Tseries.clear();   /// or is it better here ??? YES YES YES
                          result = dbGate.select(Qc2SeriesData, kvQueries::selectData(id->stationID(),pid,XTime,YTime));
                          for (std::list<kvalobs::kvData>::const_iterator is = Qc2SeriesData.begin(); is != Qc2SeriesData.end(); ++is) {
                             if  ( !CheckFlags.condition(is->controlinfo(),params.Aflag) ) { /// If one or more of the analysis flags are set then will not process further! 
                                   Tseries.push_back(*is);
                             }
                          }
                             if (Tseries.size()==3) {
                                   //std::cout << "---------------------" << std::endl;
                                   //std::cout << "Found an analysis case ... " << std::endl;
                                   LOGINFO("Interpolating data ...");
                                   //std::cout << Tseries[0].stationID() << " " <<  Tseries[1].stationID() << " " << Tseries[2].stationID() << std::endl; 
                                   //std::cout << Tseries[0].obstime() << " " <<  Tseries[1].obstime() << " " << Tseries[2].obstime() << std::endl; 
                                   //std::cout << Tseries[0].original() << " " <<  Tseries[1].original() << " " << Tseries[2].original() << std::endl; 
                                   if (Tseries[0].original() > params.missing && Tseries[1].original()==params.missing && 
                                       Tseries[2].original() > params.missing){
                                       //std::cout << "Checking Other Parameters" << std::endl;
                                       result = dbGate.select(MaxT, kvQueries::selectData(id->stationID(),215,YTime,YTime));
                                       result = dbGate.select(MinT, kvQueries::selectData(id->stationID(),213,YTime,YTime));
                                       //std::cout <<  MaxT.size() << " " << MinT.size() << std::endl;
                                       if (MaxT.size()==1 && MinT.size()==1){

                                               //std::cout <<  MaxT.begin()->original() << " " << MinT.begin()->original() << std::endl;
                                               LinInterpolated=0.5*(Tseries[0].original()+Tseries[2].original());
                                               TanTaxInterpolated=0.5*(MinT.begin()->original()+MaxT.begin()->original());
                                               TanTaxInterpolated=round<float,1>(TanTaxInterpolated);

                                     result = dbGate.select(Qc2InterpData, kvQueries::selectData(StationIds,pid,Tseries[1].obstime(),
                                                                                                           Tseries[1].obstime() ));
                                     Qc2D GSW(Qc2InterpData,StationList,params);
                                     GSW.Qc2_interp(); 
                                     fixtime=Tseries[1].obstime();
                                     //fixtime.addDay(730);  // Write the data to 2 years hence (beware leap years in back calculating)
                                     fixflags=Tseries[1].controlinfo();
                                     CheckFlags.setter(fixflags,params.Sflag);

// The logic to write results back to the database and inform kvServiceD
//  In this case the data we are working with (i.e. potentially right back to the db) is Tseries[1]

                      try {
                             /// Update if correction is out of TAN TAX range!
                             if ( Tseries[1].corrected() <  MinT.begin()->original() || Tseries[1].corrected() >  MaxT.begin()->original() ) 
                            {  
                             //Set data structure to write to the database
                             /// REMOVE ALL THE CONTROLS
                                //if ( CheckFlags.condition(id->controlinfo(),params.Wflag) )  {  /// WFLAG needs putting back in
                                /// and a control in the configuration file to turn totally on or off!!!!
                                 {
                                        //std::cout << "W-flag-Passed" << std::endl;
                                        kvData d;                                                   
                                        d.set(Tseries[1].stationID(),
                                              Tseries[1].obstime(),
                                              Tseries[1].original(),
                                              Tseries[1].paramID(),
                                              Tseries[1].tbtime(),
                                              Tseries[1].typeID(),
                                              Tseries[1].sensor(),
                                              Tseries[1].level(),
                                              TanTaxInterpolated,                                                           
                                              fixflags,
                                              Tseries[1].useinfo(),
                                              Tseries[1].cfailed()+" Qc2 UnitT corrected was:"+StrmConvert(Tseries[1].corrected()) );
                             // Set use info corresponding to controlinfo
                                        kvUseInfo ui = d.useinfo();
                                        ui.setUseFlags( d.controlinfo() );
                                        d.useinfo( ui );   
                             // write the data back
                                        //std::cout << "This data to be written back to db ... " << std::endl; 
                                        dbGate.insert( d, "data", true); 
                             // fill structure to inform the serviced
                                        kvalobs::kvStationInfo::kvStationInfo DataToWrite(id->stationID(),id->obstime(),id->paramID());
                                        stList.push_back(DataToWrite);
                                   }
                              }
                              //Tseries.clear();  /// This may be better here than at the bottom ... check later
                       }
                          catch ( dnmi::db::SQLException & ex ) {
                            IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
                            std::cout<<"INSERTO> CATCH ex" << result <<std::endl;
                          }
                          catch ( ... ) {
                            IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
                            std::cout<<"INSERTO> CATCH ..." << result <<std::endl;
                          }
                          if(!stList.empty()){
                              checkedDataHelper.sendDataToService(stList);
                              stList.clear();
                          }
                                       }
                                   }
                             }
                          }
                   }
                //Tseries.clear();
                //
  ProcessTime.addHour(-1);
  }
return 0;
}

