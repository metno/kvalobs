/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvpush.cc,v 1.1.2.2 2007/09/27 09:02:48 paule Exp $                                                       

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
#include <stdlib.h>
#include <iostream>
#include <list>
#include <sstream>
#include <thread>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <decodeutility/kvalobsdata.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvWorkelement.h>
#include <kvalobs/kvPath.h>
#include <kvcpp/KvApp.h>
#include <kvcpp/kvevents.h>
#include "Option.h"

using namespace std;
//using namespace miutil;
using namespace kvalobs;
using namespace dnmi::db;

namespace pt=boost::posix_time;
namespace ks=kvservice;
using std::cerr;
using std::cout;
using std::endl;

void doGetDat(std::shared_ptr<ks::KvApp> app, const Options &opt );
void doSubscribe(std::shared_ptr<ks::KvApp> app, const Options &opt );

int main(int argn, char** argv) {
  pt::ptime  undefTime;
  Connection *con;
  Options opt;

  ParseOpt(argn, argv, &opt);

  std::shared_ptr<ks::KvApp> pKvApp(ks::KvApp::create("kvpull", argn, argv, opt.conf));


  if( ! opt.doSubscribe ) {
    doGetDat(pKvApp, opt );
  } else {
    doSubscribe(pKvApp, opt);
  }

  return 0;
}


class DataReceiver : public ks::KvGetDataReceiver {
  public:
    bool next(ks::KvObsDataList &d) {
      cout << d << endl;
      return true;
    }
};


void doGetDat(std::shared_ptr<ks::KvApp> app, const Options &opt )
{
  ks::WhichDataHelper dh;

  cerr << "Getting data: " << opt.fromtime;

  if ( ! opt.totime.is_special() ) {
    cerr << " - " << opt.totime;
  }

  cerr << endl;
  cerr << "For stations: ";

  if (opt.stations.size()==0 ) {
    cerr << "all" << endl;
    dh.addStation(0, opt.fromtime, opt.totime);
  } else { 
    auto first = true;
    for( auto const &s: opt.stations) {
      if (!first) {
        cerr << ", ";
      }
      first = false;
      cerr << s;
      dh.addStation(s, opt.fromtime, opt.totime);
    }
    cerr << endl;
  }

  ks::KvObsDataList d;
  DataReceiver dr;
  //app->getKvData(d, dh);
  app->getKvData(dr, dh);

  cout << d << endl << endl;
}

void DataReceiverThread(dnmi::thread::CommandQue *que) {
  try {
    while( ! que->isSuspended() ) {
      auto pd = que->get(2);

      if( ! pd ) 
        continue;

      cerr << "Inncomming data ..... " << endl << endl;
      ks::DataEvent *de = static_cast<ks::DataEvent*>(pd);
      cout << *de->data() << endl;
      delete de;
   }
  }
  catch(const dnmi::thread::QueSuspended &ex) {
    cerr << "Exiting DataReceiverThread" << endl;
  }
}


void doSubscribe(std::shared_ptr<ks::KvApp> app, const Options &opt ) {
    ks::KvDataSubscribeInfoHelper dh;
  
  cerr << "Subscribe data stations: ";
  
  if (opt.stations.size()==0 ) {
    cerr << "all" << endl;
    //dh.addStationId(0);
  } else { 
    auto first = true;
    for( auto const &s: opt.stations) {
      if (!first) {
        cerr << ", ";
      }
      first = false;
      cerr << s;
      dh.addStationId(s);
    }
    cerr << endl;
  }

  dnmi::thread::CommandQue *que = new dnmi::thread::CommandQue();

  auto id = app->subscribeData(dh, *que);

  cerr << "Subscribed with id: " << id << endl;

  std::thread t(DataReceiverThread, que);
  while(!app->shutdown()) {
    sleep(1);
  }
  que->suspend();
  t.join();
}