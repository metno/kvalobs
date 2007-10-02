/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: QABase.cc,v 1.2.6.2 2007/09/27 09:02:38 paule Exp $                                                       

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
#include <fstream>
#include <iostream>

/* Created by DNMI/PU: a.christoffersen@dnmi.no
   at Tue Jun  4 13:58:11 2002 */

#include <puTools/miTime>
#include <puTools/miString>
#include "../include/kvQABaseControl.h"
#include <dnmithread/mtcout.h>
#include "../include/kvCronString.h"


// TEST of QABase module

using namespace miutil;
using namespace std;
using namespace kvalobs;



int main(int argc, char** argv)
{
  
  CronString CS1("* * * * *"), CS2("30 * * * *"), CS3("30 12 * * *"),
    CS4("30 12 15 * *"), CS5("30 12 15 6 *"),  CS6("30 12 15 6 2003");
  
  miTime t1(2003,6,15,13,30,0), t2(2004,7,15,12,30,0);

  cerr << "TIME:" << t1 << " against string:" << CS1.str() << " gives "
       << (CS1.active(t1) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t1 << " against string:" << CS2.str() << " gives "
       << (CS2.active(t1) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t1 << " against string:" << CS3.str() << " gives "
       << (CS3.active(t1) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t1 << " against string:" << CS4.str() << " gives "
       << (CS4.active(t1) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t1 << " against string:" << CS5.str() << " gives "
       << (CS5.active(t1) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t1 << " against string:" << CS6.str() << " gives "
       << (CS6.active(t1) ? "ACTIVE" : " NO") << endl;

  cerr << "TIME:" << t2 << " against string:" << CS1.str() << " gives "
       << (CS1.active(t2) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t2 << " against string:" << CS2.str() << " gives "
       << (CS2.active(t2) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t2 << " against string:" << CS3.str() << " gives "
       << (CS3.active(t2) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t2 << " against string:" << CS4.str() << " gives "
       << (CS4.active(t2) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t2 << " against string:" << CS5.str() << " gives "
       << (CS5.active(t2) ? "ACTIVE" : " NO") << endl;
  cerr << "TIME:" << t2 << " against string:" << CS6.str() << " gives "
       << (CS6.active(t2) ? "ACTIVE" : " NO") << endl;

  exit(1);

//   const int numstat= 5;
//   int stations[numstat]= {1001,1032,1865,1234,2001};
//   miTime starttime(2002,6,3,12,0,0);
//   miTime stoptime (2002,6,5,12,0,0);
//   miTime obstime;
  
//   kvQABaseControl control;
  
//   if (argc > 1){
//     miString a= argv[1];
//     if (a == "loop"){
      
//       for (obstime= starttime; obstime<stoptime; obstime.addHour(3)){
// 	for (int stat=0; stat<numstat; stat++){
// 	  int station= stations[stat];
// 	  control.runChecks(station,  // stationid
// 			    obstime); // observation-time
// 	}
// 	COUT( "Hit return to continue:");
// 	char c= getchar();
//       }
//       return 0;
//     }
//   } else {
//     COUT( "===============================" << std::endl);
//     COUT( "Use:" << argv[0] << " [loop]"    << std::endl);
//     COUT( "===============================" << std::endl);
//   }
  
//   obstime= starttime;

//   control.runChecks(1001,      // stationid
// 		    obstime);  // observation-time
  
  return 0;
};
