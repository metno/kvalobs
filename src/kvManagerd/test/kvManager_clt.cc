/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvManager_clt.cc,v 1.5.2.2 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <stdlib.h>
#include <getopt.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <dnmithread/mtcout.h>
#include <kvskel/managerInput.hh>
#include <kvskel/datasource.hh>
#include <puTools/miTime.h>
#include <kvalobs/kvapp.h>


using namespace CKvalObs::CDataSource;
using namespace CKvalObs;
using namespace std;
using namespace miutil;

bool
readFile(const std::string &file, 
	 std::string &content);

void
use();

void 
createDummyData(StationInfoList &st);


int main(int argc, char** argv)
{
  CORBA::ORB_ptr          orb;  
  PortableServer::POA_ptr poa;
  string            decoder;
  string            file;
  string            content;
  Result            *res;                 
  CORBA::Object_var refObject;
  Data_var          refData;
  int               ch;

  KvApp app(argc, argv);

  orb=app.getOrb();
  poa=app.getPoa();

  opterr=0; //dont print to standard error.

  while((ch=getopt(argc, argv,"d:h"))!=-1){
    switch(ch){
    case 'd':
      decoder=optarg;
      break;
    case 'h':
      use();
      break;
    case '?':
      cout << "Unknown option -" << static_cast<char>(optopt) << endl;
      use();
      break;
    case ':':
      cout << "Option -: " << optopt << " missing argument!" << endl;
      use();
      break;
    }
  }

  if(decoder.empty()){
    cerr << "Missing -d decoder\n\n";
    use();
  }

  if(optind<argc)
    file=argv[optind];
  else{
    cout << "Missing filename!" << endl;
    use();
    return 1;
  }
  
  if(!readFile(file, content)){
    cout << "Cant read file <" << file << ">!\n\n";
    return 1;
  }

  try {
    refObject=app.getRefInNS("kvinput");
    refData=Data::_narrow(refObject);
    
    if(CORBA::is_nil(refData)){
      CERR("Can't find <kvinput>\n");
      return 1;
    }
   
    cout << "Sending observation to kvDataInput!" << endl
	 << "decoder: " << decoder << endl
	 << "Observation: " << content << endl << endl;
 
    res=refData->newData(decoder.c_str(), content.c_str());
    
    switch(res->res){
    case OK:
      cout << "OK!\n";
      break;
    case NODECODER:
      cout << "NODECODER: " << res->message << endl;
      break;
    case DECODEERROR:
      cout << "DECODEERROR: " << res->message << endl;
      break;

    case NOTSAVED:
      cout << "NOTSAVED: " << res->message << endl;
      break;
    case ERROR:
      cout << "ERROR: " << res->message << endl;
      break;
    }

    orb->destroy();
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
         << "object." << endl;
  }
  catch(CORBA::SystemException&) {
    cerr << "Caught a CORBA::SystemException." << endl;
  }
  catch(CORBA::Exception&) {
    cerr << "Caught CORBA::Exception." << endl;
  }
  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
  }
  catch(...) {
    cerr << "Caught unknown exception." << endl;
  }

  return 0;
}

void 
createDummyData(StationInfoList &st)
{
  st.length(1);

  st[0].stationId=18700;
//   st.obstime=miTime::nowTime().isoTime().c_str();
  st[0].obstime=miTime(2002,10,18,11,0,0).isoTime().c_str();
  st[0].typeId_=30;
}

bool
readFile(const std::string &file, 
	 std::string &content)
{
    ifstream      fs(file.c_str());
    ostringstream ost;
    char          ch;

    if(!fs){
	CERR("Cant open file <" << file << ">!");
	return false;
    }

    while(fs.get(ch)){
	ost.put(ch);
    }

    if(!fs.eof()){
	CERR("Error while reading file <" << file << ">!");
	fs.close();
	return false;
    }

    fs.close();
    content=ost.str();

    return true;
}




void
use()
{
  cout << "\n\n  kvInputd_clt -d [-h] filename\n\n"
       << "\t-d decoder, ex. synop, autoobs, ....\n"
       << "\t-h print this help screen and exit!\n\n";
  exit(1);
}


