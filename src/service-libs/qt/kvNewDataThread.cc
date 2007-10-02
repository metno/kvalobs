/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvNewDataThread.cc,v 1.4.2.2 2007/09/27 09:02:47 paule Exp $                                                       

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
#include <kvskel/kvService.hh>
#include "kvQtEvents.h"
#include "kvQtApp.h"
#include "kvQtCorbaApp.h"
#include <qthread.h>
#include "kvNewDataThread.h"
#include "WhichDataHelper.h"

using namespace CKvalObs::CService;
using namespace std;
using namespace miutil;

namespace kvservice{
  namespace priv{

KvNewDataThread::KvNewDataThread(KvQtApp &app, KvWhatListQue &que):
  app_(app), que_(que)
{
}

void
KvNewDataThread::operator()()
{
  IKvWhatList    it;
  CORBA::ULong   i;
  KvDataList     *data;
  KvNewDataEvent *dataEvent;
  bool           cancel=false;
  bool           ret;
  

  std::cerr << "KvNewThread: thread created!\n";

  while(!cancel){
    WhichDataHelper whichData;

    try{
      KvWhatListPtr p=que_.get(0);

      CERR("INFO: KvNewThread: kvWhatListPtr received from que!\n");

      it=p->begin();

      for(; it!=p->end(); it++){
	whichData.addStation(it->stationID(), it->obsTime(), it->obsTime());
      }
      
      data=getData(whichData);
      
      if(data){
	try{
	  dataEvent=new KvNewDataEvent(data);
	  QApplication::postEvent(qApp, dataEvent);
	}
	catch(...){
	  delete data;
	}
      }
    }
    catch(MessageQueCancel &ex){
      std::cerr << "KvNewThread: cancel received!\n";
      cancel=true;
    }
    catch(MessageQueEmpty &ex){
      //Do nothing.
    }
    catch(...){
      std::cerr << "KvNewThread: unexpected exception!\n";
    }
    
  }

  std::cerr << "KvNewThread: return from thread!\n";
}


KvDataList*
KvNewDataThread::getData(const kvservice::WhichDataHelper &whichData)
{
  KvQtCorbaApp *capp;
  KvDataList   *dataList;

  capp=static_cast<KvQtCorbaApp*>(CorbaHelper::CorbaApp::getCorbaApp());

  try{
    dataList=new KvDataList();
  }
  catch(...){
    CERR("FATAL (KvNewDataThread::getData): OUT OF MEMMORY!\n");
    return 0;
  }

  if(!capp->getKvData(*dataList, whichData)){
    delete dataList;
    return 0;
  }

  return dataList;
}

}
}
