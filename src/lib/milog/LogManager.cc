/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: LogManager.cc,v 1.6.6.3 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <milog/LogManager.h>
#include <milog/LogStream.h>
#include <milog/TraceLayout.h>
#include <milog/StdErrStream.h>
#include <milog/private/LoggerImpl.h>
#include <milog/private/types.h>

using namespace std;

namespace milog{
    
    //Initialize some static variables.
    LogManager *LogManager::logManager=0;
    thread::Mutex LogManager::mutex_;

    LogManager::LogManager()
	:traceDefaultLogger(0), logLevel_(NOTSET), enabled_(true)
    {
    }

    LogManager::~LogManager()
    {
	/*
	 * We delete all LoggerImpl, except the loggerImpl that
	 * is associated with the id '__milog_default_logger__' since 
	 * this logger refers to another logger in the list or to
	 * the 'traceDefaultLogger'.
	 */
	priv::ITLoggerImplList it=loggers_.begin();

	for(;it!=loggers_.end(); it++){
	    if(it->first!="__milog_default_logger__"){
		delete it->second;
	    }
	}
	
	delete traceDefaultLogger;
    }

    priv::LoggerImpl* 
    LogManager::getLogger(const std::string &id)
    {
	priv::ITLoggerImplList it=loggers_.find(id);

	if(it!=loggers_.end()){
	    return it->second;
	}

	return 0;
    }


    LogManager*
    LogManager::instance()
    {
	TraceLayout *layout=0;
	StdErrStream *err=0;
	milog::priv::LoggerImpl *theLogger;

	if(!logManager){
	  milog::thread::ScopedLock l(milog::LogManager::mutex_);
	  
	  if(!logManager){
	    try{
	      logManager=new LogManager();
	      layout=new TraceLayout();
	      err=new StdErrStream(layout);
	      layout=0;
	      theLogger=new priv::LoggerImpl(err);
	      err=0;
	      logManager->loggers_["__milog_default_logger__"]=theLogger;
	      logManager->traceDefaultLogger=theLogger;
	    }
	    catch(...){
	      delete logManager;
	      delete layout;
	      delete err;
	      delete theLogger;
	      
	      return 0;
	    }
	  }
	}
	
	return logManager;
    }

    bool 
    LogManager::addStream(LogStream *strm)
    {
	return addStream("__milog_default_logger__", strm);
    }
    
    bool 
    LogManager::addStream(const std::string &id, LogStream *strm)
    {
	LogManager *mgr=instance();
		
		
	if(!mgr)
	    return false;

	thread::ScopedLock lock(LogManager::mutex_);

	priv::ITLoggerImplList it=mgr->loggers_.find(id);

	if(it==mgr->loggers_.end())
	    return false;
	
	it->second->addLogStream(strm);

	return true;
    }

    bool 
    LogManager::setDefaultLogger(const std::string &id)
    {
	LogManager *mgr=instance();
	
	if(!mgr)
	    return false;

	priv::ITLoggerImplList it=mgr->loggers_.find(id);

	if(it==mgr->loggers_.end())
	    return false;
	
	thread::ScopedLock lock(LogManager::mutex_);
	mgr->loggers_["__milog_default_logger__"]=it->second;
	return true;
	      
    }
    

    bool 
    LogManager::resetDefaultLogger()
    {
	LogManager *mgr=instance();
	
	if(!mgr)
	    return false;
	
	thread::ScopedLock lock(LogManager::mutex_);
	
	mgr->loggers_["__milog_default_logger__"]=mgr->traceDefaultLogger;
	
	return true;
    }
    
    bool 
    LogManager::createLogger(const std::string &id, LogStream *strm)
    {
	LogManager *mgr=instance();
	priv::LoggerImpl *log;

	if(!mgr)
	    return false;

      	thread::ScopedLock lock(LogManager::mutex_);

	priv::ITLoggerImplList it=mgr->loggers_.find(id);
	
	if(it!=mgr->loggers_.end())
	    return false;

	try{
	    log=new priv::LoggerImpl(strm);
	}
	catch(...){
	    return false;
	}


	mgr->loggers_[id]=log;
	
	return true;
	
    }

    bool 
    LogManager::enabled()
    {
	LogManager *mgr=instance();
	
	if(!mgr)
	    return false;

	thread::ScopedLock lock(mgr->mutex_);

	return mgr->enabled_;

    }

    void 
    LogManager::enabled(bool en)
    {
	LogManager *mgr=instance();
	
	if(!mgr)
	    return;;

	thread::ScopedLock lock(mgr->mutex_);

	mgr->enabled_=en;
    }

    LogLevel 
    LogManager::loglevel()
    {
	LogManager *mgr=instance();
	
	if(!mgr)
	    return NOTSET;

	thread::ScopedLock lock(mgr->mutex_);
	return mgr->logLevel_;
    }
    
    void     
    LogManager::loglevel(LogLevel ll)
    {
	LogManager *mgr=instance();
	
	if(!mgr)
	    return;

	thread::ScopedLock lock(mgr->mutex_);
	mgr->logLevel_=ll;
    }

    
}
