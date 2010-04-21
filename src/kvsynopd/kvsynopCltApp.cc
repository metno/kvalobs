/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvsynopCltApp.cc,v 1.6.2.3 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <errno.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <dnmithread/mtcout.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <iostream>
#include <kvskel/kvsynopd.hh>
#include <list>
#include <stdio.h>
#include "kvsynopCltApp.h"
#include "kvsynopCorbaThread.h"
#include "kvsynopCltSynopcbImp.h"

using namespace std;
using namespace CorbaHelper;

namespace{
  volatile sig_atomic_t sigTerm=0;
  void sig_term(int);
  void setSigHandlers();

  omni_mutex mutex;
}


SynopCltApp *SynopCltApp::app =0;

SynopCltApp::SynopCltApp(int argn, char **argv, miutil::conf::ConfSection *conf )
  :corbaThread(0),synopcb(kvsynopd::synopcb::_nil()), shutdown_(false),capp(0),
   synop(kvsynopd::synop::_nil())
{
  SynopcbImpl *synopImpl;
  struct timespec spin;

  spin.tv_sec=0;
  spin.tv_nsec=10000000;

  if(!getOptions(argn, argv, conf, opt)){
    cerr << "Inavlid or missing option!";
    use(1);
  }

  if(!app)
    app=this;

  setSigHandlers();
  
  try{
    corbaThread = new SynopCltCorbaThread(argn, argv);
  }
  catch(...){
    CERR("FATAL: failed to initialize KVALOBS service interface!!");
    exit(2);
  }
  
  while(!corbaThread->isInitialized())
    nanosleep(&spin, 0);

  capp=CorbaApp::getCorbaApp();
 
  try{
    synopImpl=new SynopcbImpl(*this);
  }
  catch(...){
    CERR("FATAL: NOMEM, failling to initialize the application!");
    exit(2);
  }

  try{
    PortableServer::ObjectId_var id; 
    id=CorbaApp::getCorbaApp()->getPoa()->activate_object(synopImpl);
    synopcb=synopImpl->_this();
  }
  catch(...){
    CERR("FATAL: Failling to initialize the application! (CORBA)");
    exit(2);
  }

  capp->setNameservice(opt.nshost);

  string name=opt.kvserver;
  
  if(name.empty()){
    CERR("FATAL: No kvserver given!");
    exit(1);
  }

  if(name[name.length()-1]!='/')
    name+="/";

  name+="kvsynopd";

  CORBA::Object_ptr obj=capp->getObjFromNS(name);

  CERR("INFO: Looking up <" << name << "> in nameserver at <" << 
       capp->getNameservice() << ">!\n");
  
  if(CORBA::is_nil(obj)){
    CERR("FATAL: Failed too look up <" << name << "> in CORBA nameserver!\n");
    doShutdown();
    exit(2);
  }

  synop=kvsynopd::synop::_narrow(obj);
  
  if(CORBA::is_nil(synop)){
    CERR("FATAL: Cant downcast the object from the CORBA nameserver!\n");
    exit(2);
  }

  try{
    synop->ping();
  }
  catch(...){
    CERR("FATAL: kvsynopd is down!\n");
    doShutdown();
    exit(2);
  }
  
}
 
SynopCltApp::~SynopCltApp()
{
  doShutdown();
}


void
SynopCltApp:: 
doShutdown()
{
  omni_mutex_lock lock(mutex);

  CorbaHelper::CorbaApp *capp;
  
  shutdown_=true;

  if(!corbaThread){
    //CERR("The CORBA subsystem is allready shutdown!" <<endl );
    return;
  }

  capp=CorbaHelper::CorbaApp::getCorbaApp();
  
  capp->getOrb()->shutdown(true);
  corbaThread->join(0);
  //delete corbaThread_; //This cause a segmentation fault
  corbaThread=0;
  //CERR("AFTER: join\n");
}

bool 
SynopCltApp::
shutdown()const
{
  omni_mutex_lock lock(mutex);

  return shutdown_ || sigTerm;
}

bool 
SynopCltApp::
wait()const
{
  omni_mutex_lock lock(mutex);

  return wait_;
}

void 
SynopCltApp::
wait(bool w)
{
 omni_mutex_lock lock(mutex);
 
 wait_=w;
}

void
SynopCltApp::
run()
{
  while(!shutdown())
    sleep(1);
}


bool 
SynopCltApp::
uptime(miutil::miTime &startTime, long &uptime_)
{
  CORBA::String_var startTime_;
  CORBA::Long       tmpTime;

  try{
    if(!synop->uptime(startTime_, tmpTime))
      return false;
  }
  catch(...){
    return false;
  }
   
  cerr << "tmpTime: " << tmpTime << endl; 

  startTime=miutil::miTime(startTime_);
  uptime_=static_cast<long>(tmpTime);
  return true;
}

bool
SynopCltApp::
stationsList(kvsynopd::StationInfoList &infoList)
{
  kvsynopd::StationInfoList_var list;

  try{
    if(!synop->stations(list))
      return false;
  }
  catch(...){
    return false;
  }

  infoList=list;
  return true;
}

bool
SynopCltApp::
delayList(kvsynopd::DelayList &delayList, miutil::miTime &nowTime)
{
  kvsynopd::DelayList_var list;
  CORBA::String_var       t;
  
  try{
    if(!synop->delays(t, list))
      return false;
    
    nowTime=miutil::miTime(t);
  }
  catch(...){
    return false;
  }

  delayList=list;
  return true;
}

kvsynopd::ReloadList*
SynopCltApp::
cacheReloadList(std::string &msg)
{
  CORBA::String_var        msg_;
  kvsynopd::ReloadList_var ll;

  try{
    if(!synop->reloadCache("", ll, msg_)){
      msg=msg_;
      return 0;
    }

    msg=msg_;
  }
  catch(...){
    COUT("EXCEPTION: cacheReloadList!"); 
    return false;
  }
    
  return ll._retn();
}

bool
SynopCltApp:: 
createSynop(int wmono, 
	    const miutil::miTime &obstime,
	    const TKeyVal &keyvals,
	    int timeoutInSeconds,
	    kvsynopd::SynopData &res)
{
  time_t t;
  time_t tt;
  struct timespec spin;
  micutil::KeyValList keyVals;
  CITKeyVal it=keyvals.begin();

  spin.tv_sec=0;
  spin.tv_nsec=10000000;
  
  wait(false);

  keyVals.length(keyvals.size());
  
  for(CORBA::Long i=0; it!=keyvals.end(); it++, i++){
    keyVals[i].key=CORBA::string_dup(it->first.c_str());
    keyVals[i].val=CORBA::string_dup(it->second.c_str());
  }

  try{
    if(!synop->createSynop(wmono, obstime.isoTime().c_str(),
			   keyVals, synopcb)){
      CERR("Cant create SYNOP for station <" << wmono << ">!");
      return false;
    }
  }
  catch(...){
    CERR("Cant connect to <kvsynopd>!");
    return false;
  }

  wait(true);

  time(&t);
  tt=t;

  while((tt-t)<timeoutInSeconds){
    if(wait() && !shutdown()){
      nanosleep(&spin, 0);
      time(&tt);
    }else
      break;
  }

  if(wait())
    return false;

  res=result;
  
  return true;
}



bool 
SynopCltApp::reloadConf()
{
  CORBA::String_var msg;

  try{
    if(!synop->reloadConf(msg)){
      CERR(msg << endl);
      return false;
    }
  }
  catch(...){
    CERR("Cant connect to kvsynopd!" << endl);
    return false;
  }
   
  CERR(msg<<endl); 
  return true; 
}

bool
SynopCltApp::getOptions(int argn, char **argv, miutil::conf::ConfSection *conf, Options &opt)
{
	struct option long_options[]={{"list-stations", 0, 0, 0},
			{"uptime", 0, 0, 0},
			{"help", 0, 0, 0},
			{"synop", 0, 0, 0},
			{"delay-list", 0, 0, 0},
			{"reload", 0, 0, 0},
			{"cachereload", 0, 0, 0},
			{0,0,0,0}};
	int c;
	int index;
	bool hasTime=false;
	std::string sWmo;

	miutil::conf::ValElementList valElem;

	valElem=conf->getValue("corba.nameserver");

	if(valElem.empty()){
		CERR("No nameserver <corba.nameserver> in the configurationfile!");
		exit(1);
	}

	opt.nshost=valElem[0].valAsString();

	if(opt.nshost.empty()){
	  CERR("The key <corba.nameserver> in the configurationfile has no value!");
	  exit(1);
	}
  
	valElem=conf->getValue("corba.kvpath");

	if(valElem.empty())
		valElem=conf->getValue("corba.path");
	
	if( valElem.empty() ) {
		CERR("Either the <corba.kvpath> or <corba.path> has a value in the configuration file.");
		exit(1);
	}

	opt.kvserver = valElem[0].valAsString();

	if( opt.kvserver.empty() ) {
		CERR("Either the <corba.kvpath> or <corba.path> has a value in the configuration file.");
		exit(1);
	}
  
	while(true){
		c=getopt_long(argn, argv, "s:n:t:", long_options, &index);

		if(c==-1)
			break;

		switch(c){
		case 's':
			opt.kvserver=optarg;
			break;
		case 'n':
			opt.nshost=optarg;
			break;
		case 't':
			int y, m, d, h;
			if(sscanf(optarg, "%d-%d-%d %d", &y, &m, &d, &h)!=4){
				CERR("Invalid timespec: " << optarg<< endl);
				use(1);
			}
      
			opt.time=miutil::miTime(y, m, d, h);
			hasTime=true;
			break;
		case 0:
			if(strcmp(long_options[index].name,"list-stations")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::StationList;
				CERR("list-stations!" << endl);
			}else if(strcmp(long_options[index].name,"uptime")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Uptime;
				CERR( "uptime!" << endl );
			}else if(strcmp(long_options[index].name,"help")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Help;
				use(0);
			}else if(strcmp(long_options[index].name,"synop")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Synop;

				CERR("synop!" << endl);
			}else if(strcmp(long_options[index].name,"delay-list")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Delays;
				CERR("delay-list!" << endl );

			}else if(strcmp(long_options[index].name,"reload")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::Reload;
				CERR( "Reload!" << endl );
			}else if(strcmp(long_options[index].name, "cachereload")==0){
				if(opt.cmd!=Options::Undef)
					return false;
	
				opt.cmd=Options::CacheReload;
				CERR( "CacheReload!" << endl );
			}

			break;
		case '?':
			CERR( "Unknown option : <" << (char)optopt << "> unknown!" << endl);
			return false;
			break;
		case ':':
			CERR( optopt << " missing argument!" << endl);
			return false;
			break;
		default:
			CERR("?? option caharcter: <" << (char)optopt << "> unknown!" << endl);
			return false;
		}
	}

	if(optind<argn){
		while(optind<argn){
			sWmo=argv[optind++];
      
			for(std::string::size_type i=0; i<sWmo.length(); i++){
				if(!isdigit(sWmo[i])){
					return false;
				}
			}
      
			opt.wmonoList.push_back(atoi(sWmo.c_str()));
		}
	}

	if(opt.cmd==Options::Undef){
		opt.cmd=Options::Synop;
	}

	if(opt.cmd==Options::Synop){
		if(opt.wmonoList.empty()){
			CERR("No wmono to create SYNOP for!\n");
			return false;
		}
    
		if(!hasTime){
			CERR("No time specified!\n");
			return false;
		}
	}
   
	return true;
}
void
SynopCltApp::saveTo(const miutil::miString &wmono,
	  const miutil::miString &copyto,
       const miutil::miTime &obstime, 
	   const miutil::miString &wmomsg,
       int ccx)
{
  
  ofstream      f;
  struct stat   sbuf; 

  if(stat(copyto.c_str(), &sbuf)<0){
    if(errno==ENOENT || errno==ENOTDIR){
      CERR("copyto: <"<<copyto<<"> invalid path!");
    }else if(errno==EACCES){
      CERR("copyto: <"<<copyto<<"> permission denied!");
    }else{
      CERR("copyto: <"<<copyto<<">, lstat unknown error!");
    }
    
    return;
  }

  if(!S_ISDIR(sbuf.st_mode)){
    CERR("copyto: <"<<copyto<<"> not a directory!");
    return;
  }
  bool isWritten = false;
  // Try to write to file, even if its already there... 
  while (!isWritten)
  {
	  ostringstream ost;
	  if(ccx==0){
		  ost << copyto << "/" <<  wmono << "-"
#ifdef USE_KVDATA
			  << setfill('0') << setw(4) << obstime.year() << setw(2) << obstime.month() << setw(2) << obstime.day() << setw(2) 
#else
			  << setfill('0') << setw(2) << obstime.day() << setw(2)
#endif
			  << obstime.hour()
			  << ".synop";
	  }else{ 
		  ost << copyto << "/" <<  wmono << "-" 
#ifdef USE_KVDATA
			  << setfill('0') << setw(4) << obstime.year() << setw(2) << obstime.month() << setw(2) << obstime.day() << setw(2) 
#else
			  << setfill('0') << setw(2) << obstime.day() << setw(2)
#endif
			  << obstime.hour() << "-" << static_cast<char>('A'+(ccx-1))
			  << ".synop";
	  }
	  f.open(ost.str().c_str(),std::ios::out|std::ios::in);
	  if (f.is_open())
	  {
		  // close the file if it exist...
		  f.close();
	  }
	  else
	  {
		  // the file not exist, open it...
		  f.open(ost.str().c_str(),std::ios::out);

		  if(f.is_open()){
			  COUT("Writing SYNOP to file: " << ost.str()<<std::endl;);
			  f << wmomsg;
			  f.close();
			  isWritten = true;
		  }else{
			  CERR("Cant write SYNOP to file: " << ost.str()<<std::endl;);
		  }
	  }
	  ccx++;
  }
}
      

/**
 * kvsynopclt [options] wmono, wmono, ....
 * options:
 *      -s kvalobsserver, ex kvtest, kvalobs 
 *      -n nameserver, the host name to the nameserver ex localhost, corbans
 *      -t 'YYYYY-MM-DD HH', timespec in iso format, ex 2004-01-25 15
 *      -h print the help screen and exit.
 */
void
use(int exitcode)
{
  cerr << "Use\n\n"
       << "    kvsynopclt [-n host] [-s kvserver] [CMDS]  \n\n"
       << "       -s kvserver : use the kvalobs server 'kvserver'.\n"
       << "          'kvserver' is the name of the kvalobsserver as it is\n"
       << "          known in the CORBA nameserver.\n"
       << "          Default kvserver is 'kvalobs'.\n"
       << "       -n host :    use the CORBA nameserver at host.\n"
       << "          Default host is 'corbans'!\n"
       << "\n"    
       << "    CMDS\n\n"
       << "     --list-stations: list the stations that is configured in\n"
       << "                      the kvsynopd.\n"
       << "     --delay-list:  list the stations in the dely que.\n"
       << "     --uptime: returns when kvsynopd was started.\n"
       << "     --help: print this help screen!\n"
       << "     --synop [OPTIONS] wmono wmono .... wmono\n\n"
       << "       OPTIONS\n\n"
       << "       -t 'YYYY-MM-DD HH': Create the synop for this time.\n\n"
       << "       wmono: wmo number of the station we shall generate a synop\n"
       << "              for. There can be multiple wmono's.\n"
       << "     --reload: Update the station configrations from the\n"
       << "               configuration file."
       << "     --cachereload: list all stations marked for reload."
       << "\n\n";
  exit(exitcode);
}


namespace{
  void 
  sig_term(int)
  {
    sigTerm=1;
  }
  
  
  void
  setSigHandlers()
  {
    struct sigaction act, oldact;
    
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGTERM, &act, &oldact)<0){
      CERR("Can't install signal handler for SIGTERM\n");
      exit(2);
    }
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGINT, &act, &oldact)<0){
      CERR("Can't install signal handler for SIGTERM\n");
      exit(2);
    }
  }
  
}

