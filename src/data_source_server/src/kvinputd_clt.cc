/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvinputd_clt.cc,v 1.1.2.6 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <dnmithread/mtcout.h>
#include <kvskel/datasource.hh>
#include <puTools/miTime>
#include <kvalobs/kvapp.h>

using namespace CKvalObs::CDataSource;
using namespace CKvalObs;
using namespace std;
using namespace miutil;

bool
readFile(const std::string &file, 
	 	  	std::string &content,
	 		std::string &decoder);

void 
use(bool exit=true);


int main(int argc, char** argv)
{
  	CORBA::ORB_ptr          orb;  
  	PortableServer::POA_ptr poa;
  	string            decoder;
  	string            file;
  	string            content;
  	string            kvserver;
  	Result            *res;                 
  	CORBA::Object_var refObject;
  	Data_var          refData;
  	char              ch;

	//Check if the argument list contain --help. 
	//If so print our help text, but don't exit. 
	//KvApp will check for this option and print a help text
	//about standard options and exit. 
	for(int i=0; i<argc; i++){
		if(strcmp(argv[i], "--help")==0){
			use(false);
			break;
		}else if(strcmp(argv[i], "-h")==0){
			use(false);
			
			//Replace -h with --help so KvApp prints the standard help screen and exit.
			argv[i]=strdup("--help");  
			break;
		}
	}

  	KvApp app(argc, argv);

  	orb=app.getOrb();
  	poa=app.getPoa();

  	opterr=0; //dont print to standard error.

  	while((ch=getopt(argc, argv,"d:s:h"))!=-1){
    	switch(ch){
    	case 'd':
      	decoder=optarg;
      	break;
    	case 'h':
      	use();
      	break;
    	case 's':
    		kvserver=optarg;
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
  
  	if(kvserver.empty())
  		kvserver=app.kvserver();

	cout << "Sending data to the kvalobs server: " << kvserver << endl;

  	if(decoder.empty()){
    	cerr << "Missing -d decoder\n\n";
    	cerr << "Assume the first line in the file specifies the decoder!\n\n";
  	}

  	if(optind<argc)
    	file=argv[optind];
  	else{
    	cout << "Missing filename!" << endl;
    	use();
    	return 1;
  	}
  
  	if(!readFile(file, content, decoder)){
    	cout << "Cant read file <" << file << ">!\n\n";
    	return 1;
  	}

	try {
 		refObject=app.getRefInNS("kvinput", kvserver);
    	refData=Data::_narrow(refObject);
    
    	if(CORBA::is_nil(refData)){
      	CERR("Can't find <kvinput>\n");
      	return 1;
    	}
   
    	cout << "Sending observation to kvDataInput!" << endl
	 		  << "decoder: " << decoder << endl
	 		  << "Observation: " << content << endl << endl;
 
    	res=refData->newData(content.c_str(), decoder.c_str());
    
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

      cout << res->message;

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

  	if(res->res==OK)
    	return 0;
  	else
    	return 1;
}

bool
readFile(const std::string &file, 
	 std::string &content, std::string &decoder)
{
	ifstream      fist(file.c_str());
   ostringstream ost;
   string        data;

   if(!fist){
   	return false;
   }

 	if(decoder.empty())
  		getline(fist, decoder);
  
  	while(getline(fist, data)){
   	ost << data << endl;
  	}
	
   if(!fist.eof()){
		fist.close();
		return false;
   }

   fist.close();
   content=ost.str();

   return true;
}




void
use(bool exit_)
{
  	cout << "\n\n  kvDataInput_clt [-d decoder] [-h] filename\n\n"
        << "\t-d decoder, ex. synop, autoobs, ....\n"
        << "\t-h print this help screen and exit!\n\n"
        << "\t-s kvalobs server to send the data to.\n\n"
        << "  Assume the decoder is given as the first line of filename, if \n"
        << "  the -d option is not given.\n\n"
        << "  If the kavlobs server to send to is not given, the server is taken\n"
        << "  from the variable corba.path in the configuration\n"
        << "  file $KVALOBS/etc/kvalobs.conf\n\n";
       
	if(exit_)
  		exit(1);
}


