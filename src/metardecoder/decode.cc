/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decode.cc,v 1.2.2.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#include "kvMetarDecoder.h"
#include <string>
#include <vector>

#include <db/dbdrivermgr.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvStation.h>


///  METAR DECODING TEST PROGRAM ... LOOKS TERRIBLY ALIKE
///  THE SAME ROUTINE IN SYNOP ....  


/// Created by DNMI/PU: j.schulze@met.no
/// at Wed Apr  2 08:34:13 2003 

using namespace std; 
using namespace miutil;
using namespace kvalobs;
using namespace dnmi::db;

int main(int argc, char** argv)
{
  kvMetarDecoder           mdec;
  list<kvalobs::kvStation> pos;

  /// TEST STATION LIST (just to make this go round)

  string kvalobs = getenv("KVALOBS");

  if(kvalobs.empty() ) {
    cerr << "KVALOBS: environment not defined ... exit " << endl;
    return 1;
  }

  string driver = kvalobs + "/lib/db/pgdriver.so";
  string driveID;

  string constr("host=rime dbname=kvalobs user=kvalobs password=kvalobs12");
  Connection    *con;
  DriverManager manager;
    
    if(!manager.loadDriver(driver, driveID)){
      cerr << "Can't load driver <" << driver << ">\n" << manager.getErr() << endl;
      return 1;
    }
    
    con=manager.connect(driveID, constr);
    
    if(!con){
	cerr << "Can't create connection to <" << driveID << endl;
	return 1;
    }
    cerr << "Connected to <" << driveID <<">"<< endl;

 
    kvDbGate gate(con);

    /// Creating station list ...

    gate.select(pos);

   
  ifstream in((argc > 1 )? argv[1] : "test.metar");
  vector<string> tst;
  string tmp;
  while(in) {
    getline(in,tmp);
    if(tmp.empty())
      continue;
    tst.push_back(tmp);
  }

  /// THIS IS THE REAL USER INTERFACE TO METARDECODER ....


  /// ATTENTION! you need a file with weather-contents (static c-stuff)
  /// this comes in a file weather.tab --- 
  /// without this initialisation -> NO METAR ... ever ....

  mdec.initialise(pos);

  list<kvalobs::kvData>               data; 
  list<kvalobs::kvData>::iterator     ditr; 
  list<kvalobs::kvTextData>           txt;
  list<kvalobs::kvTextData>::iterator titr;

  for( int i=0; i< tst.size(); i++ ) {
    cout << "[7;34mDECODING:----------------------[0;0m" << endl;

    if (mdec.decode(tst[i])) {
      data = mdec.Data();
      ditr = data.begin();
      
      cout << "[1;1mSending: DATA-------------\n[0m";
      for(;ditr!=data.end();ditr++)
	cout << ditr->toSend()  << endl; 
      

      txt  = mdec.Text();
      titr = txt.begin();

      cout << "[1;1mSending: TEXT-------------\n[0m";
      for(;titr!=txt.end();titr++)
      cout << titr->toSend()  << endl; 
      
    }
    else {
      kvalobs::kvRejectdecode  reject = mdec.rejected();
      cout <<   "[1;31mRejected: ------------\n[0;0m" << reject.toSend() << endl;
    }
  }
  return 0;
};


