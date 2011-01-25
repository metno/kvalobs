/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: App.cc,v 1.12.6.7 2007/09/27 09:02:37 paule Exp $                                                       

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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h> 
#include <string.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <milog/milog.h>
#include "App.h"
#include <fstream>
#include <miconfparser/miconfparser.h>
#include <sstream>
#include <kvalobs/kvPath.h>

using namespace std;
using namespace CKvalObs::CDataSource;
using namespace milog;
using namespace boost;
using miutil::conf::ConfSection;
using miutil::conf::ValElementList;
using miutil::conf::CIValElementList;

namespace{
  volatile sig_atomic_t sigTerm=0;
  void sig_term(int);
  void setSigHandlers();
  void usage();
}





App::App(int argn, 
	 char **argv,
	 const char *options[0][2])
    :KvApp(argn, argv, options), refData(Data::_nil()),test_(false),
    debug_(false)
{
    string::size_type i;
    string::size_type i2;
    string            arg;
    string            key;
    string            val;
    char              *buf;
    string            kvservers;
    string            loglevel;
    string            tracelevel;
    ConfSection       *myConf=App::getConfiguration();
    ValElementList    valelem;

    if(!myConf){
      LOGFATAL("Cant read the configuration file <" << kvPath("sysconfdir") + "/norcom2kv.conf>");
      exit(1);
    }

    for(int k=1; k<argn; k++){
	arg=argv[k];
	i=arg.find_first_of("=");

	if(i!=string::npos){
	    key=arg.substr(0, i);
	    val=arg.substr(i+1);
	}else{
	    key=arg;
	    val.erase();
	}
	
	//LOGDEBUG("Key=" << key << "  val=" << val << endl);

	if(key=="--tracelevel"){
	  tracelevel=val;
	}else if(key=="--loglevel"){
	  loglevel=val;
	}else if(key=="--test"){
	  test_=true;
	}else{
	  LOGWARN("Unknown option: " << key << 
		  (!val.empty()?string("="+val):"") 
		  << endl); 
	}
    }

 	if(myConf){
      	valelem=myConf->getValue("debug");
      	if(valelem.size()>0){
      		string tmp;
			tmp=valelem[0].valAsString();
			
			if(!tmp.empty() && (tmp[0]=='t' || tmp[0]=='T'))
				debug_=true;
			else
				debug_=false;
      	}
 	}
    


    data2kvdir_ = kvPath("localstatedir","norcom2kv")+"/data2kv/";
    tmpdir_     = kvPath("localstatedir", "norcom2kv")+"/tmp/";
    logdir_     = kvPath("logdir", "norcom2kv")+"/";

    data2kvdir_=checkdir(data2kvdir_, true);
    tmpdir_=checkdir(tmpdir_, true);
    logdir_=checkdir(logdir_, true);
    

    if(myConf){
      valelem=myConf->getValue("synopdir");
      if(valelem.size()==1)
	synopdir_=valelem[0].valAsString();
      
    
      if(synopdir_.empty())
	usage();

      synopdir_=checkdir(synopdir_);
    }

    if(myConf){
      valelem=myConf->getValue("loglevel");
      
      if(valelem.size()==1)
	loglevel=valelem[0].valAsString();
      else
	loglevel="INFO";

      valelem=myConf->getValue("tracelevel");
      
      if(valelem.size()==1)
	tracelevel=valelem[0].valAsString();
      else
	tracelevel="DEBUG";
    }
      

    /**
     * COMMENT:
     * The kvalobsservers we shall send observation to is given 
     * in the conf file $KVALOBS/etc/norcom2kv.conf. 
     */
    
    if(myConf){
      valelem=myConf->getValue("corba.kvservers");

      CIValElementList it=valelem.begin();
    
      for(;it!=valelem.end(); it++){
	refDataList.push_back(KvDataSrc(it->valAsString()));
      }
    }

    if(refDataList.empty()){
      LOGFATAL("At least one server to receive data must be specified!" << endl
	       << "Servers is set up in the configuration file" <<endl <<
	       "norcom2kv.conf!");
      exit(1);
    }
	       
    kvservers.erase();

    for(CITKvDataSrcList it=refDataList.begin();
	it!=refDataList.end(); it++){
	kvservers+=" ";
	kvservers+=it->name();
    }
    
    LOGINFO("Pushing data to kvDataInputd in the following paths in the\n" <<
	    "CORBA nameserver:\n" << kvservers);


       
    setSigHandlers();
}


App::~App()
{
}


std::string 
App::checkdir(const std::string &dir_, bool rwaccess)
{
    struct  stat sbuf;
    int     mode= R_OK;
    std::string dir(dir_);
 
    if(rwaccess)
      mode |= W_OK;

    if(stat(dir.c_str(), &sbuf)<0){
      LOGFATAL("Cant stat the file/path <" << dir << ">\n" <<
	       "-- " << strerror(errno) << endl);
      exit(1);
    }
    
    if(!S_ISDIR(sbuf.st_mode)){
	LOGFATAL("The logdir=<" << dir << "> is not a directory!");
	exit(1);
    }
    
    if(access(dir.c_str(), mode)<0){
      LOGFATAL("norcom2kv must have read/write access to the '" <<
	       dir <<"' directory\n");
	exit(1);
    }
    
    if(dir.rbegin()!=dir.rend() && *dir.rbegin()!='/')
	dir.append("/");

    return dir;
}


Result* 
App::sendDataToKvalobs(const std::string &message, 
		       const std::string &obsType,
		       std::string &sendtTo)
{
    ITKvDataSrcList it=refDataList.begin();
    Data_ptr ptrData;
    bool     forceNS=false;
    bool     usedNS;
    Result   *resToReturn=0;
    Result   *res;
    ostringstream ost;
   
	if(message.empty() || obsType.empty()){
		LOGERROR("INTERNAL: Invalid input message or obstype not given!");
		return 0;
    }

    for(;it!=refDataList.end(); it++){
		forceNS=false;

		if(it==refDataList.begin())
	  		ost << "*" << it->name();
		else
	  		ost << ", " <<it->name();

		for(int i=0; i<2; i++){
	    	ptrData=lookUpKvData(forceNS, usedNS, it->name());
	    
	    	if(CORBA::is_nil(ptrData)){
	      		ost << " (FAILED)";
	      		break;
	    	}
	    
	    	try{
				res=ptrData->newData(message.c_str(), obsType.c_str());
		
				if(it==refDataList.begin()){
		  			resToReturn=res;
				}else{
		  			delete res;
				}

				ost << " (OK)";
				break;  //out of this for loop
	    	}
	    	catch(...){
				if(usedNS){
		  			ost << " (FAILED)";
		  			break;
				}
				forceNS=true;
	    	}
		}
    }
    
    sendtTo=ost.str();
    
    return resToReturn;
}



CKvalObs::CDataSource::Data_ptr 
App::lookUpKvData(bool forceNS, 
		  bool &usedNS,
		  const std::string &kvpath_)

{
  CORBA::Object_var obj;
  Data_ptr ptr;

  ITKvDataSrcList it=refDataList.begin();

  for(;it!=refDataList.end(); it++){
    if(it->name()==kvpath_)
      break;
  }

  if(it==refDataList.end()){
    return Data::_nil();
  } 
  

  usedNS=false;

  while(true){
    if(forceNS){
      usedNS=true;
      
      obj=getRefInNS("kvinput", it->name());
      
      if(CORBA::is_nil(obj))
	return Data::_nil();
      
      ptr=Data::_narrow(obj);
      
      if(CORBA::is_nil(ptr))
	return Data::_nil();
      
      it->ref(ptr);
      
      return ptr;
    }

    if(CORBA::is_nil(obj))
      forceNS=true;
    else
      return it->ref();
  }
}

bool
App::saveFInfoList(const std::string &name, const FInfoList &infoList)
{
  ofstream fost(name.c_str(), ios::out);

  if(!fost){
    LOGERROR("Cant save infoList to file: " << endl << name);
    return false;
  }
  
  
  CIFInfoList it=infoList.begin();
  
  if(it==infoList.end()){
    LOGINFO("There is now information in the infoList to save!");
    fost.close();
    return true;
  }
  
  LOGINFO("Save Info for file(s). In file: " << name);  
  
  
  for(;it!=infoList.end(); it++){
    fost << it->first << ":" << it->second.offset() << ":" 
	 << it->second.crc() << endl;
  }

  fost.close();
  
  return true;
  
}

bool
App::readFInfoList(const std::string &name, FInfoList &infoList)
{
  regex reg("(.*):(.*):(.*)");
  smatch what;
  ifstream fist(name.c_str());
  string   line;

  milog::LogContext cntxt("Init from file: norcom2kv_finfo.dat !");
  
  if(!fist){
    LOGINFO("No file information file!");
    return true;
  }

  while(getline(fist, line)){
    LOGDEBUG(line);

    if(regex_match(line, what, reg)){
      LOGDEBUG("Matched size:" << what.size());
      
      string fname(what[1].first, what[1].second);
      
      File fd(fname);
      
      if(!fd.ok()){
	LOGINFO("<" << fname << "> No longer exist!");
	continue;
      }
      
      FInfo fi(fd,
	       lexical_cast<long>(string(what[2].first, what[2].second)),
	       lexical_cast<unsigned int>(string(what[3].first, what[3].second))
	       );

      infoList[fi.name()]=fi;
	LOGINFO("Adding <" << fi.name() << "> to the list of known files!");
     
    }else{
      LOGDEBUG("NOT Matched");
    }
      
  }

  return true;
}

void 
App::doShutdown()
{
  sigTerm=1;
}



std::string 
App::getKvalobsServer()const
{
  if(refDataList.empty())
    return string();

  return refDataList.front().name();
}



bool 
App::inShutdown()const
{
    return sigTerm>0;
}


/*
std::string 
App::
relpath(const std::string &path_, bool kvalobs)
{
  string path(path_);
  string::size_type i=path.find(kvdir());

  if(i==string::npos)
    return path;

  if(i!=0)
    return path;

  if(kvalobs)
    path.replace(0, kvdir().length(), "$KVALOBS/");
  else
    path.erase(0, kvdir().length());

  return path;
}
*/
namespace{
  void 
  sig_term(int)
  {
    sigTerm=1;
  }
  
  
  void
  setSigHandlers()
  {
    sigset_t         oldmask;
    struct sigaction act, oldact;
    
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGTERM, &act, &oldact)<0){
      LOGFATAL("Can't install signal handler for SIGTERM\n");
      exit(1);
    }
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGINT, &act, &oldact)<0){
      LOGFATAL("Can't install signal handler for SIGTERM\n");
      exit(1);
    }
  }
  
  void 
  usage()
  {
    LOGFATAL(" \n\nUSE \n\n" <<
	     "   norcom2kv OPTIONS \n" 
	     "     OPTIONS: \n" 
	     "       --test  [Use this to save additonal files to 'tmpdir'.\n"
	     "                This files can be used for regression testing]\n"
	     "       --traclevel[=level]\n"
	     "       --loglevel[=level]\n"
	     "       --help  print this help and exit.\n\n"
	     "       \nlevel may be one of: \n"
	     "        FATAL or 0\n"
	     "        ERROR or 1\n"
	     "        WARN  or 2\n"
	     "        INFO  or 3\n"
	     "        DEBUG or 4\n\n"
	     "        DEBUGN where N is in the set [1,5].\n"
	     "     - Default level is DEBUG, if no level is specified.\n"
	     "\n\n");
    exit(1);
  }
}
