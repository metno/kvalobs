/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: DataReceiverApp.cc,v 1.3.2.1 2007/09/27 09:02:41 paule Exp $                                                       

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
#include <fstream>
#include <milog/milog.h>
#include "DataReceiverApp.h"

using namespace std;
using namespace kvservice;
using namespace miutil::conf;

DataReceiverApp::DataReceiverApp(int argn, char **argv,
                                 miutil::conf::ConfSection *conf)
    : KvAppSimple(argn, argv, conf) {
  struct stat sbuf;
  ValElementList valelem;

  if (!conf) {
    cerr << "No configuration file!";
    exit(1);
  }

  valelem = conf->getValue("datadir");

  if (valelem.empty()) {
    cerr << "No value for <datadir> in confile!";
    exit(1);
  }

  datadir = valelem[0].valAsString();

  if (stat(datadir.c_str(), &sbuf) < 0) {
    cerr << "Cant stat the file/path <" << datadir << ">\n" << "-- "
         << strerror(errno) << endl;
    exit(1);
  }

  if (!S_ISDIR(sbuf.st_mode)) {
    LOGFATAL("The datadir=<" << datadir << "> is not a directory!");
    exit(1);
  }

  cerr << "Saving data to directory: " << datadir << endl;

}

void DataReceiverApp::onKvHintEvent(bool up) {
  if (up) {
    cerr << "KvUpEvent received!" << endl;
  } else {
    cerr << "KvDownEvent received!" << endl;
  }
}

void DataReceiverApp::onKvDataNotifyEvent(kvservice::KvWhatListPtr what) {
  cerr << "KvDataNotifyEvent received!" << endl;
}

void DataReceiverApp::onKvDataEvent(kvservice::KvObsDataListPtr data) {
  ostringstream ost;
  ofstream f;
  char c;

  cerr << "KvDataEvent received!" << endl;
  //it er en iterator. Ireratorer brukes for traversere
  //en container, i dette tilfellet KvDataList. KvDataList 
  //er en liste av kvData element.
  CIKvObsDataList it = data->begin();

  if (it == data->end()) {
    cerr << "No Data!!!";
    return;
  }

  ost << datadir << "/" << it->front().stationID() << ".dat";

  f.open(ost.str().c_str(), ios_base::out | ios_base::app);

  if (!f.is_open()) {
    cerr << "Cant open file: " << ost.str() << endl;
    return;
  }

  for (; it != data->end();     //Er vi kommet til slutten
      it++) {               //Flytt it til neste element i listen.

    //cerr skriver til standard error stream.
    //Skriv ut stationid, paramid, corrected og useinfo fra 
    //kvData elementet som it peker på.
    cerr << it->front().stationID() << " " << it->front().obstime().isoTime()
        << " " << it->front().typeID() << endl;
    /*f << it->front().obstime().isoTime() << "," << it->front().typeID() << "," 
     << it->front().tbtime() << endl;*/

    for (CIKvDataList p = it->begin(); p != it->end(); p++) {
      c = p->sensor();
      f << p->stationID() << "|" << p->obstime() << "|" << p->original() << "|"
          << p->paramID() << "|" << p->tbtime() << "|" << p->typeID() << "|"
                                                << c << "|" << p->level() << "|"
                                                << p->corrected() << "|"
                                                << p->controlinfo().flagstring()
                                                << "|"
                                                << p->useinfo().flagstring()
                                                << "|" << p->cfailed() << endl;
    }
  }

}

bool DataReceiverApp::onStartup() {
  dataid = subscribeData(KvDataSubscribeInfoHelper());

  if (dataid.empty()) {
    cerr << "Cant subscribe to KvData!" << endl;
    exit(1);
  } else {
    cerr << "Subscribe on KvData!" << endl;
  }

  hintid = subscribeKvHint();

  if (hintid.empty()) {
    cerr << "Cant subscribe to KvHint!" << endl;
    exit(1);
  } else {
    cerr << "Subscribe on KvHint!" << endl;
  }

  cerr << "Inittializing success!" << endl;
  return true;
}

void DataReceiverApp::onShutdown() {
  cerr << "Terminating .....!" << endl;
}

