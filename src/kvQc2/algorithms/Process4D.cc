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
Process4D( ReadProgramOptions params )
{
 /// Need to integrate multiple handling of different type ids OR resolve this issue
 /// by separate program that scan kvalobs database and identifies the value of
 /// each duplicate measurement to use ...
   int pid=params.pid;
   miutil::miTime stime=params.UT0;
   miutil::miTime etime=params.UT1;
   std::string CIF=params.ControlInfoString;

  uint midpoint;
  uint minlower,minupper;
  uint maxlower,maxupper;
  int mcount;

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


  int HW=params.StepH; /// carry window with the paramter StepH
  ///This algorithm steps back in time.
  GetStationList(StationList);  /// StationList is all the possible stations
  for (std::list<kvalobs::kvStation>::const_iterator sit=StationList.begin(); sit!=StationList.end(); ++ sit) {
     StationIds.push_back( sit->stationID() );
  }  /// Does this not have to go in the loop ...

  while (ProcessTime >= stime) 
  {
     XTime=ProcessTime;
     XTime.addHour(-HW);
     YTime=ProcessTime;
     YTime.addHour(HW);
     Tseries.clear();

             try {
                result = dbGate.select(Qc2Data, kvQueries::selectMissingData(params.missing,pid,ProcessTime));
                          //std::cout << "QueryA: "<< kvQueries::selectMissingData(params.missing,pid,ProcessTime) << std::endl;
                //std::cout << kvQueries::selectMissingData(params.missing,pid,ProcessTime) << std::endl;
                //std::cout << "....................................................." << std::endl;
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
                          mcount=0;
                          midpoint=0;
                          for (uint kkk=0;kkk<Tseries.size();++kkk){
                              if (Tseries[kkk].obstime() == ProcessTime) {
                                    //std::cout << "middle Point: " << kkk << std::endl;
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
                                     //std::cout << kkk << std::endl;
                                     if (Tseries[kkk].original() != params.missing){
                                         minupper=kkk;
                                         break;                                
                                     }
                                     if (kkk==Tseries.size()-1) minupper=midpoint; // no value found
                                 }

                                 for (uint kkk=midpoint-1;kkk>0;--kkk){
                                     //std::cout << kkk << std::endl;
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

                                 //std::cout << minlower << " " << minupper << std::endl;
                                 //////for (uint kkk=minlower;kkk<=minupper;++kkk){
                                      //if (kkk==midpoint) std::cout << "Midpoint ..." << std::endl;
                                      //std::cout <<"Min Range: "<< Tseries[kkk] << std::endl;
                                 //}

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

                                 std::cout << Tseries[maxlower].stationID() << " START ";
                                 for (uint lll=maxlower;lll<=minlower;++lll){
                                      //std::cout <<"Min Range: "<< Tseries[lll] << std::endl;
                                      std::cout << Tseries[lll].obstime() << ">"<<Tseries[lll].original() << ",";
                                      //std::cout << Tseries[lll].original() << ",";
                                 }




                              //std::cout << "Mx: "<< Tseries[minlower] << std::endl; 
                              for (uint kkk=minlower+1;kkk<=minupper-1;++kkk){
                                     result = dbGate.select(Qc2InterpData, kvQueries::selectData(StationIds,pid,Tseries[kkk].obstime(),
                                                                                                           Tseries[kkk].obstime() ));
                                     Qc2D GSW(Qc2InterpData,StationList,params);

                                     //for (std::map<int,int>::iterator it=GSW.stindex.begin(); it!=GSW.stindex.end(); ++it){
                                          //std::cout << (*it).second << " " << (*it).first << std::endl;
                                          //std::cout << (*it).second << " " << GSW.stid_[(*it).second] << std::endl;
                                     //}
                               
                                     GSW.Qc2_interp(); 

                                     //std::cout << "Match: "
                                               //<< "S: " << Tseries[kkk].stationID() << " "
                                               //<< "T: " << Tseries[kkk].obstime() << " "
                                               //<< "O: " << Tseries[kkk].original() << " "
                                               //<< "C: " << Tseries[kkk].corrected() << " "
                                               //<< "TI: " << Tseries[kkk].typeID() << " "
                                               //<< "TI: " << GSW.typeid_[GSW.stindex[Tseries[kkk].stationID()]] << " "
                                               //<< "S: " << GSW.stid_[GSW.stindex[Tseries[kkk].stationID()]] << " "
                                               //<< "I: " << GSW.intp_[GSW.stindex[Tseries[kkk].stationID()]] <<  " "
                                               //<< "T: " << GSW.obstime_[GSW.stindex[Tseries[kkk].stationID()]] <<  " "
                                               //<< "C: " << GSW.corrected_[GSW.stindex[Tseries[kkk].stationID()]] << " *** " 
                                               //<< "Index " << GSW.stindex[Tseries[kkk].stationID()] << " "
                                               //<< std::endl;
                                       std::cout << Tseries[kkk].obstime()<<">"<<Tseries[kkk].original() << "," << GSW.intp_[GSW.stindex[Tseries[kkk].stationID()]] << ",";
                                       //std::cout << Tseries[kkk].original() << "," << GSW.intp_[GSW.stindex[Tseries[kkk].stationID()]] << ",";
                             }
                              //std::cout << "Mx: "<< Tseries[minupper] << std::endl; 
                                           //std::cout << Tseries[minupper].original();
                                           std::cout << Tseries[minupper].obstime() << ">" << Tseries[minupper].original();
                              for (uint lll=minupper+1;lll<=maxupper;++lll){
                                           //std::cout <<"Max Range: "<< Tseries[lll] << std::endl;
                                           std::cout<<"," << Tseries[lll].obstime() << ">" << Tseries[lll].original();
                                           //std::cout<<","<< Tseries[lll].original();
                                      }
                                      std::cout << " FINISH" << std::endl;
                          }
                          Tseries.clear();
                   }
              }
  ProcessTime.addHour(-1);
  //std::cout << "Timings : " << ProcessTime << std::endl;
  }
return 0;
}

