/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kv2metdat.cc,v 1.9.2.2 2007/09/27 09:02:20 paule Exp $                                                       

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
#include <list>
#include <vector>

#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvParam.h>
#include <puTools/miString>
#include <kvalobs/kvPath.h>


///  Created by met.no/PU: j.schulze@met.no
///  at Wed Dec 11 14:27:37 2002 
///  
///  Program to generate metdat-interpolation input files from 
///  the KVALOBS-database    
///  creating 
///  stations.kvalobs    // lat / lon / id
///  parammap.kvalobs    // name -> id
///  The output is semi-static and will be updated every
///  run. If the database  is offline or busy, the update 
///  is skipped and the last files will be used......

using namespace std; 
using namespace miutil;
using namespace kvalobs;
using namespace dnmi::db;

int main(int argc, char** argv)
{
  string home    = getenv("HOME");
  string constr  = "  ";


  if( home.empty() ) {
	  cerr << "The HOME environment variable must be set!" << endl;
	  return 1;
  }
	  
  
  string passwdfile(home+"/.kvpasswd");
  ifstream passf(passwdfile.c_str());

  vector<miString> words;
  miString line;
  
  while(passf) {
    getline(passf,line);
    words=line.split();
    if(words.size() < 2)
      continue;
    
    if(words[0]=="dbpass") {
   	 if( ! words[1].empty() )
   		 constr = "user=kvalobs dbname=kvalobs host=localhost password="+ words[1];
   	 else
   		 constr = "user=kvalobs dbname=kvalobs host=localhost";
   	 
      break;
    }
  }
  
  if( constr.empty() ) {
	  constr = "user=kvalobs dbname=kvalobs host=localhost";
  }
  
  string driver( kvPath("pkglibdir")+"/db/pgdriver.so");
  string driveID;


  Connection    *con;
  DriverManager manager;
    
    if(!manager.loadDriver(driver, driveID)){
      cerr << "Can't load driver <" << driver << ">\n" << manager.getErr() << endl;
      return 1;
    }
    
    con=manager.connect(driveID,constr);
    
    if(!con){
	cerr << "Can't create connection to <" << driveID << ">" << endl;
	return 1;
    }
    cerr << "Connected to <" << driveID <<">"<< endl;

 
    kvDbGate gate(con);

    // Creating station list ...

    list<kvStation> pos;
    gate.select(pos," where (lat!=0 or lon!=0) and static=true and stationid < 100000");

    list<kvStation>::iterator po=pos.begin();

    ofstream opo("stations.kvalobs");
    
    for(;po!=pos.end();po++) 
      opo << "3,"  << po->lat() << "," <<  po->lon() << ", \'" << po->stationID()
	  << "\',  2,  0000, 0000,   0,0," << endl;
    opo << "0, 0000, 0000,   \'*\'  0=SLUTT  1=I,J  2=X,Y  3=B,L  4=B,L(GRAD*100+MIN)\n";
    

    // Creating parameter list ...

    
    list<kvParam> par;
    gate.select(par);

    list<kvParam>::iterator pa=par.begin();

    ofstream opa("parmap.kvalobs");
    
    for(;pa!=par.end();pa++) 
      opa << pa->name() << ":" << pa->paramID() << endl;

    // Clean up and go....

    delete con;
    return 0;
};







