/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: klTestdata.cc,v 1.1.6.1 2007/09/27 09:02:47 paule Exp $                                                       

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

#include <dbdrivermgr.h>
#include <kvDbGate.h>
#include <kvStation.h>
#include <kvParam.h>
#include <map>
#include <vector>
#include <set>
/*
  Created by DNMI/PU: j.schulze@met.no
  at Wed Mar 13 13:03:37 2002 
  
  Program to generate maps for an kvtestdata upload from klima into
  the KVALOBS-database    
  creating 
*/

using namespace std; 
using namespace miutil;
using namespace kvalobs;
using namespace dnmi::db;


int transformVV(int vv) {
  if (vv < 50)
    return vv * 100;

  if( vv < 81 )
    return (vv - 50 ) * 1000;
  
  if (vv < 90 )
    return ( vv - 80 ) * 5000 + 30000;

  if ( vv == 90 )
    return 0;
  if ( vv == 91 )
    return 50;
  if ( vv == 92 )
    return 200;
  if ( vv == 93 )
    return 500;
  if ( vv == 94 )
    return 1000;
  if ( vv == 95 )
    return 2000;
  if ( vv == 96 )
    return 4000;
  if ( vv == 97 )
    return 10000;
  if ( vv == 98 )
    return 20000;
  if ( vv == 99 )
    return 50000;
  return 0;
}







typedef struct par_lvl {
  int par;
  int lvl;
  int hour;
  par_lvl()  : par(0), lvl(0),hour(0) {}
  par_lvl(const int p, const int l=0) : par(p), lvl(l), hour(0) {}
  void set(int p, int l=0 ) { par =p; lvl = l;hour=0;}

} par_lvl;

int main(int argc, char** argv)
{
  if(argc < 2) {
    cerr << "need a Inputfilename from klimadb .... exiting ! " << endl;
    cerr << "You can use the transformed file by psql < testdata.dat" << endl;
    




    exit(0);
  }
  
  ifstream in(argv[1]);
  if(!in) {
    cerr << "Unable to open file " << argv[1] << "  .... exiting ! " << endl;
    exit(0);

  }
  ofstream tfile("testdata.dat");
  if(!tfile)  {
    cerr << "Unable to open testdata.dat " << endl;
    exit(0);
  }
    
  set<int>      posfail;
  set<miString> parfail;
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

    // Creating station list ...

    list<kvStation> pos;
    gate.select(pos);

    list<kvStation>::iterator po=pos.begin();
    set<int> posmap;

    for(;po!=pos.end();po++) 
      posmap.insert( po->stationID());
      
    // Creating parameter list ...

    
    list<kvParam> par;
    gate.select(par);

    list<kvParam>::iterator pa=par.begin();
    par_lvl pl;

    map<miString,par_lvl> parmap,nmap;

    for(;pa!=par.end();pa++) {
      pl.set(pa->paramID(),0);
      parmap[pa->name()] = pl;
    }
    delete con;

    miString line;
    vector<miString> words;
    ifstream special("specialparmap.txt");
    
    if(!special) {
      cout << " Attention!!! no special parmap for V1 etc found "
	   << " looking for specialparmap.txt ... " << endl;
    }
    else {
      bool typen = false;
      while(special) {
	getline(special,line);
	line.trim();	
	if (line =="<TYPEN>") {
	  typen = true;
	  nmap = parmap;
	}
	if(!line.exists())
	  continue;
	words = line.split();

	if(words.size() < 4 )
	  continue;
       
	pl.par  = parmap[ words[1] ].par;
	pl.lvl  = atoi(words[2].cStr());
	pl.hour = atoi(words[3].cStr());
	
	if(typen)
	   nmap[  words[0]  ]  = pl;
	else
	  parmap[ words[0]  ] = pl;
      }
    }

    miTime  valid;
    miTime  tbt=miTime::nowTime();
    kvData data;
    int y,m,d,h;  
    int typ = 0; // test dataset

    tfile << "COPY  test_data from stdin using delimiters \',\';" << endl;


    while(in) {
      getline(in,line);
      line.trim();
      
      if(!line.exists())
	continue;
      words = line.split();

      if(words.size() < 8 )
	continue;
      int p = atoi(words[0].cStr());
      
      if(!posmap.count(p)) {
	posfail.insert(p);
	continue;
      }
      y     = atoi(words[1].cStr());
      m     = atoi(words[2].cStr());
      d     = atoi(words[3].cStr());
      h     = atoi(words[4].cStr());
      miString stype = words[6];

      float org = atof(words[5].cStr());
      miString  pname = words[7];
      valid.setTime(y,m,d,h,0,0);
      par_lvl pl;

      if(stype =="N") {
	if (!nmap.count(pname) ) {
	  parfail.insert(pname);
	  continue;
	}
	pl = nmap[pname];
	valid.addHour(pl.hour);
      }
      else {
	if (!parmap.count(pname) ){
	  parfail.insert(pname);
	  continue;
	}
	pl = parmap[pname];
      }

      if(pl.par == 273 ) {
	int a = int(org);
	org = transformVV(a);
      }

      data.set(p,valid,org,pl.par ,tbt,typ, pl.lvl);
      tfile << data.toUpload() << endl;
	
    }
    tfile << "\\." << endl;


    // Clean up and go....
    
    if(posfail.size()) {
      set<int>::iterator posi = posfail.begin();
      cout << "failed for positions: " << endl;
      for(;posi!=posfail.end();posi++)
	cout << " > " << *posi << endl;
    }
    if(parfail.size()) {
      set<miString>::iterator pari = parfail.begin();
      cout << "failed for parameters: " << endl;
      for(;pari!=parfail.end();pari++)
	cout << " > " << *pari << endl;
    }
    return 0;
};







