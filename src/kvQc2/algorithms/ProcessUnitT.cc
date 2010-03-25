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
#include <puTools/miTime.h>
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
                          Tseries.clear();  
                          result = dbGate.select(Qc2SeriesData, kvQueries::selectData(id->stationID(),pid,XTime,YTime));
                          for (std::list<kvalobs::kvData>::const_iterator is = Qc2SeriesData.begin(); is != Qc2SeriesData.end(); ++is) {
                             if  ( !CheckFlags.condition(is->controlinfo(),params.Aflag) ) { /// If one or more of the analysis flags are set then will not process further! 
                                   Tseries.push_back(*is);
                             }
                          }
                             if (Tseries.size()==3) {
                                   //LOGINFO("Interpolating data ...");
                                   if (Tseries[0].original() > params.missing && Tseries[1].original()==params.missing && 
                                       Tseries[2].original() > params.missing){
                                       result = dbGate.select(MaxT, kvQueries::selectData(id->stationID(),215,YTime,YTime));
                                       result = dbGate.select(MinT, kvQueries::selectData(id->stationID(),213,YTime,YTime));
                                       if (MaxT.size()==1 && MinT.size()==1 && 
                                           MaxT.begin()->original() > -99.9 && MinT.begin()->original() > -99.9){

                                               LinInterpolated=0.5*(Tseries[0].original()+Tseries[2].original()); //this is calculate but not used
                                                                                                                  //maybe add a comparitive test??
                                               TanTaxInterpolated=0.5*(MinT.begin()->original()+MaxT.begin()->original());
                                               TanTaxInterpolated=round<float,1>(TanTaxInterpolated);

                                               //result = dbGate.select(Qc2InterpData, kvQueries::selectData(StationIds,pid,Tseries[1].obstime(),
                                                                                                           //Tseries[1].obstime() ));
                                               //Qc2D GSW(Qc2InterpData,StationList,params);
                                               //GSW.Qc2_interp(); 
                                               fixtime=Tseries[1].obstime();
                                               fixflags=Tseries[1].controlinfo();
                                               CheckFlags.setter(fixflags,params.Sflag);

// The logic to write results back to the database and inform kvServiceD
//  In this case the data we are working with (i.e. potentially right back to the db) is Tseries[1]

                      try {
                             /// Update if correction is out of TAN TAX range!
                             //if ( Tseries[1].corrected() <  MinT.begin()->original() || Tseries[1].corrected() >  MaxT.begin()->original() ) 
                             if ( ( Tseries[1].corrected() <  MinT.begin()->original() || Tseries[1].corrected() >  MaxT.begin()->original() )
                                  &&  CheckFlags.true_nibble(id->controlinfo(),params.Wflag,15,params.Wbool) )
                            {  
                                /// and a control in the configuration file to turn Wflag totally on or off!!!!
                                 {
                                        kvData d;                                                   
                                        d.set(Tseries[1].stationID(), Tseries[1].obstime(), Tseries[1].original(), Tseries[1].paramID(),
                                              Tseries[1].tbtime(), Tseries[1].typeID(), Tseries[1].sensor(), Tseries[1].level(), TanTaxInterpolated,                                                    fixflags, Tseries[1].useinfo(),
                                              Tseries[1].cfailed()+" Qc2 UnitT corrected was:"+StrmConvert(Tseries[1].corrected())+params.CFAILED_STRING );
                             // Set use info corresponding to controlinfo
                                        kvUseInfo ui = d.useinfo();
                                        ui.setUseFlags( d.controlinfo() );
                                        d.useinfo( ui );   
                             // write the data back
                                        LOGINFO("ProcessUnitT Writing Data "+StrmConvert(TanTaxInterpolated)+" "
                                                                            +StrmConvert(Tseries[1].stationID())+" "
                                                                            +StrmConvert(Tseries[1].obstime().year())+"-"
                                                                            +StrmConvert(Tseries[1].obstime().month())+"-"
                                                                            +StrmConvert(Tseries[1].obstime().day())+" "
                                                                            +StrmConvert(Tseries[1].obstime().hour())+":"
                                                                            +StrmConvert(Tseries[1].obstime().min())+":"
                                                                            +StrmConvert(Tseries[1].obstime().sec()) );
                                        dbGate.insert( d, "data", true); 
                             // fill structure to inform the serviced
                                        kvalobs::kvStationInfo::kvStationInfo DataToWrite(id->stationID(),id->obstime(),id->paramID());
                                        stList.push_back(DataToWrite);
                                   }
                              }
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
  ProcessTime.addHour(-1);
  }
return 0;
}

