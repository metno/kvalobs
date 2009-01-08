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

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qfont.h>
#include <qlayout.h>


#include "scone.h"



using namespace kvalobs;
using namespace std;
using namespace miutil;

int 
ProcessImpl::
Process4D( ReadProgramOptions params )
{
   int pid=params.pid;
   int tid=params.tid;
   miutil::miTime stime=params.UT0;
   miutil::miTime etime=params.UT1;
   std::string CIF=params.ControlInfoString;

  std::list<kvalobs::kvStation> StationList;
  std::list<int> StationIds;
  std::list<kvalobs::kvData> Qc2Data;
  std::list<kvalobs::kvData> Qc2SeriesData;
  std::list<kvalobs::kvData> ReturnData;
  bool result;

  kvalobs::kvStationInfoList  stList;
  CheckedDataHelper checkedDataHelper(app);

  kvalobs::kvDbGate dbGate( &con );

  miutil::miTime ProcessTime;
  miutil::miTime XTime;
  miutil::miTime YTime;
  ProcessTime = etime;



  int HW=params.StepH; /// carry window with the paramter StepH


  while (ProcessTime >= stime) 
  {
     

     XTime=ProcessTime;
     XTime.addHour(-HW);

     YTime=ProcessTime;
     YTime.addHour(HW);

     std::vector<float> Tseries;
     std::vector<float> Mseries;

     Tseries.clear();
     Mseries.clear();

     bool TOK=false;


     // Do station loop


     GetStationList(StationList);
     for (std::list<kvalobs::kvStation>::const_iterator sit=StationList.begin(); sit!=StationList.end(); ++ sit){
          StationIds.push_back( sit->stationID() );
     }

     for (std::list<kvalobs::kvStation>::const_iterator sit=StationList.begin(); sit!=StationList.end(); ++ sit){

             try {
                result = dbGate.select(Qc2Data, kvQueries::selectMissingData(params.missing,pid,ProcessTime));

                std::cout << kvQueries::selectData(params.missing,pid,YTime) << std::endl;
                std::cout << "....................................................." << std::endl;
              }
              catch ( dnmi::db::SQLException & ex ) {
                IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
              }
              catch ( ... ) {
                IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
              }


              if(!Qc2Data.empty()) {
                   for (std::list<kvalobs::kvData>::const_iterator id = Qc2Data.begin(); id != Qc2Data.end(); ++id) {
                          //select the data before and after the missing value
                          result = dbGate.select(Qc2SeriesData, kvQueries::selectData(id->stationID(),pid,id->typeID(),XTime,YTime));
                          //select the other parameters TAN, TAX


                          //result = dbGate.select(Qc2TanData, kvQueries::selectData(id->stationID(),pid,id->typeID(),XTime,YTime));
                          //result = dbGate.select(Qc2TaxData, kvQueries::selectData(id->stationID(),pid,id->typeID(),XTime,YTime));


                          ///result = dbGate.select(Qc2SeriesData, kvQueries::selectData(id->stationID(),pid,XTime,YTime)); To use typeid or not?????
                          for (std::list<kvalobs::kvData>::const_iterator is = Qc2SeriesData.begin(); is != Qc2SeriesData.end(); ++is) {
                             Tseries.push_back(is->original());
                             if (is->original() > -32767) TOK=true;
                             if (TOK) std::cout << *is << std::endl;
                          }
                          TOK=false;
                          //for (unsigned int iv=0;iv<Tseries.size();iv++) {
                          //    std::cout << Tseries[iv] <<" "<< " ... " << std::endl;
                          //} 
                          Tseries.clear();
                          std::cout << "..............." << std::endl;
                   }
                       //result = dbGate.select(Qc2Data, kvQueries::selectData(StationIds,pid,tid,XTime,YTime));
                       //Qc2D GSW(Qc2Data,StationList,params);
                       //GSW.Qc2_interp(); 
                       //Mseries=GSW.intp(); 
              }
                  //std::cout << "Actual" <<" "<< "Model"  << std::endl;
     }





              
       /** if(!ReturnData.empty()) {
       for (std::list<kvalobs::kvData>::const_iterator id = ReturnData.begin(); id != ReturnData.end(); ++id) {
                      try {
                           if (!id->controlinfo().flag(15)) {  // Do not overwrite data controlled by humans!
                                kvData d = *id;
                                kvUseInfo ui = d.useinfo();
                                ui.setUseFlags( d.controlinfo() );
                                d.useinfo( ui );   
                                dbGate.insert( d, "data", true); 
                                kvalobs::kvStationInfo::kvStationInfo 
                                     DataToWrite(id->stationID(),id->obstime(),id->paramID());
                                stList.push_back(DataToWrite);


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
          }
          if(!stList.empty()){
              checkedDataHelper.sendDataToService(stList);
              stList.clear();
          }
          ReturnData.clear();
       }         **/
  ProcessTime.addHour(-1);
  std::cout << "Timings : " << ProcessTime << std::endl;
  }

return 0;
}

