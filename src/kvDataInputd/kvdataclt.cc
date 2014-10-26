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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <dnmithread/mtcout.h>
#include <kvskel/datasource.hh>
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
  	string            optDecoder;
  	string            decoder;
  	string            file;
  	string            content;
  	string            kvserver;
  	Result            *res;                 
  	CORBA::Object_var refObject;
  	Data_var          refData;
  	char              ch;
  	bool              verbose=false;
  	string            confile;
  	string            cwDir;
  	bool ok;


	char buf[512];

	if( getcwd( buf, 512 ) ) {
		cwDir = buf;

		if( cwDir[cwDir.length()-1] != '/' )
			cwDir += "/";
	}

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

	KvApp::setConfFile( "kvdataclt.conf");

	opterr=0; //dont print to standard error.

  	while((ch=getopt(argc, argv,"c:d:s:hv"))!=-1){
  		switch(ch){
  		case 'c':
  			confile = optarg;
  			break;
    	case 'd':
    		decoder=optarg;
    		break;
    	case 'h':
    		use();
    		break;
    	case 's':
    		kvserver=optarg;
    		break;
    	case 'v':
    		verbose=true;
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


 	if( ! optDecoder.empty() && verbose ) {
    	cerr << "Missing -d decoder\n\n";
    	cerr << "Assume the first line in the file specifies the decoder!\n\n";
  	}

  	if(optind<argc)
    	file=argv[optind];
  	else{
    	cerr << "Missing filename!" << endl;
    	use();
    	return 1;
  	}

  	if( ! confile.empty() ) {
  		if( confile[0] != '/' )
  			confile = cwDir + confile;
  	} else if( confile.empty() ) {
  		confile = cwDir + ".kvdataclt";
  		struct stat sbuf;

  		if( stat( confile.c_str(), &sbuf ) < 0 )
  			confile.erase();
  	}
  
  	if( confile.empty() )
  		confile = "kvdataclt.conf";

	KvApp::setConfFile( confile );

  	KvApp app(argc, argv);

  	orb=app.getOrb();
  	poa=app.getPoa();


  	if( kvserver.empty() )
  		kvserver=app.kvserver();

  	cout << "Sending data to:  " << kvserver << endl;


  	if(!readFile(file, content, decoder))
    	return 1;

	try {
		refObject=app.getRefInNS("kvinput", kvserver);
    	refData=Data::_narrow(refObject);
    
    	if(CORBA::is_nil(refData)){
    		CERR("Can't find <kvinput>\n");
    		return 1;
    	}
   
    	if( verbose ) {
    		cerr << "Sending observation to kvDataInput!" << endl
                 << "decoder: " << decoder << endl
	 		     << "Observation: " << content << endl << endl;
    	}

    	res=refData->newData(content.c_str(), decoder.c_str());
    
    	ok = false;

    	switch(res->res){
    	case OK:
    		cout << "OK";
    		if( verbose )
    	    	cout << endl << res->message;
    		cout << endl;
    		ok=true;
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
    	cerr << "Caught system exception COMM_FAILURE -- unable to contact <kvDataInputd> at '"
                 << kvserver << "'." << endl;
  	}
  	catch(CORBA::TRANSIENT & ex) {
  	        cerr << "Caught system exception TRANSIENT -- unable to contact <kvDataInputd> at '"
  	             << kvserver << "'." << endl;
  	}
  	catch(CORBA::SystemException& ex) {
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

  	if( ok )
    	return 0;
  	else
    	return 1;
}

bool
readFile(const std::string &file, 
		 std::string &content,
		 std::string &decoder)
{
	ifstream      fist(file.c_str());
	ostringstream ost;
	string        data;

	if(!fist) {
		cerr << "Cant open file: " << file << endl;
		return false;
	}

	decoder.erase();
	content.erase();

 	while( decoder.empty() )
  		getline(fist, decoder);
  
  	while(getline(fist, data))
  		ost << data << endl;

	
   if( !fist.eof() ) {
	   cerr << "Cant read file: " << file << endl;
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
  	cout << "\n\n  kvdataclt [-d decoder] [-h] filename\n\n"
        << "\t-d decoder, ex. synop, autoobs, ....\n"
        << "\t-c conffile use this confile instead of kvdataclt.conf\n"
        << "\t-h print this help screen and exit!\n"
        << "\t-v be verbose.\n"
        << "\t-s kvalobs server to send the data to.\n\n"
        << "  Assume the decoder is given as the first line of filename, if \n"
        << "  the -d option is not given.\n\n"
        << "  If the kvalobs server to send to is not given, the server is taken\n"
        << "  from the variable corba.path in the configuration\n"
        << "  file $KVALOBS/etc/kvalobs.conf\n\n";
       
	if(exit_)
  		exit(1);
}


