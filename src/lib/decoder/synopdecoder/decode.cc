/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decode.cc,v 1.8.6.6 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include "kvSynopDecoder.h"
#include <string>
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvDbGate.h>
#include <list>

/* Created by DNMI/PU: j.schulze@dnmi.no
   at Fri Dec  7 12:50:03 2001 */

using namespace std;
using namespace dnmi::db;
using namespace kvalobs;

bool 
checktimes(list<kvalobs::kvData> &data);

bool
readStationsFromKvalobs(list<kvalobs::kvStation> &stat);

void
use();

bool
readObservation(std::ifstream &f, std::string &obs, bool emptyLineSeparator);

int main(int argc, char** argv)
{
  kvSynopDecoder           sdec;
  kvalobs::kvStation       tstat;
  kvalobs::kvRejectdecode  reject;
  list<kvalobs::kvStation> stat;
  string                   obs;
  bool                     sepByEmptyLine=false;
  int                      opt;
  ifstream                 in;
  string                   filename;

  opterr=0; //dont print to standard error.

  while((opt=getopt(argc, argv,"sh"))!=-1){
    switch(opt){
    case 's':
      sepByEmptyLine=true;
      break;
    case 'h':
      use();
      break;
    case '?':
      cerr << "Unknown option -" << static_cast<char>(optopt) << endl;
      use();
      break;
    }
  }

  if(optind<argc)
    filename=argv[optind];
  else{
    cerr << "Missing observation file! " << endl;
    cerr << "Using <test.synop>!\n\n";
    use();
  }

  in.open(filename.c_str(), ios_base::in);

  if(!in){
    cout << "Cant open file <" << filename << ">!\n\n";
    return 1;
  }

  cout << "Reading observations from <" << filename << ">!\n";


  if(!readStationsFromKvalobs(stat)){
    return 1;
  }

   
  sdec.initialise(stat);
  
  while(readObservation(in, obs, sepByEmptyLine)){
   
    list<kvalobs::kvData> data;


    if (sdec.decode(obs, data)) {
      cout << "[" << obs << "]" << endl;
      list<kvData>::iterator itr=data.begin();
      for(;itr!=data.end();itr++)
	cout << *itr << endl;
      



       if(sdec.tmpStation(tstat))
	 cout << "New tmpstation:�------\n" << tstat.toSend() << endl;

    }else {
      reject = sdec.rejected();
      cout <<   "Rejected: ------------\n" << reject.toSend() << endl 
	   << "-------------------------" << endl;
      
       cout << "[" << obs << "]" << endl;
    }
  }

   return 0;
};



bool
readObservation(std::ifstream &in, std::string &obs, bool emptyLineSep)
{
  string buf;
  ostringstream ost;
  
  buf.erase();

  while(in){ //Skip empty lines at start
    getline(in, buf);
    
    if(!buf.empty())
      break;
  }
  
  if(!in && buf.empty())
    return false;

  if(!emptyLineSep || !in){
    obs=buf;
    return true;
  }

  do{
    ost << buf << endl;
    getline(in, buf);
    
    if(buf.empty() || !in){
      obs=ost.str();
      return true;
    }
  }while(in);

  return false;
}

bool
readStationsFromKvalobs(list<kvalobs::kvStation> &stat)
{
  char *buf= getenv("KVALOBS");

  if(!buf){
    cerr << "KVALOBS: environment not defined ...  " << endl;
    return false;
  }

  string kvalobs(buf); 
  string driver = kvalobs + "/lib/db/pgdriver.so";
  string driveID;
  string constr("host=seca dbname=kvalobs user=kvalobs password=kvalobs12 port=5434");

  Connection    *con;
  DriverManager manager;
  
  if(!manager.loadDriver(driver, driveID)){
    cerr << "Can't load driver <" << driver << ">" <<  endl 
	 << manager.getErr() << endl;
    return false;
  }
    
  con=manager.connect(driveID, constr);
  
  if(!con){
    cerr << "Can't create connection to <" << driveID << endl;
    return false;
  }

  cerr << "Connected to <" << driveID << ">" << endl;
  
  
  kvalobs::kvDbGate gate(con);
  
  // Creating station list ...
  
 
  gate.select(stat);

  manager.releaseConnection(con);
  return true;

}


void
use()
{
  cout << "Use\n\tdecode [-s] filename\n\n"
       << "\t-s the observations in the file is separated by\n"
       << "\t   empty lines.\n"
       << "\tfilename, a file with observations!\n\n";
  
  exit(1);

}


bool checktimes(list<kvalobs::kvData> &data)
{
  list<kvalobs::kvData>::iterator it=data.begin();
    
  for(;it!=data.end(); it++){
    if(it->obstime().undef()) 
      cout << "obstime undef() tbtime= "<< it->tbtime() << endl;
    if(it->tbtime().undef())
      cout  << "tbtime undef() obstime= "<< it->obstime() << endl;

	continue;
    }
  return true;
}
