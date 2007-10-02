/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvsynopdparse.cc,v 1.5.6.1 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <miconfparser/miconfparser.h>
#include "StationInfoParse.h"
#include <milog/milog.h>

using namespace std;
using namespace miutil::conf;

void 
use(int retval);

void 
readWmonoFromFile(const string &file, list<long> &wmoList);

bool
readConfFile(const string &file,  std::list<StationInfoPtr> &stationList);

int
main(int argn, char **argv)
{
  string                    file;
  string                    wmonoFile;
  std::list<StationInfoPtr> stationList;
  bool                      listWmono=false;
  bool                      listconf=false;
  bool                      listconftab=false;
  bool                      check=false;
  bool                      hasCmd=false;
  
  milog::Logger::logger().logLevel(milog::INFO);
  
  for(int i=1; i<argn; i++){
    if(string(argv[i])=="-f"){
      i++;

      if(i<argn)
	file=argv[i];
      
      if(file.empty() || file[0]=='-'){
	cerr << "Missing file!" << endl<<endl;
	use(1);
      }
    }else if(string(argv[1])=="-w"){
      if(hasCmd)
	use(1);
      
      listWmono=true;
      hasCmd=true;
    }else if(string(argv[1])=="-l"){
      if(hasCmd)
	use(1);
      
      hasCmd=true;
      listconf=true;
    }else if(string(argv[1])=="-t"){
      if(hasCmd)
	use(1);
      
      hasCmd=true;
      listconftab=true;
    }else if(string(argv[i])=="-c"){
      if(hasCmd)
	use(1);
      
      hasCmd=true;
      check=true;
      i++;

      if(i<argn)
	wmonoFile=argv[i];
      
      if(wmonoFile.empty() || wmonoFile[0]=='-'){
	cerr << "Missing file!" << endl<<endl;
	use(1);
      }
    }else{
      cerr << "\n\tUnknown option: " <<argv[1] << endl;
      use(1);
    }
  }

  if(file.empty()){
    if(getenv("KVALOBS")){
      file=getenv("KVALOBS");
      file=file+"/etc/kvsynopd.conf";
    }else{
      cerr << "\n\tEnvironment variable KVALOBS must be set!\n\n";
      use(1);
    }
  }
      
  cerr << "Reading file: " << file << endl;

  if(!readConfFile(file, stationList)){
    cerr << "FATAL: error in SYNOP sections in the configuration file!";
    return 1;
  }

  if(listWmono){
    for(std::list<StationInfoPtr>::iterator it=stationList.begin();
	it!=stationList.end(); it++){
      cout << (*it)->wmono() << " ";
    }
    cout << endl;
  }else if(listconftab){
    cout <<setw(6) << left << "WMONO" << "|" 
	 << setw(15) << left << "typepriority" << "|"  
	 << setw(8) << "mhTypes" << "|"
	 << setw(16) << left << "precipitation" << "|"  
	 << setw(16) << "delay" << "|"
	 << setw(5) << "list" <<   "|"
	 << setw(5) << "owner" << endl;
    cout << setfill('-') << setw(77) << "-" << setfill(' ') << endl;
    for(std::list<StationInfoPtr>::iterator it=stationList.begin();
	it!=stationList.end(); it++){
      cout << setw(6)  << left << (*it)->wmono() << "|"
	   << setw(15) << left << (*it)->keyToString("typepriority") << "|"
	   << setw(8)  << left << (*it)->keyToString("mustHaveTypes") << "|"
	   << setw(16) << left << (*it)->keyToString("precipitation") << "|"
	   << setw(16) << left << (*it)->keyToString("delay") << "|"
	   << setw(5)  << left << (*it)->keyToString("list") << "|"
	   << setw(5)  << left << (*it)->keyToString("owner") 
	   << endl;
    }
    cout << endl;
  }else if(listconf){
    for(std::list<StationInfoPtr>::iterator it=stationList.begin();
	it!=stationList.end(); it++){
      cout << **it << endl;
    }
  }else if(check){
    list<long> wmoList;
    list<long> kvWmoList;

    readWmonoFromFile(wmonoFile, wmoList);

    for(list<StationInfoPtr>::iterator kvit=stationList.begin();
	kvit!=stationList.end(); kvit++){
      kvWmoList.push_back((*kvit)->wmono());
    }
    
    list<long>::iterator pos;
    list<long>::iterator it;

    cout << "WMO numbers in " << endl
	 << "file: " << wmonoFile << endl
	 << "MISSING in " << endl
	 << "file: " << file <<  endl<< endl;

    for(it=wmoList.begin();it!=wmoList.end(); it++){
      pos=find(kvWmoList.begin(), kvWmoList.end(), *it);
      if(pos==kvWmoList.end())
	cout << *it << endl;
    }
    
    cout << "\n\n -----------------------------------------------------\n\n";

    cout << "WMO numbers in " << endl
	 << "file: " << file << endl
	 << "MISSING in " << endl
	 << "file: " << wmonoFile <<  endl<< endl;

    for(it=kvWmoList.begin();it!=kvWmoList.end(); it++){
      pos=find(wmoList.begin(), wmoList.end(), *it);
      if(pos==wmoList.end())
	cout << *it << endl;
    
    }
  }
  
}

bool
readConfFile(const string &file,  std::list<StationInfoPtr> &stationList)
{
  StationInfoParse theSynParser;
  ifstream    fs;
  ConfSection *conf;
  ConfParser  theParser;

  fs.open(file.c_str());

  if(!fs){
    cerr << "Cant open file <" << file << ">!" << endl;
    use(1);
  }

  conf=theParser.parse(fs);

  if(!conf){
    cerr << "Error while parsing file <" << file << ">!" << endl;
    cerr << theParser.getError() << endl;
    use(1);
  }

  cerr << "No syntax error!\n";

  return theSynParser.parse(conf, stationList);
}

void 
use(int retval)
{
  cerr << endl << endl 
       <<"\tkvsynopdparse [-f confile]  [-l|-w|-t] filename" << endl
       <<"\t\t-f confile Use confile instead of the default file" << endl
       <<"\t\t           $KVALOBS/etc/kvsynopd.conf" << endl
       <<"\t\t-w list all wmonumbers that is configured!" << endl
       <<"\t\t-t list the configuration for all station (tabulated)!" << endl
       <<"\t\t-l list the configuration for all station!" << endl
       << endl;

  
  exit(retval);
}


void 
readWmonoFromFile(const string &file, list<long> &wmoList)
{
  ifstream    fs;

  fs.open(file.c_str());

  if(!fs){
    cerr << "Cant open file <" << file << ">!" << endl;
    use(1);
  }
  
  while(fs){
    string val;
    getline(fs, val);
    
    if(fs){
      wmoList.push_back(atol(val.c_str()));
    }
  }
  
}
  
