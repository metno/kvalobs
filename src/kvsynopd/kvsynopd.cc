/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvsynopd.cc,v 1.12.2.11 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <fstream>
#include <boost/thread/thread.hpp>
#include <milog/milog.h>
#include <fileutil/pidfileutil.h>
#include <kvalobs/kvapp.h>
#include "SynopWorker.h"
#include "DataReceiver.h"
#include "App.h"
#include "Replay.h"
#include "delaycontrol.h"
#include "InitLogger.h"
#include <kvalobs/kvPath.h>

//using namespace kvservice;
using namespace std;
using namespace miutil;
using namespace miutil::conf;

int
main(int argn, char **argv)
{
  bool error;
  std::string pidfile;
  std::string confFile;

#ifdef SMHI_LOG
  milog::LogLevel traceLevel_;
  milog::LogLevel logLevel_;

  InitLogger(argn, argv, "kvsynopd", &traceLevel_, &logLevel_);     
#else
  InitLogger(argn, argv, "kvsynopd");
#endif

  confFile = kvPath("sysconfdir")+"/kvsynopd.conf";
  pidfile = dnmi::file::createPidFileName( kvPath("localstatedir") + "/run",
		                                     "kvsynopd" );

  dnmi::file::PidFileHelper pidFile;
  miutil::conf::ConfSection *conf;
  
  try{
      conf=miutil::conf::ConfParser::parse(confFile);
  }
  catch( const logic_error &ex ){
     LOGFATAL( ex.what() );
     return 1;
  }
  
  App  app(argn, argv, confFile, conf );
  dnmi::thread::CommandQue newDataQue;  
  dnmi::thread::CommandQue newObsQue;  
  dnmi::thread::CommandQue replayQue;  
  DataReceiver        dataReceiver(app, newDataQue, newObsQue);
  SynopWorker         synopWorker(app, newObsQue, replayQue);
  Replay              replay(app, replayQue);
  DelayControl        delayControl(app, newDataQue);
  miTime              startTime;


  if(dnmi::file::isRunningPidFile(pidfile, error)){
    if(error){
      LOGFATAL("An error occured while reading the pidfile:" << endl
	       << pidfile << " remove the file if it exist and"
	       << endl << "kvsynopd is not running. " << 
	       "If it is running and there is problems. Kill kvsynopd and"
	       << endl << "restart it." << endl << endl);
      return 1;
    }else{
      LOGFATAL("Is kvsynopd allready running?" << endl
	       << "If not remove the pidfile: " << pidfile);
      return 1;
    }
  }

#ifdef USE_KVDATA
  //COMMENT: YE: 2009-10-22
  // If present,start_time gives a hard starting point in time,
  // else, use app.checkpoint()
  ValElementList valElem; 
  
  valElem=conf->getValue("start_time");

  if(!valElem.empty()){
    string t=valElem[0].valAsString();
    
    if(!t.empty())
      startTime=miTime(t);
#else
  //COMMENT:
  //For debugging. At the momment a time spec
  //on the form "YYYY-MM-DD hh:mm:ss" as the first
  //argument on the command line is taken to mean that
  //we shall get data from the server from this data 
  //until now.
  
  if(argn>1){

    int n=atoi(argv[1]);
    if(n<0){
      if(n<-24)
	n=-24;

      startTime=miTime::nowTime();
      startTime.addHour(n);
    }else{
      startTime.setTime(argv[1]);
    }
#endif
  }else{
    startTime=app.checkpoint();
    
    if(!startTime.undef()){
      IDLOGINFO("main", "checkpoint at: " << startTime);
    }
  }
      
  
  if( ! app.initKvSynopInterface(  newObsQue ) ){
    LOGFATAL("Cant initialize the interface to <kvsynopd>.");
    return 1;
  }
  
  std::string id=app.subscribeData(kvservice::KvDataSubscribeInfoHelper(), newDataQue);
  
  if(id.empty()){
    LOGFATAL("Cant subscribe on <kvData>.");
    return 1;
  }


  //Write the subscriber id to the file $KVALOBS/var/kvsynop/datasubscriber.id
  ofstream subidfile;

  subidfile.open( string(kvPath("localstatedir")+"/kvsynop/datasubscriber.id").c_str());

  if(subidfile.is_open()){
    subidfile << id << endl;
    subidfile.close();
  }

  pidFile.createPidFile(pidfile);

  boost::thread synopWorkerThread(synopWorker);
  IDLOGDEBUG("main","Started <SynopWorkerThread>!");

  int hoursback = 48;
#ifdef USE_KVDATA
  valElem=conf->getValue("days_back_in_time");

  if(!valElem.empty()){
    int t=valElem[0].valAsInt();
    
    hoursback = t * 24;
  }
#endif
  if(!startTime.undef()){
    miTime now(miTime::nowTime());
    
    now.addHour(-hoursback);
    
    if(startTime<now)
      startTime=now;
    
  }else{
    startTime=miTime::nowTime();
    startTime.addHour(-hoursback);
  }
  
  IDLOGINFO("main","Getting data from kvalobs from time: " << startTime);
  app.getDataFrom(startTime, -1, 0, newObsQue);
  IDLOGDEBUG("main","Return from app.getDataFrom!");


  boost::thread dataReceiverThread(dataReceiver);
  IDLOGDEBUG("main","Started <dataReceiverThread>!");

  boost::thread replayThread(replay);
  IDLOGDEBUG("main","Started <replayThread>!");


  boost::thread delayThread(delayControl);
  IDLOGDEBUG("main","Started <delayControlThread>!");

  app.run();

  dataReceiverThread.join();
  IDLOGDEBUG("main","Joined <dataReceiverThread>!");

  synopWorkerThread.join();
  IDLOGDEBUG("main","Joined <synopWorkerThread>!");

  replayThread.join();
  IDLOGDEBUG("main","Joined <replayThread>!");

  delayThread.join();
  IDLOGDEBUG("main","Joined <delayControlThread>!");

  app.doShutdown();

  return 0;
}


