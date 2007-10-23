/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: InitLogger.cc,v 1.1.6.1 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <string>
#include <milog/milog.h>
#include <unistd.h>
#include "InitLogger.h"
#include <iostream>

using namespace milog;
using namespace std;

namespace{
    LogLevel getLogLevel(const char *c);
}

void
InitLogger(int argn, char **argv, const std::string &logname)
{
    char         *name;
    string       filename;
    LogLevel     traceLevel=milog::NOTSET;
    LogLevel     logLevel=milog::NOTSET;
    FLogStream   *fs;
    StdErrStream *trace;
    

    name=getenv("KVALOBS");
    
    if(name){
	filename=name;
    }else{
	char buf[PATH_MAX+1];
	
	if(getcwd(buf, PATH_MAX+1)){
	    buf[PATH_MAX]='\0';
	    filename=buf;
	}
    }
    
    if(filename.empty()){
	std::cerr << "FATAL: Can't initialize the Logging system.\n";
	exit(1);
    }
    
    if(filename[filename.length()-1]=='/'){
	filename+="var/log/" + logname + ".log";
    }else{
	filename+="/var/log/" + logname +".log";
    }
    
    
    for(int i=0; i<argn; i++){
	if(strcmp("--tracelevel", argv[i])==0){
	    i++;
	    
	    if(i<argn){
	      traceLevel=getLogLevel(argv[i+1]);
	    }
	}else if(strcmp("--loglevel", argv[i])==0){
	    i++;
	    
	    if(i<argn){
		logLevel=getLogLevel(argv[i+1]);
	    }
	}
    }
    
    try{
	fs=new FLogStream(4);
	
	if(!fs->open(filename)){
	    std::cerr << "FATAL: Can't initialize the Logging system.\n";
	    std::cerr << "------ Cant open the Logfile <" << filename << ">\n";
	    delete fs;
	    exit(1);
	}
	
	trace=new StdErrStream();
    
	if(!LogManager::createLogger(logname, trace)){
	    std::cerr << "FATAL: Can't initialize the Logging system.\n";
	    std::cerr << "------ Cant create logger\n";
	    exit(1);
	}
	
	if(!LogManager::addStream(logname, fs)){
	    std::cerr << "FATAL: Can't initialize the Logging system.\n";
	    std::cerr << "------ Cant add filelogging to the Logging system\n";
	    exit(1);
	}
	
	trace->loglevel(traceLevel);
	fs->loglevel(logLevel);
	
	LogManager::setDefaultLogger(logname);
    }
    catch(...){
	std::cerr << "FATAL: Can't initialize the Logging system.\n";
	std::cerr << "------ OUT OF MEMMORY!!!\n";
	exit(1);
    }

    std::cerr << "Logging to file <" << filename << ">!\n";
    
}

namespace{
    LogLevel getLogLevel(const char *str)
    {
	if(strcmp("FATAL", str)==0){
	    return milog::FATAL;
	}else if(strcmp("ERROR", str)==0){
	    return milog::ERROR;
	}else if(strcmp("WARN", str)==0){
	    return milog::WARN;
	}else if(strcmp("DEBUG", str)==0){
	    return milog::DEBUG;
	}else if(strcmp("INFO", str)==0){
	    return milog::INFO;
	}else if(strcmp("0", str)==0){
	    return milog::FATAL;
	}else if(strcmp("1", str)==0){
	    return milog::ERROR;
	}else if(strcmp("2", str)==0){
	    return milog::WARN;
	}else if(strcmp("3", str)==0){
	    return milog::INFO;
	}else if(strcmp("4", str)==0){
	    return milog::DEBUG;
	}else{
	    return milog::NOTSET;
	}
    }
}
    
	       
