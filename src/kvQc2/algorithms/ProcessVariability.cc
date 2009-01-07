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
#include "PaperField.h"


#include "ProcessControl.h"




using namespace kvalobs;
using namespace std;
using namespace miutil;

int 
ProcessImpl::
Variability( ReadProgramOptions params )
{


// This code here as a test to get controlflags working...
  kvalobs::kvControlInfo testflags;
  testflags.set(12,2);
  testflags.set(13,2);
  testflags.set(15,0xA);
  ProcessControl Sedgemore;
  std::cout << Sedgemore.condition(testflags, params.zflag) << std::endl;


  for (std::vector<unsigned char>::const_iterator ik = params.Vfs.begin(); ik != params.Vfs.end(); ++ik){
    std::cout << *ik << std::endl;
  }


//  return 0;  /// temporarily do this for testing !!!
// --------------------------------------------


  QApplication a(0,0);
  PaperField Plotter;
  Plotter.setGeometry( 100, 100, 500, 355 );
  a.setMainWidget( &Plotter );
  Plotter.show();

  std::list<kvalobs::kvStation> StationList;
  std::list<int> StationIds;
  std::list<kvalobs::kvData> Qc2Data;
  std::list<kvalobs::kvData> ReturnData;
  bool result;

  miutil::miTime stime=params.UT0;
  miutil::miTime etime=params.UT1;
  int StepD=params.StepD;
  int StepH=params.StepH;
  int paramID=params.pid;

  std::cout << "Check values From Config File: " << stime << " to " << etime 
            << " StepDays= " << StepD << " StepHours= " << StepH
            << " " << paramID << std::endl;


  miutil::miTime ProcessTime;
  kvalobs::kvStationInfoList  stList;

  CheckedDataHelper checkedDataHelper(app);
  kvalobs::kvDbGate dbGate( &con );


  /// Possibly this should be checked for every time interval ?????
  GetStationList(StationList);
  for (std::list<kvalobs::kvStation>::const_iterator sit=StationList.begin(); sit!=StationList.end(); ++ sit) {
     StationIds.push_back( sit->stationID() );
  }

  double sum, mean, var, dev, skew, kurt;
  vector<float> fdata;

  ProcessTime = stime;

  int iday=0;

  while (ProcessTime <= etime) {
             try {
                result = dbGate.select(Qc2Data, kvQueries::selectData(StationIds,paramID,ProcessTime,ProcessTime));
              }
              catch ( dnmi::db::SQLException & ex ) {
                IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
              }
              catch ( ... ) {
                IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
              }

       if(!Qc2Data.empty()) {
                Qc2D GSW(Qc2Data,StationList,params);
                GSW.filter(fdata, -1, 20.0, -1.0, 0.0);
                computeStats(fdata.begin( ), fdata.end( ), sum, mean, var, dev, skew, kurt);
                std::cout << " " << fdata.size( );
                std::cout << " " << sum;
                std::cout << " " << mean;
                std::cout << " " << var;
                std::cout << " " << dev;
                std::cout << " " << skew;
                std::cout << " " << kurt;
                std::cout << endl;
                Plotter.AddPoint( iday,(int)(mean*10.0) );
                ++iday;
       }
              
       if(!ReturnData.empty()) {
          for (std::list<kvalobs::kvData>::const_iterator id = ReturnData.begin(); id != ReturnData.end(); ++id) {
                      try {
                           if (!id->controlinfo().flag(15)) {  // Do not overwrite data controlled by humans!

                
                                kvData d = *id;
                                kvUseInfo ui = d.useinfo();
                                ui.setUseFlags( d.controlinfo() );
                                d.useinfo( ui );   
                                                      // ideally move setting useinfo flags to a generic level 

                                std::cout << "_FLAGS_" << id->useinfo() << " " << ui << " " << id->controlinfo() << std::endl; 

                                //dbGate.insert( *id, "data", true);
                                dbGate.insert( d, "data", true); // see above
                                kvalobs::kvStationInfo::kvStationInfo DataToWrite(id->stationID(),id->obstime(),id->paramID());
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
       }
  ProcessTime.addHour(StepH);
  }
a.exec();
return 0;
}

