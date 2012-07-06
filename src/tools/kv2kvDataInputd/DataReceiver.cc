/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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

#include <map>
#include <set>
#include <iostream>
#include <dnmithread/CommandQue.h>
#include <kvcpp/kvevents.h>
#include "App.h"
#include "DataReceiver.h"


using namespace std;
using namespace miutil;
using namespace std;
using namespace kvservice;


DataReceiver::
DataReceiver( App &app_,
              dnmi::thread::CommandQue &que_ ) 
   : app( app_ ),
     que( que_ )
{
}

DataReceiver::
~DataReceiver()
{
}
      
void 
DataReceiver::
onKvDataEvent( kvservice::KvObsDataListPtr data )
{
   if( !paramdefs || paramdefs->size() == 0 )
      paramdefs = app.getParamdefs();

   if( !paramdefs || paramdefs->size() == 0 ) {
      cerr << "--- [BEGIN] DataEvent received ---\n";
      cerr << "ERROR: no parameters defined." << endl;
      cerr << "--- [END] ------------------------\n";
      return;
   }


   DataHelper dataHelper;
   kvservice::IKvObsDataList it=data->begin();
 
   cerr << "--- [BEGIN] DataEvent received ---\n";
    
   for( it=data->begin();   
        it!=data->end();    
        it++){              
      if(it->dataList().size()>0)
         cout << "stationID: " << it->stationid() 
              << " obstime: " << it->dataList().front().obstime()
              << " parameters: " << it->dataList().size() << endl;
   }


   dataHelper.addData( data );

   string newdata;
   string decoder;
   int nCount=0;

   dataHelper.initNext();

   //cerr << dataHelper << endl;


   while( dataHelper.nextData( newdata, decoder, paramdefs) ) {
      cerr << "**** SEND: ";
//      cerr << "[" << decoder<<"]" << endl;
//      cerr << newdata << endl;

      if(  app.sendData( decoder, newdata )  != 0 ) {
         cerr << "FAILED";
      } else {
         cerr << "OK";
         ++nCount;
      }
      cerr << " ****************" << endl;
   }
   cerr << "--- [END] (" << nCount << ") -------------\n";
}
      
void  
DataReceiver::
onKvHintEvent( bool up )
{
   if(up){
      paramdefs = app.getParamdefs();
      cerr << "KvUpEvent received! Reloaded the parameter defs." << endl;
   }else{
      cerr << "KvDownEvent received!" << endl;
   }
}

void  
DataReceiver::
onKvDataNotifyEvent( kvservice::KvWhatListPtr what )
{
   cerr << "KvDataNotifyEvent received!" << endl;
}
      
      
      
int 
DataReceiver::
run()
{
   dnmi::thread::CommandBase *cmd;
   kvservice::KvEventBase *event;
   
   while( ! app.shutdown() ) {
      cmd = que.get( 2 );
      
      if( ! cmd )
         continue;
         
      event = dynamic_cast<kvservice::KvEventBase*>( cmd );
      
      if( ! event ) {
         cerr << "DataReceiver::run: Unknown event!" << endl;
         continue; 
      }
         
      event->dispatchEvent( *this );
   }
   
   return 0;
}   
