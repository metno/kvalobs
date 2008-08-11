/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvapp.cc,v 1.22.2.4 2007/09/27 09:02:31 paule Exp $                                                       

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
#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <miconfparser/miconfparser.h>
#include <milog/milog.h>
#include <dnmithread/mtcout.h>
#include <kvalobs/kvapp.h>
#include <kvalobs/kvPath.h>
#include <fileutil/pidfileutil.h>

using namespace std;
using namespace miutil::conf;

namespace{
  ConfSection* confLoader();
}

ConfSection* KvApp::conf=0;
std::string KvApp::confFile;

KvApp::KvApp(int argn, char **argv, const char *opt[0][2])
  :CorbaHelper::CorbaApp(argn, argv, opt)
{
    string corbaNS;
    string kvconfig;
    ValElementList val;
    
    kvPathInCorbaNS.erase();

    for(int i=0; i<argn; i++){
		if(strcmp(argv[i], "--kv-cnspath")==0){
	    	if((i+1)<argn){
				i++;
				kvPathInCorbaNS=argv[i];
	    	}
		}else if(strcmp(argv[i], "--kv-cnserver")==0){
	    	if((i+1)<argn){
				i++;
				corbaNS=argv[i];
	    	}
		}else if(strcmp(argv[i], "--help")==0){
			printUseMsgAndExit(0);
		}else if(strcmp(argv[i], "--kv-config")==0){
			cout << "--kv-config\n";
	    	if((i+1)<argn){
				i++;
				cout << "--kv-config [" << argv[i] << "]\n";
				setConfFile(argv[i]);
	    	}
		}
		
    }
    
    
    //Sets the variable conf
    getConfiguration();
    

    if(kvPathInCorbaNS.empty()){
       if(conf){
          val=conf->getValue("corba.path");
	
          if(val.size()>0){
             kvPathInCorbaNS=val[0].valAsString();
          }
       }
    }

    if(kvPathInCorbaNS.empty()){
       LOGFATAL("Path in CORBA nameserver missing. " << endl <<
                "Set the corba.path value in " << kvPath("sysconfdir") +"/kvalobs.conf." 
                <<  endl );
       printUseMsgAndExit( 1 );
    }

    if(!kvPathInCorbaNS.empty() && 
       kvPathInCorbaNS[kvPathInCorbaNS.length()-1]!='/')
      kvPathInCorbaNS+="/";
    
    LOGINFO("Using <" <<kvPathInCorbaNS << "> as path in CORBA nameserver"); 
    
    
    if(corbaNS.empty()){
       if(conf){
          val=conf->getValue("corba.nameserver");
	
          if(val.size()==1){
             corbaNS=val[0].valAsString();
          }
       }
    }
    
  
    if(corbaNS.empty()) {
      LOGFATAL("No CORBA nameserver given!" << endl << 
              "Set the corba.nameserver value in" <<  kvPath("sysconfdir")+"/kvalobs.conf " 
              << endl );
      printUseMsgAndExit( 1 ); 
    }          
    
    LOGINFO("Using CORBA nameserver at: " << corbaNS);
    
    setNameservice(corbaNS);
}

KvApp::~KvApp()
{
  if(conf){
    delete conf;
    //cerr << "~KvApp:: after delete\n"; 
  }
  //cerr << "~KvApp:: after delete!!!\n"; 
}

//Inherited from CorbaApp.
bool 
KvApp::isOk()const
{
    if( ! CorbaHelper::CorbaApp::isOk() )
       return false;
    
    return true;
}

bool 
KvApp::putRefInNS(CORBA::Object_ptr objref, 
		  const std::string &name_)
{
    std::string name(kvPathInCorbaNS);
    name+=name_;
    
    return putObjInNS(objref, name);
}



CORBA::Object_ptr 
KvApp::getRefInNS(const std::string &name_)
{
    std::string name(kvPathInCorbaNS);
    name+=name_;
    
    return getObjFromNS(name);
}


CORBA::Object_ptr 
KvApp::getRefInNS(const std::string &name_, const std::string &path_)
{
  
    std::string name(path_);

    if(name.empty())
      return CORBA::Object::_nil();

    if(*name.rbegin()!='/')
      name.append("/");

    name.append(name_);
    
    return getObjFromNS(name);
}


void 
KvApp::useMessage(std::ostream &os)
{
    os << "Use: \n\n";
}

void    
KvApp::printUseMsgAndExit(int exitStatus)
{
    cout << "Standard kvalobs configuration options:\n";
    cout << "\t\t--kv-cnspath <path>    The path in CORBA nameserver.\n";
    cout << "\t\t--kv-cnserver <host>   Use the CORBA nameserver at <host>.\n";
    cout << "\t\t--kv-config <filename> Read the configuration file <filename>\n"
         << "\t\t                       instead of the kvalobs.conf\n";
    cout << "\t\t--help Print this help message and exit!\n\n";

    exit(exitStatus);
}


std::string 
KvApp::
createPidFileName(const std::string &progname)
{
	return dnmi::file::createPidFileName( progname );
}


void 
KvApp::createPidFile(const std::string &progname)
{
  FILE *fd;
  
  pidfile = createPidFileName( progname );
  
  LOGINFO("Writing pid to file <" << pidfile << ">!");

  fd=fopen(pidfile.c_str(), "w");

  if(!fd){
      LOGWARN("Can't create pidfile <" << pidfile << ">!\n");
      pidfile.erase();
      return;
  }

  fprintf(fd, "%ld\n", (long)getpid());
  fclose(fd);
}

void 
KvApp::deletePidFile()
{
  if(pidfile.empty())
    return;

  LOGINFO("Deleting pidfile <" << pidfile << ">!");

  unlink(pidfile.c_str());
}


std::string 
KvApp::createConnectString(const std::string &dbname,
			   const std::string &kvdbuser,
			   const std::string &host,
			   const std::string &port)
{
  
  char         *buf;
  stringstream ost;
  ConfSection  *myConf=KvApp::getConfiguration();

  if(myConf){
    ValElementList   val=myConf->getValue("database.dbconnect");

    if(val.size()==1){
      LOGINFO("Using 'database.dbconnect' from configuration file");
      return val[0].valAsString();
    }
  }

  ost << "dbname=";
  
  if(dbname.empty()){
    buf=getenv("KVDB");
    
    if(buf){
      ost << buf;
    }else{
      ost << "kvalobs ";
    }
  }else
    ost << dbname;
   
  ost << " ";

  if(host.empty()){
    buf=getenv("PGHOST");
    
    if(buf){
      ost << "host=" << buf << " ";
    }
  }else
    ost << "host=" << host << " ";

  if(port.empty()){
    buf=getenv("PGPORT");
    
    if(buf){
      ost << "port=" << buf << " ";
    }
  }else
    ost << "port=" << port << " ";

  if(kvdbuser.empty()){
    buf=getenv("KVDBUSER");
    
    if(buf){
      ost << "user=" << buf << " ";
    }else
      ost << "user=kvalobs ";
  }else
    ost << "user=" << kvdbuser << " ";

  return ost.str();
}


miutil::conf::ConfSection* 
KvApp::getConfiguration()
{
  if(!KvApp::conf)
    KvApp::conf=confLoader();

  return conf;
}

std::string 
KvApp::getConfFile(const std::string &ifNotSetReturn)
{
  if(confFile.empty())
    return ifNotSetReturn;

  return confFile;
}

void        
KvApp::setConfFile(const std::string &filename)
{
  confFile=filename;
}


namespace{
  ConfSection* 
  confLoader()
  {
    ConfParser parser;
    string     conffile;
    ConfSection *conf;
    ifstream   fis;
 
    conffile=kvPath("sysconfdir")+"/" + KvApp::getConfFile();

    fis.open(conffile.c_str());

    if(!fis){
      LOGERROR("Cant open the configuration file <" << conffile << ">!" << endl);
    }else{
      LOGINFO("Reading configuration from file <" << conffile << ">!" << endl);
      conf=parser.parse(fis);
      
      if(!conf){
	LOGERROR("Error while reading configuration file: <" << conffile 
		 << ">!" << endl << parser.getError() << endl);
      }else{
	LOGINFO("Configuration file loaded!\n");
	return conf;
      }
    }

    return 0;
  }
}


