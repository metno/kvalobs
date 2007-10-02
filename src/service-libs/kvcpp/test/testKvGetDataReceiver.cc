/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testKvGetDataReceiver.cc,v 1.1.6.2 2007/09/27 09:02:45 paule Exp $                                                       

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
#include <iostream>
#include <unistd.h>
#include <KvApp.h>


using namespace std;
using namespace kvalobs;
using namespace kvservice;

class MyDataReceiver: public kvservice::KvGetDataReceiver{

public:
  MyDataReceiver(){};

  bool next(KvObsDataList &dataList){
    cout << "MyDataReceiver::next" << endl;

    for(IKvObsDataList it=dataList.begin();
	    it!=dataList.end();
	    it++){
        
        if(it->dataList().begin()!= it->dataList().end())
            cout << it->dataList().front().stationID() << " obstime: " << it->dataList().front().obstime() << "  " 
	             << it->dataList().size() << " parameter(s)" << endl;
    }

    for(IKvObsDataList it=dataList.begin();
        it!=dataList.end();
        it++){
            
        KvObsData::kvTextDataList &textData=it->textDataList();
        
        for(KvObsData::kvTextDataList::iterator it=textData.begin();
            it!=textData.end();
            it++){
                
            cout << it->stationID() << " obstime: " << it->obstime() << "  " 
                    << it->original() << endl;
        }
    }

    return true;
  }
};

int
main(int argn, char **argv)
{
  WhichDataHelper whichData;
  KvObsDataList   dataList;
  miutil::miTime  from(miutil::miTime::nowTime());
  MyDataReceiver  dataReceiver;
  
  KvApp   app(argn, argv, KvApp::readConf(string(getenv("KVALOBS"))+"/etc/kvalobs.conf"));
  
  from.addDay(-1);

  COUT("Getting data for ALL station: " << endl
       << "From date: " << from << "  until now!" << endl);

  //from.setTime( "2004-06-10 00:00:00");
  whichData.addStation(36560, from);
  //whichData.addStation(0, from);

  if(!app.getKvData(dataReceiver, whichData)){
    cerr << "Cant connect to kvalobs!";
    return 1;
  }

  app.run();

  CERR("testKvGetDataReceiver ..... exit\n");

  return 0;
}
