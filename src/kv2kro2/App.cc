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

#include <stdio.h>
#include <fstream>
#include <puTools/miTime>
#include "GetData.h"
#include "App.h"

using namespace std;
using namespace kvservice;

App::
App( int argn, char **argv, miutil::conf::ConfSection *conf )
   : kvservice::corba::CorbaKvApp( argn, argv, conf ),
     dataReceiverThread( 0 )
{
   subscribeSetup();
   dataReceiverSetup();
}

App::
~App()
{
   dataReceiverThread.join( true );
}

bool 
App::
subscribeSetup()
{
   notifyid = subscribeDataNotify( kvservice::KvDataSubscribeInfoHelper(), eventQue );
    
   if( notifyid.empty() ){
      cerr << "Cant subscribe to KvDataNotify!" << endl;
      return false;
   }else{
      cerr << "Subscribe on KvDataNotify!" << endl;
   }

   dataid = subscribeData( KvDataSubscribeInfoHelper(), eventQue );

   if( dataid.empty() ){
      cerr << "Cant subscribe to KvData!" << endl;
      return false;
   }else{
      cerr << "Subscribe on KvData!" << endl;
   }

   hintid = subscribeKvHint( eventQue );

   if( hintid.empty() ){
      cerr << "Cant subscribe to KvHint!" << endl;
      return false;
   }else{
      cerr << "Subscribe on KvHint!" << endl;
   }

   miutil::miTime toTime = miutil::miTime::nowTime();
   miutil::miTime fromTime = lastFromTime( m_fileFromTime );

   cout << "toTime= " << toTime.isoTime() << endl;
   cout << "fromTime= " << fromTime.isoTime() << endl;

   kvservice::WhichDataHelper whichData;
  
   whichData.addStation( 0, fromTime );

   GetData dataReceiver;

   if( ! getKvData( dataReceiver, whichData ) ){
      cerr << "Cant connect to kvalobs!";
      return false;
  }
  
  return true;
}

bool 
App::
dataReceiverSetup()
{
   DataReceiver *dr;
   
   try{
      dr = new DataReceiver( *this, eventQue );
   }
   catch( ... ) {
      return false;
   }
   
   //The dataReceiverThread take over the responsibility
   //for the (dr) pointer, ie delete it when it is no longer
   //needed.
   dataReceiverThread = DataReceiverThread( dr );
   
   //Start the thread and return.
   
   return dataReceiverThread.start();
}
      
      

void 
App::
run()
{
   if( ! subscribeSetup() )
      return;
      
   if( ! dataReceiverSetup() )
      return;
      
   kvservice::corba::CorbaKvApp::run();
}      
    
miutil::miTime 
App::
lastFromTime( const string& filename )
{
   miutil::miTime time;
   miutil::miString str;

   cerr<<"filename= "<<filename<<endl;

   ifstream fin( filename.c_str() );
   if(!fin){
      cerr<<"the file "<<filename<<" does not exist"<<endl;
      time = miutil::miTime::nowTime();
      time.addDay(-1);
      cerr<<"return lastFromTime: no file"<<endl;
      return time;
   }


   //fin.exceptions(ios_base::badbit|ios_base::failbit|ios_base::eofbit);
   fin.exceptions(ios_base::badbit);

   try{
      cerr<<"try lastFromTime"<<endl;

      while(getline(fin,str)){
         str.trim();
         cerr<<"str= "<<str<<endl;
         
         if(!str.empty()){
            if( time.isValid(str) ){
               time.setTime(str);
               
               if(time.undef()){
                  cerr<<"time is undef"<<endl;
                  time = miutil::miTime::nowTime();
                  time.addDay(-1);
               }
            }else{
               cerr<<"time is not valid"<<endl;
               time = miutil::miTime::nowTime();
               time.addDay(-1);
            }
         }
      }
   }catch( std::ios_base::failure ){
      cerr<<"catch( std::ios_base::failure )"<<endl;
      time = miutil::miTime::nowTime();
      time.addDay(-1);
   }catch(...){
      cerr<<"catch(...)"<<endl;
      time = miutil::miTime::nowTime();
      time.addDay(-1);
   }

   cerr<<"return lastFromTime"<<endl;
   return time;
}
    
      
void 
App::
printObsDataList( KvObsDataList& dataList )
{
   string fileFromTime = "fromTime";
   string datadir=".";
   string fileDataList = datadir + "/" + "obsDataList";
   string fileTextDataList= datadir + "/" + "textDataList";
  
   miutil::miTime toTime = miutil::miTime::nowTime();
  
   string filename = fileDataList + "." + toTime.isoDate() + "_" 
                     + toTime.isoClock();
   string pfilename = filename + ".new";

   string textfilename = fileTextDataList + "." + toTime.isoDate() + "_" 
                         + toTime.isoClock();
   string ptextfilename = textfilename + ".new";

   ofstream fout( pfilename.c_str() );
   ofstream tfout;

   bool readText=false;
 
   for( IKvObsDataList dit=dataList.begin();
        dit!=dataList.end();
        dit++){
      KvObsData::kvDataList::iterator it=dit->dataList().begin();
    
      if(dit->dataList().size()>0){
         miutil::miTime obstime= it->obstime();
         miutil::miTime tbtime= it->tbtime();

         if( obstime < toTime )
            toTime = obstime;
      
         if( tbtime  < toTime )
            toTime = tbtime;
      
          
         for( ;
              it != dit->dataList().end();
              it++){
            char c= it->sensor();
            fout << it->stationID() << "|" << it->obstime() << "|" << it->original() 
                 << "|" << it->paramID()<< "|" <<  it->tbtime() << "|" 
                 << it->typeID() << "|" << c << "|" << it->level() << "|" 
                 << it->corrected() << "|" << it->controlinfo().flagstring() 
                 << "|" << it->useinfo().flagstring() << "|" << it->cfailed() 
                 << endl;
         }
      }

      KvObsData::kvTextDataList::iterator i=dit->textDataList().begin();
      
      if(dit->textDataList().size()>0){
         if( ! tfout.is_open() ){
            readText =true;
            tfout.open( ptextfilename.c_str() );
         }
      
         miutil::miTime obstime= i->obstime();
         miutil::miTime tbtime= i->tbtime();

         if( obstime < toTime )
            toTime = obstime;
         
         if( tbtime  < toTime )
            toTime = tbtime;
       
         for( ;
              i !=dit->textDataList().end();
              i ++){
            tfout << i->stationID() << "|" << i->obstime() << "|" << i->original() 
                  << "|" << i->paramID()<< "|" <<  i->tbtime() << "|" 
                  << i->typeID() << endl;
         }
      }
   }

   fout.close();
   tfout.close();
  
   if( rename(  pfilename.c_str(), filename.c_str() )){
      cerr<<"can not rename file "<<pfilename<<endl;
   }

   if( readText && rename(  ptextfilename.c_str(), textfilename.c_str() )){
      cerr<<"can not rename file"<<ptextfilename<<endl;
   }
  
   string sysarg = "./obsInsert " + filename;
   system(sysarg.c_str());

   if( readText ){
      sysarg = "./obsInsert " + textfilename;
      system( sysarg.c_str() );
   }
  
   toTime.addHour(-6);
   storeToFile( fileFromTime, toTime );
   cout<<"**  printObsDataList is finished **"<<endl;
}

void 
App::
storeToFile( const std::string& filename, const miutil::miTime& toTime )
{
   ofstream fout(filename.c_str());
   fout <<toTime.isoTime();
}
