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
#include "ProcessControl.h"

#include "scone.h"

#include "epsx.h"



using namespace kvalobs;
using namespace std;
using namespace miutil;


int 
ProcessImpl::
Redistribute( ReadProgramOptions params )
{

  miutil::miTime stime=params.UT0;
  miutil::miTime etime=params.UT1;
  const int pid=params.pid;
  const int tid=params.tid;

  std::vector<float> XP;
  std::vector<float> YP;

  ProcessControl CheckFlags;

  //QApplication a(0,0);
  //PaperField Plotter(-100,-100,700,700,0,0);
  //Plotter.setGeometry( 100, 100, 500, 355 );
  //a.setMainWidget( &Plotter );
  //Plotter.show();

  std::list<kvalobs::kvStation> StationList;
  std::list<kvalobs::kvStation> ActualStationList;
  std::list<int> StationIds;
  std::list<int> StationIdsActual;
  std::list<kvalobs::kvData> Qc2Data;
  std::list<kvalobs::kvData> ReturnData;
  bool result;

  kvalobs::kvStationInfoList  stList;
  CheckedDataHelper checkedDataHelper(app);

  kvalobs::kvDbGate dbGate( &con );

  miutil::miTime ProcessTime;

  GetStationList(StationList);  /// StationList is all the possible stations
  for (std::list<kvalobs::kvStation>::const_iterator sit=StationList.begin(); sit!=StationList.end(); ++ sit) {
     StationIds.push_back( sit->stationID() );
  }

  ProcessTime = stime;

  while (ProcessTime <= etime) {


             try {
              result = dbGate.select(Qc2Data, kvQueries::selectData(StationIds,pid,tid,ProcessTime,ProcessTime));
///<<<<<<< .mine
              //std::cout << kvQueries::selectData(StationIds,pid,tid,ProcessTime,ProcessTime) << std::endl;
///=======
              /// put back the typeid becuase we have two measurements from the same station with different typeid            

              //result = dbGate.select(Qc2Data, kvQueries::selectData(StationIds,pid,ProcessTime,ProcessTime));
                                  ///do not mind which type id is used here
                                  ///only set the time id for selecting paramters to redistribute.
///>>>>>>> .r679
              }
              catch ( dnmi::db::SQLException & ex ) {
                IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
              }
              catch ( ... ) {
                IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
              }

      if(!Qc2Data.empty()) {

                ///Find the actual StationIds present
                ///for (std::list<kvalobs::kvData>::const_iterator sid = Qc2Data.begin(); sid != Qc2Data.end(); ++sid) {
                    ///StationIdsActual.push_back( sid->stationID() );
                    ///ActualStationList.push_back( StationList(where sid->stationID() ) is the chap ?????
                ///}

                Qc2D GSW(Qc2Data,StationList, params,"Generate Missing Rows");
                GSW.Qc2_interp(); 
                GSW.distributor(StationList,ReturnData,0);
                GSW.write_cdf(StationList);
       }

       StationIdsActual.clear();
              
       if(!ReturnData.empty()) {
          for (std::list<kvalobs::kvData>::const_iterator id = ReturnData.begin(); id != ReturnData.end(); ++id) {
                      try {
                           //if (!id->controlinfo().flag(15)) {  // Do not overwrite data controlled by humans!
                           if ( CheckFlags.condition(id->controlinfo(),params.Wflag) ) {  // Do not overwrite data controlled by humans!
                                kvData d = *id;
                                kvUseInfo ui = d.useinfo();
                                ui.setUseFlags( d.controlinfo() );
                                d.useinfo( ui );   
                                dbGate.insert( d, "data", true); 
                                kvalobs::kvStationInfo::kvStationInfo DataToWrite(id->stationID(),id->obstime(),id->paramID());
                                std::cout << "ZZZ To Write: "<< id->original() << " " << id->corrected() << std::endl;
                                stList.push_back(DataToWrite);

                           //if (id->controlinfo().flag(13)==9) {  
                           if ( CheckFlags.condition(id->controlinfo(),params.zflag) ) {  
                           //if (id->controlinfo().flag(13)==params.zflag[15]) {    // This is just to test parameter control flag passing
                                //Plotter.AddPoint( (int)(10*d.original()),(int)(10*d.corrected()) ); 
                                XP.push_back(d.original());
                                YP.push_back(d.corrected());
                                std::cout << "ZZZ Plotted: "<< d.original() << " " << d.corrected() << std::endl;
                                // Note the plot here is used when test data is being analysed
                                // for the optimisation of algorithms.
                                // 10* just for visibility on the simple pixmap!!
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
          }
          if(!stList.empty()){
              checkedDataHelper.sendDataToService(stList);
              stList.clear();
          }
          ReturnData.clear();
       }
  ProcessTime.addDay();

  }
epsx Fish2(XP,YP,"Plout.eps");
//std::cout << "Chi-squared = " << Plotter.linearChi2() << std::endl;
//a.exec();
Qc2D GSW(Qc2Data,StationList,params);
GSW.distributor(StationList,ReturnData,1); // temporary solution for memory cleanup
return 0;
}

