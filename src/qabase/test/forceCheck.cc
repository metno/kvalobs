/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: forceCheck.cc,v 1.8.2.4 2007/09/27 09:02:38 paule Exp $                                                       

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
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <dnmithread/mtcout.h>
#include <kvskel/qabase.hh>
#include <puTools/miTime>
#include <corbahelper/corbamacros.h>
#include "SendDataToQaBase.h"
#include "CheckedInputImpl.h"
#include <corbahelper/corbaApp.h>
#include <dnmithread/CommandQue.h>
#include <miconfparser/miconfparser.h>

using namespace std;
using namespace miutil;
using namespace miutil::conf;
using namespace CorbaHelper;

static CORBA::Object_ptr
getObjectFromNameserver(CORBA::ORB_ptr    orb, 
			const std::string &name,
			const std::string &nameserver);

void printHelp(const string app);


miutil::conf::ConfSection* readConf(const std::string &fname);


int main(int argc, char** argv)
{
  dnmi::thread::CommandQue dataQue;  //Data from CheckedInputImpl to 
                                     //SendDataToQaBase
  CKvalObs::CQaBase::QaBaseInput_ptr refQaBase;
  CORBA::ORB_ptr          orb; 
  PortableServer::POA_ptr poa;
  CORBA::Object_var refObject;
  CheckedInputImpl *checkedInput;
  char              *buf;
  bool teststations=false, testtimes=false;
  miString stime, spos;
  vector<miString> stations;
  miString stime_start="UDEFINERT", stime_stop="UDEFINERT";
  string kvserver;
  string corbans;
  string logpath;
  string type;
  ConfSection   *conf=0;
  ValElementList valElem;
  string         val;


  CorbaApp app(argc, argv);

  if(!app.isOk()){
    CERR("\n\n\tCant initialize the CORBA subsystem!!!\n\n");
    return 1;
  }
   
  buf=getenv("KVALOBS");

  if(!buf){
    CERR("\n\n\tMissing environment variable <KVALOBS>!\n\n");
    return 1;
  }

  conf=readConf(string(buf)+"/etc/kvalobs.conf");
  
  if(!conf){
    CERR("\n\n\tCant read the configuration file:\n" <<
	 "/etc/kvalobs.conf\n\n");
    return 1;
  }

  valElem=conf->getValue("corba.path");

  if(valElem.empty()){
    CERR("\n\n\tNo <corba.path> in the configurationfile!\n\n");
    return 1;
  }

  kvserver=valElem[0].valAsString();


  
  if(!kvserver.empty()){
    /* if(kvserver[0]!='/')
      kvserver.insert(0, "/");
    if(*kvserver.rbegin()!='/')
    kvserver.append("/");*/
    kvserver.append("/kvQabaseInput");
  }else{
    CERR("\n\n\tNo kvalobs server is given!!!" << endl <<
	 "\tcheck the <corba.path> variable in the configurationfile:"<< endl<<
	 "\t" << buf << "/etc/kvalobs.conf\n\n");
    return 1;
  }

  valElem=conf->getValue("corba.nameserver");

  if(valElem.empty()){
    CERR("\n\n\tNo <corba.nameserver> in the configurationfile!\n\n");
    return 1;
  }

  corbans=valElem[0].valAsString();

  app.setNameservice(corbans);      
  cout << "USING CORBA name server '" << corbans << "'\n";
  cout << "USING kvalobs server    '" << kvserver << "'\n";
  
  while (1){
    int c= getopt(argc,argv,"s:t:a:o:l:i:");

    if (c == -1) break;

    switch(c){
    case 's':
      spos = optarg;
      if (spos=="TEST") teststations= true;
      break;
    case 't':
      stime= optarg;
      stime_start= stime;
      stime_stop=  stime;
      break;
    case 'a':
      stime= optarg;
      stime_start= stime;
      break;
    case 'o':
      stime= optarg;
      stime_stop= stime;
      break;
    case 'l':
      logpath=optarg;
     	break;
    case 'i':
    	type=optarg;
    	break;
    }
  }
  
  if (!testtimes){
    if (!miTime::isValid(stime_start) || 
	!miTime::isValid(stime_stop)){
      CERR("Illegal obstimes:" << stime_start
	   << " to " << stime_stop << endl);
      printHelp(argv[0]);
      return 1;
    }
  }else{
    stime_start= "2000-12-31 12:0:0";
    stime_stop = "2001-12-31 23:0:0";
  }


  if (!teststations){
    if (spos.length() == 0){
      CERR("No station defined" << endl);
      printHelp(argv[0]);
      return 1;
    }
    if (spos.contains(",")){
      vector<miString> vpos= spos.split(",");
      for (int i=0; i<vpos.size();i++)
	stations.push_back(vpos[i]);

    } else {
      stations.push_back(spos);
    }
  }else{
    stations.push_back("4040");
    stations.push_back("4260");
    stations.push_back("4440");
    stations.push_back("4780");
    stations.push_back("5350");
    stations.push_back("11500");
    stations.push_back("17150");
    stations.push_back("17770");
    stations.push_back("18040");
    stations.push_back("18500");
    stations.push_back("18700");
    stations.push_back("18950");
    stations.push_back("19100");
    stations.push_back("19710");
    stations.push_back("20250");
    stations.push_back("27500");
  }
  
  orb=app.getOrb();
  poa=app.getPoa();
  
  refObject=getObjectFromNameserver(orb, kvserver, corbans);
  refQaBase=CKvalObs::CQaBase::QaBaseInput::_narrow(refObject);
    
  if(CORBA::is_nil(refQaBase)){
    CERR("Can't find <" << kvserver << ">\n");
    orb->destroy();
    return 1;
  }

  try{
    checkedInput=new CheckedInputImpl(dataQue);
  }
  catch(...){
    CERR("NOMEM: out of memmory!!!!! \n\n");
    return 1;
  }
   
  try{
    PortableServer::ObjectId_var mgrImplIid=poa->activate_object(checkedInput);
    PortableServer::POAManager_var pman = app.getPoaMgr();
    pman->activate();
  }
  catch(...){
    COUT("ACTIVATE ERROR: cant activate CORBA object (CheckedInput)!!!\n\n");
    return 1;
  }
 
  SendDataToQaBase sendData(stations, 
  				 type,
			    miTime(stime_start), 
			    miTime(stime_stop),
			    logpath,
			    dataQue, 
			    refQaBase,
			    checkedInput->_this()
			    ); 

  boost::thread sendDataThread(sendData);
  
  try{
    orb->run();
    orb->destroy();
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
         << "object." << endl;
  }
  CORBA_STD_EXCEPTION_COMPLETED
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
  
  sendDataThread.join();

  return 0;
}




static CORBA::Object_ptr
getObjectFromNameserver(CORBA::ORB_ptr    orb, 
			const std::string &name_,
			const std::string &nameserver )
{
  CORBA::Object_ptr obj;
  string name("corbaname::" + nameserver+"#");

  if(name_.length()==0){
    CERR("ERROR: name is empty!\n");
    return CORBA::Object::_nil();
  }


  string::size_type i=name_.find_first_not_of('/');
  
  if(i==string::npos )
    return CORBA::Object::_nil();
    
  name+=name_.substr(i);
  
  CERR("Looking up object in CORBA nameservice: " << name << endl); 

  try{
    obj=orb->string_to_object(name.c_str()); 
  }
  catch(...){
    CERR("Exception: cant find object or nameservice\n");
    return CORBA::Object::_nil();
  }

  return obj;

}

void printHelp(const string app)
{
  CERR("Use:" << app
       << " -s [ <stationid> | <stationid>,<stationid>,.. | TEST ]"
       << " -t <\"isotime\"> "
       << " -a <\"start isotime\"> "
       << " -o <\"stop isotime\"> "
       << " -l logpath"
       << " -i typeid"
       << endl);

  CERR(endl << "-s <stationid>                : one station" << endl
       <<"-s <stationid>,<stationid>,.. : specify list of stations" << endl
       <<"-s TEST                       : stations in TEST-DATA set" << endl
       << endl
       <<"-t <\"isotime\">                : one observation-time (\"YYYY-MM-DD hh:mm:ss\")" << endl
       <<"-a <\"start isotime\">          : specify start observation-time" << endl
       <<"-o <\"stop isotime\">           : specify stop observation-time" << endl 
       <<"-l logpath                      : specify where kvQaBased shall write the html logfiles." << endl
       <<"-i typeid                       : specify the typeid for the message" << endl
       );
       
}




miutil::conf::ConfSection* 
readConf(const std::string &fname)
{
  miutil::conf::ConfParser  parser;
  miutil::conf::ConfSection *conf;
  ifstream    fis;
  
  fis.open(fname.c_str());

  if(!fis){
    CERR("Cant open the configuration file <" << fname << ">!" << endl);
  }else{
      CERR("Reading configuration from file <" << fname << ">!" << endl);
      conf=parser.parse(fis);
      
      if(!conf){
	CERR("Error while reading configuration file: <" << fname 
		 << ">!" << endl << parser.getError() << endl);
      }else{
	CERR("Configuration file loaded!\n");
	return conf;
      }
    }

    return 0;
}
