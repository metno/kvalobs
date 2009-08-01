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
#include <puTools/miDate>
#include <puTools/miClock>
#include <memory>
#include <stdexcept>

#include "CheckedDataCommandBase.h"
#include "CheckedDataHelper.h"

#include <math.h>
//GNU Statistical library
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>



#include "scone.h"



using namespace kvalobs;
using namespace std;
using namespace miutil;

int 
ProcessImpl::
Process4D( ReadProgramOptions params )
{
// For gsl compilation
  int i, nseries;
  double xi, yi;
  double tt[100], pp[100]; // only set up for time series; max 100 points, an trap to catch errors ...

  int counter=0;
/// Need to integrate multiple handling of different type ids OR resolve this issue
/// by separate program that scan kvalobs database and identifies the value of
/// each duplicate measurement to use ...
  int pid=params.pid;
  int tid=params.tid;
  miutil::miTime stime=params.UT0;
  miutil::miTime etime=params.UT1;

  uint midpoint;
  uint minlower,minupper;
  uint maxlower,maxupper;
  int mcount;

  std::vector<int> gap_index;

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
  miutil::miDate PDate;
  double JulDec;
  long StartDay;
  double HourDec;
  ProcessTime = etime;

  std::vector<kvalobs::kvData> Tseries;

  int HW=params.StepH; /// carry window with the paramter StepH
  ///This algorithm steps back in time.
  GetStationList(StationList);  /// StationList is all the possible stations
  for (std::list<kvalobs::kvStation>::const_iterator sit=StationList.begin(); sit!=StationList.end(); ++ sit) {
     StationIds.push_back( sit->stationID() );
  }  /// Does this not have to go in the loop ...

///
/// The window is determined by the params.StepH (need to add a uniques parameter in config options for this), fed into HW
/// The time series selected is checked for missing values and the optimum number of niegbours 
///    x: value 
///    m: missing value
///
///  m x x x x x m m m m x x x x x x m   time --->
///    1       2         3         4
///
/// 1: maxlower
/// 2: minlower
/// 3: minupper
/// 4: maxupper
///
/// The indices for the missing values 'm' are held in gap_index.

  while (ProcessTime >= stime) 
  {
     XTime=ProcessTime;
     XTime.addHour(-HW);
     YTime=ProcessTime;
     YTime.addHour(HW);
     Tseries.clear();

             try {
                //result = dbGate.select(Qc2Data, kvQueries::selectMissingData(params.missing,pid,ProcessTime));
                result = dbGate.select(Qc2Data, kvQueries::selectMissingData(params.missing,pid,tid,ProcessTime));
              }
              catch ( dnmi::db::SQLException & ex ) {
                IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
              }
              catch ( ... ) {
                IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
              }
              if(!Qc2Data.empty()) {
                   for (std::list<kvalobs::kvData>::const_iterator id = Qc2Data.begin(); id != Qc2Data.end(); ++id) {
                          result = dbGate.select(Qc2SeriesData, kvQueries::selectData(id->stationID(),pid,tid,XTime,YTime));
                          for (std::list<kvalobs::kvData>::const_iterator is = Qc2SeriesData.begin(); is != Qc2SeriesData.end(); ++is) {
                             Tseries.push_back(*is);
                          }
                          mcount=0;
                          midpoint=0;
                          for (uint kkk=0;kkk<Tseries.size();++kkk){
                              if (Tseries[kkk].obstime() == ProcessTime) {
                                    midpoint=kkk;
                                    ++mcount;
                              }
                          }

                          minupper=midpoint;
                          minlower=midpoint;
                          maxupper=midpoint;
                          maxlower=midpoint;

                          if (midpoint > 0 && mcount == 1) {

                                 for (uint kkk=midpoint+1;kkk<Tseries.size();++kkk){
                                     if (Tseries[kkk].original() != params.missing){
                                         minupper=kkk;
                                         break;                                
                                     }
                                     if (kkk==Tseries.size()-1) minupper=midpoint; // no value found
                                 }

                                 for (uint kkk=midpoint-1;kkk>0;--kkk){
                                     if (Tseries[kkk].original() != params.missing){
                                         minlower=kkk;
                                         break;                                
                                     }
                                     if (kkk==1) minlower=midpoint; // no value found
                                 }

                                 if (minlower==midpoint || minupper==midpoint) {
                                     minlower=midpoint;
                                     minupper=midpoint;
                                 }

                                 if (minupper != midpoint && minlower != midpoint){     
                                      for (uint kkk=minupper+1;kkk<Tseries.size();++kkk){
                                          maxupper=kkk;
                                          if (Tseries[kkk].original() == params.missing){
                                              maxupper=kkk-1;
                                              break;                                
                                          }
                                      }
                                      for (uint kkk=minlower-1;kkk>0;--kkk){
                                          maxlower=kkk;
                                          if (Tseries[kkk].original() == params.missing){
                                              maxlower=kkk+1;
                                              break;                                
                                          }
                                      }
                                 }
                          }

                           
                    if (minupper != midpoint && minlower != midpoint){     
                           for (uint kkk=minlower+1;kkk<=minupper-1;++kkk){
                               result = dbGate.select(Qc2InterpData, kvQueries::selectData(StationIds,pid,Tseries[kkk].obstime(),
                                                                                                     Tseries[kkk].obstime() ));
                               Qc2D GSW(Qc2InterpData,StationList,params);
                               GSW.Qc2_interp(); 
                           }
                           nseries=0; 
                           PDate.setDate(Tseries[maxlower].obstime().year(),Tseries[maxlower].obstime().month(),Tseries[maxlower].obstime().day());
                           StartDay=PDate.julianDay();
                           for (uint lll=maxlower;lll<=maxupper;++lll){
                               PDate.setDate(Tseries[lll].obstime().year(),Tseries[lll].obstime().month(),Tseries[lll].obstime().day() );
                               JulDec=PDate.julianDay()+Tseries[lll].obstime().hour()/24.0 + 
                                                           Tseries[lll].obstime().min()/(24.0*60)+Tseries[lll].obstime().sec()/(24.0*60.0*60.0);
                               HourDec=(PDate.julianDay()-StartDay)*24.0 +Tseries[lll].obstime().hour() +
                                                                             Tseries[lll].obstime().min()/60.0+Tseries[lll].obstime().sec()/3600.0;
                               std::cout << "INPUT " << HourDec << " " << Tseries[lll].original() << std::endl;
                               if (Tseries[lll].original() != params.missing){
                                  tt[nseries]=HourDec;
                                  pp[nseries]=Tseries[lll].original();
                                  nseries=nseries+1;
                                  std::cout << "For Routine: " << HourDec << " " << Tseries[lll].original() << std::endl;
                               } 
                               else {
                                  gap_index.push_back(lll); // need to work out the new corrected values to pass back to kvalobs
                               }
                           }
// GSL interpolation routines
/// OBS! The length of the akima spline is longer than the interval requested ... is this a behaviour of the library or a bug ??? CHECK 
                           std::cout << "N: "<< nseries << std::endl;
                           std::cout << maxlower<<" "<<minlower<<" "<<minupper<<" "<<maxupper<< std::endl;
                           gsl_interp_accel *acc = gsl_interp_accel_alloc ();
                           gsl_spline *spline = gsl_spline_alloc (gsl_interp_akima, maxupper-maxlower);
                           gsl_spline_init (spline, tt, pp, maxupper-maxlower);
                           counter=0;
                           for (xi = tt[0]; xi < tt[nseries]; xi += 1.0)  {
                                 yi = gsl_spline_eval (spline, xi, acc);
                                 std::cout << counter << " INTERP " << xi << " " << yi << std::endl;
                                 counter++;
                           }
                           gsl_spline_free (spline);
                           gsl_interp_accel_free (acc);
                           for (std::vector<int>::const_iterator ig = gap_index.begin();ig != gap_index.end(); ++ig){
                                    std::cout <<"Gaps: "<< *ig << std::endl;
                           }
                           std::cout << "------------------------------------------------- " << std::endl;
                    }

// Add here the logic to write results back to the database and inform kvServiceD
                      //try {
                              //if ( CheckFlags.condition(id->controlinfo(),params.Wflag) ) {  // Do not overwrite data controlled by humans!
                              //if ( counter==1 )  {  // Do not overwrite data controlled by humans!
                                   //std::cout << "Value to write back" << std::endl;
                                   //std::cout << "... dummy block ..." << std::endl;
                                   //kvData d = *id;
                                   //kvUseInfo ui = d.useinfo();
                                   //ui.setUseFlags( d.controlinfo() );
                                   //d.useinfo( ui );   
                                   //dbGate.insert( d, "data", true); 
                                   //kvalobs::kvStationInfo::kvStationInfo DataToWrite(id->stationID(),id->obstime(),id->paramID());
                                   //stList.push_back(DataToWrite);
                              //if ( CheckFlags.condition(id->controlinfo(),params.zflag) ) {  
                                   //XP.push_back(d.original());
                                   //YP.push_back(d.corrected());
                              //}
                           //}
                       //}
                          //catch ( dnmi::db::SQLException & ex ) {
                            //IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
                            //std::cout<<"INSERTO> CATCH ex" << result <<std::endl;
                          //}
                          //catch ( ... ) {
                            //IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
                            //std::cout<<"INSERTO> CATCH ..." << result <<std::endl;
                          //}
                          //if(!stList.empty()){
                              //checkedDataHelper.sendDataToService(stList);
                              //stList.clear();
                          //}

                    gap_index.clear();
                    Tseries.clear();
                   }
              }
  ProcessTime.addHour(-1);
  }
return 0;
}

