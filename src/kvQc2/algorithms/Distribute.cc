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
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include "Distribute.h"
#include "tround.h"
#include <dnmithread/mtcout.h>
#include <milog/milog.h>
#include <map>

using namespace std;
using namespace miutil;
using namespace dnmi;


Distribute::Distribute(const std::list<kvalobs::kvStation> & slist,ReadProgramOptions PPP)
{
    params=PPP;
} 

///Add a row to the data object holding items to be redistributed. 
void
Distribute::
add_element(int & sid, float & data, float & intp, float & corr, float & newd, miutil::miTime & tbtime, miutil::miTime & obstime, int & sensor, int & level, int & d_tid, kvalobs::kvControlInfo & d_control, kvalobs::kvUseInfo & d_use, miutil::miString & cfailed)
{
    dst_data[ sid ].push_back(data);
    dst_intp[ sid ].push_back(intp);
    dst_corr[ sid ].push_back(corr);
    dst_newd[ sid ].push_back(newd);
        
    dst_tbtime[ sid ].push_back(tbtime);
    dst_time[ sid ].push_back(obstime);
    
    d_sensor[ sid ].push_back(sensor);
    d_level[ sid ].push_back(level);

    d_typeid[ sid ].push_back(d_tid);
    d_controlinfo[ sid ].push_back(d_control);
    d_useinfo[ sid ].push_back(d_use);
    d_cfailed[ sid ].push_back(cfailed);
}

/// Clear all data from the redistribution data object.
void
Distribute::
clear_all()
{
    dst_data.clear();                
    dst_intp.clear();                
    dst_corr.clear();                
    dst_newd.clear();                
    dst_tbtime.clear();                
    dst_time.clear();                
    d_sensor.clear();                
    d_level.clear();                
    d_typeid.clear();                
    d_controlinfo.clear();                
    d_useinfo.clear();                
    d_cfailed.clear();                 
}

/// Clear   single station entry from the redistribution data object.
void
Distribute::
clean_station_entry(int & sid)
{
    dst_data[ sid ].clear();                
    dst_intp[ sid ].clear();                
    dst_corr[ sid ].clear();                
    dst_newd[ sid ].clear();                
    dst_tbtime[ sid ].clear();                
    dst_time[ sid ].clear();                
    d_sensor[ sid ].clear();                
    d_level[ sid ].clear();                
    d_typeid[ sid ].clear();                
    d_controlinfo[ sid ].clear();                
    d_useinfo[ sid ].clear();                
    d_cfailed[ sid ].clear();                

}




/// Algorithm to redistribute data based on interpolated model data.  
/// Currently rewrites all data to one year in the future (better for running tests in 
/// development phase....)
void
Distribute::
RedistributeStationData(int & sid, std::list<kvalobs::kvData>& ReturnData)
{
        kvalobs::kvData ReturnElement;
        miutil::miTime fixtime;
        kvalobs::kvControlInfo fixflags;

	int stid = sid;
        int irun = 0;
        int available_data=1;  // recode as BOOL !!!
        int index=dst_data[ stid ].size()-1 ;
        int sindex=index;
        float accval = dst_data[ stid ][ dst_data[ stid ].size() -1 ];
        float original_accval = accval;   // need this since if it is -1 need to set all redistributed elements
                                          // to -1
        if (accval == -1.0) {
        	accval=0.0;
        }

        float sumint = 0.0;
        float missing_val = params.missing;
        while (missing_val == params.missing && index != 0) {   //works out how long the series to redistribute is
        //float missing_val = -32767;
        //while (missing_val == -32767 && index != 0) {   //works out how long the series to redistribute is
              missing_val=dst_data[ stid ][ index - 1 ];
              ++irun;
              --index;
        }
                       
        if (irun>1) {
           for (int k=sindex; k>=sindex-irun ; --k) {
                 if (dst_intp[ stid ][ k ] == -10.0) available_data=0;  // this is set if any of the points are unavailable
                 sumint= dst_intp[ stid ][ k ] + sumint; 
           }
     
           if (available_data && sumint > 0.0) {
               float normaliser=accval/sumint;
               float roundSum=0.0;
               float roundVal;
               //for (int k=sindex-irun+1; k<=sindex ; ++k) {
               for (int k=sindex-irun; k<=sindex ; ++k) {

                  // Perform redistribution.
                  dst_newd[ stid ][ k ] = dst_intp[ stid ][ k ] * normaliser;
                  // Set flags and do housecleaning indicating that a redistribution has been done
                  roundVal=round<float,1>(dst_newd[ stid ][ k ]);
                  roundSum += roundVal;  // Need to check roundSum does not deviate too much from accval
                  if (original_accval == -1.0) roundVal=-1.0; 

                  //if ( k==sindex ) std::cout << "SUMCHECK " << accval << " " <<roundSum << std::endl;

                  fixtime=dst_time[ stid ][ k ];
                  fixtime.addDay(366);  // FOR TESTING ... REMOVE THIS WHEN THIS CODE IS READY
                  ///fixtime.addDay(366);  *** FOR TESTING ... REMOVE THIS WHEN THIS CODE IS READY



                  fixflags=d_controlinfo[ stid ][ k ];
                  ControlFlag.setter(fixflags,params.Sflag);
                  //fixflags.set(12,8);    // Just set the flag value to 8 for now to indicate
                                         // redistribution ... full specification is pending                     
                  //fixflags.set(9,3);     // Space to define things in fstat                            
                                         // setting this will trigger Qc2done status 

                  std::cout << "Algorithm Test Mode !!!!!!!!!!!!!!!"<< std::endl;

                   std::cout << "RESULTS: "           <<    "\"" 
                            << stid                        << "\",\"" 
                            << dst_time[ stid ][ k ]       << "\",\"" 
                            << dst_tbtime[ stid ][ k ]     << "\",\"" 
                            << dst_data[ stid ][ k ]       << "\",\"" 
                            << dst_intp[ stid ][ k ]       << "\",\"" 
                            << dst_corr[ stid ][ k ]       << "\",\"" 
                            << dst_newd[ stid ][ k ]       << "\",\"" 
                            << d_typeid[ stid ][ k ]       << "\",\"" 
                            << d_cfailed[ stid ][ k ]      << "\",\"" 
                            << d_controlinfo[ stid ][ k ]  << "\",\"" 
                            << fixflags                    << "\",\""                  
                            << d_useinfo[ stid ][ k ]      << "\"" << std::endl; 

                  if (dst_corr[ stid ][ k ]==-1) {dst_corr[ stid ][ k ]=0;}    // For algorithm testing
                  if (dst_data[ stid ][ k ]==-1) {dst_data[ stid ][ k ]=0;}    // For algorithm testing
                  ReturnElement.set(stid,fixtime,dst_corr[ stid ][ k ],110,  // For use in algorithm
                                                                               // varitional tests
                  //ReturnElement.set(stid,fixtime,dst_data[ stid ][ k ],110,
                                dst_tbtime[ stid ][ k ],d_typeid[ stid ][ k ], d_sensor[ stid ][ k ],
                                d_level[ stid ][ k ], roundVal,fixflags, 
                                d_useinfo[ stid ][ k ], d_cfailed[ stid ][ k ]+" Qc2-R");
                  ReturnData.push_back(ReturnElement);
               }
           }
        }
}

