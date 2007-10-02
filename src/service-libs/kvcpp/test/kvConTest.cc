/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvConTest.cc,v 1.1.6.1 2007/09/27 09:02:45 paule Exp $                                                       

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
#include <boost/thread/thread.hpp>
#include <KvApp.h>
#include <milog/milog.h>
#include "DataReceiver.h"

using namespace kvservice;

int
main(int argn, char **argv)
{
  KvApp  app(argn, argv, KvApp::readConf("test.conf"));
  dnmi::thread::CommandQue que;  
  
  DataReceiver dataReceiver(app, que);


  std::string id=app.subscribeData(KvDataSubscribeInfoHelper(), que);
  
  if(id.empty()){
    LOGFATAL("Cant subscribe on <kvData>.");
    return 1;
  }
  
  boost::thread dataReceiverThread(dataReceiver);
  app.run();

  dataReceiverThread.join();
  return 0;
}


