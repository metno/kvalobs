/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReceiverSimple.cc,v 1.1.6.1 2007/09/27 09:02:45 paule Exp $                                                       

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
#include <KvAppSimple.h>

using namespace std;
using namespace kvservice;

class MyApp :  public kvservice::KvAppSimple
{
  string dataid;
  string notifyid;
  string hintid;

public:
  MyApp(int argn, char **argv, miutil::conf::ConfSection *conf)
    :KvAppSimple(argn, argv, conf)
  {
  }

  void onKvHintEvent(bool up){
    if(up){
      cerr << "KvUpEvent received!" << endl;
    }else{
      cerr << "KvDownEvent received!" << endl;
    }
  }

  void onKvDataNotifyEvent(KvWhatListPtr what){
    cerr << "KvDataNotifyEvent received!" << endl;
  }
     
  void onKvDataEvent(KvObsDataListPtr data){
    cerr << "KvDataEvent received!" << endl;

    //it er en iterator. Ireratorer brukes for å traversere 
    //en container, i dette tilfellet KvDataList. KvDataList 
    //er en liste av kvData element.
    CIKvObsDataList it;

    cerr << "newData: data from kvalobs\n";
    cerr << "--------------------------\n";
    
    for(it=data->begin();    //sett it til starten av listen
	it!=data->end();     //Er vi kommet til slutten
	it++){               //Flytt it til neste element i listen.
	
	//cerr skriver til standard error stream.
	//Skriv ut stationid, paramid, corrected og useinfo fra 
	//kvData elementet som it peker på.
	cout << "stationID: " << it->front().stationID() 
	     << " obstime: " << it->front().obstime()
	     << " parameters: " << it->size() << endl;
    }

  }

  bool onStartup(){
    notifyid=subscribeDataNotify(KvDataSubscribeInfoHelper());
    
    if(notifyid.empty()){
      cerr << "Cant subscribe to KvDataNotify!" << endl;
      return false;
    }else{
      cerr << "Subscribe on KvDataNotify!" << endl;
    }

    dataid=subscribeData(KvDataSubscribeInfoHelper());

    if(dataid.empty()){
      cerr << "Cant subscribe to KvData!" << endl;
      return false;
    }else{
      cerr << "Subscribe on KvData!" << endl;
    }

    hintid=subscribeKvHint();

    if(hintid.empty()){
      cerr << "Cant subscribe to KvHint!" << endl;
      return false;
    }else{
      cerr << "Subscribe on KvHint!" << endl;
    }

    cerr << "Inittializing success!" << endl;
    return true;
  }

  void onShutdown(){
    cerr << "Terminating .....!" << endl;
  }
};

int
main(int argn, char **argv){
  MyApp  app(argn, argv, KvAppSimple::readConf("test.conf"));
  app.run();
}
