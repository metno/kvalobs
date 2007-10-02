/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReceiver.cc,v 1.1.6.1 2007/09/27 09:02:45 paule Exp $                                                       

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
#include <milog/milog.h>
#include "DataReceiver.h"

using namespace std;
using namespace kvservice;

DataReceiver::DataReceiver(kvservice::KvApp &app_,
			   dnmi::thread::CommandQue &que_)
  :app(app_), que(que_)
{
}

void 
DataReceiver::
operator()()
{
  dnmi::thread::CommandBase *com;
  DataEvent                 *event;
 

  while(!app.shutdown()){
    com=que.get(1);

    if(!com)
      continue;
    
    event=dynamic_cast<DataEvent*>(com);

    if(!event){
      LOGERROR("Unexpected event!");
      delete com;
      continue;
    }

    newData(event->data());

    delete event;
  }
  
}



void 
DataReceiver::newData(kvservice::KvObsDataListPtr data)
{
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
	cerr << "stationID: " << it->front().stationID() 
	     << " obstime: " << it->front().obstime()
	     << " parameters: " << it->size() << endl;
    }
    
}

