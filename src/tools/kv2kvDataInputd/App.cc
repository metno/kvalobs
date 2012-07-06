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
#include <puTools/miTime.h>
#include "App.h"

using namespace std;
using namespace kvservice;
using namespace CKvalObs::CDataSource;
using namespace CKvalObs;
using namespace miutil;

namespace {
std::string
getConfValue( const char *key,
              miutil::conf::ConfSection *conf,
              bool exitIfNotFound )
{
   string ret;
   miutil::conf::ValElementList valelem;

   valelem = conf->getValue( key );

   if( valelem.size() < 1 ) {
      cerr << "No <" << key << "> in the confile." << endl;

      if( exitIfNotFound )
         exit( 1 );

      return ret;
   }

   ret = valelem[0].valAsString();

   if( ret.empty( ) ) {
      cerr << "No value for key <" << key << "> in the confile." << endl;

      if( exitIfNotFound )
         exit( 1 );
   }

   return ret;
}

CorbaServerConf
getPathConfValue( const char *key,
                  miutil::conf::ConfSection *conf,
                  const string &defaultNameserver )
{
   string val=getConfValue( key, conf, true );

   CorbaServerConf sc;

   if( !sc.decodeConfspec( val, defaultNameserver ) ) {
      cerr << "Invalid value <" << val << ">. Expecting"
            << " a value on the form path[@nameserver[:port]]. "
            << "where [] specify optional parts." << endl;
      exit(1);
   }

   return sc;
}

}


App::
App( int argn, char **argv, miutil::conf::ConfSection *conf )
: kvservice::corba::CorbaKvApp( argn, argv, conf ),
  dataReceiverThread( 0 ),
  corbaApp( kvservice::corba::CorbaKvApp::getCorbaApp() ),
  dataReceiverAlive( false )
{
   string defaultNameserver;

   if( ! corbaApp ) {
      cerr << "Can't initialize CORBA!" << endl;
      exit( 1 );
   }

   defaultNameserver = getConfValue("corba.nameserver", conf, true );
   sendToKvServer = getPathConfValue( "corba.destpath", conf, defaultNameserver);
   receiveFromKvServer = getPathConfValue( "corba.path", conf, defaultNameserver);

   setNameservice( receiveFromKvServer.ns.toString() );

   subscribeSetup();
   dataReceiverSetup();

   clog << "Receiving data from: " << receiveFromKvServer << endl;
   clog << "Sending data to:     " << sendToKvServer << endl;
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

   //Check if we allready has subscribed.
   if( ! dataid.empty() )
      return true;

//   notifyid = subscribeDataNotify( kvservice::KvDataSubscribeInfoHelper(), eventQue );
//
//   if( notifyid.empty() ){
//      cerr << "Cant subscribe to KvDataNotify!" << endl;
//      return false;
//   }else{
//      cerr << "Subscribe on KvDataNotify!" << endl;
//   }

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

CORBA::Object_ptr
App::
getRefInNS(const CorbaServerConf &serverSpec,
           const std::string &interface )
{
   string path( serverSpec.name + "/" + interface );
   return corbaApp->getObjFromNS( path, serverSpec.ns );
}


ParamDefsPtr
App::
getParamdefs()
{
   if(  paramdefs && paramdefs->size() > 0 )
      return paramdefs;

   cerr << "App::getParamdefs() getting paramdefs\n";

   try {
      std::list<kvalobs::kvParam> kvParam;

      getKvParams( kvParam );

      paramdefs.reset( new ParamDefs() );

      for( std::list<kvalobs::kvParam>::iterator it=kvParam.begin();
           it != kvParam.end(); ++it ) {
         (*paramdefs)[it->paramID()] = it->name();
      }

      return paramdefs;
   }
   catch( ... ) {
      return paramdefs;
   }

   //Make the compiler happy.
   return paramdefs;
}

int
App::
sendData( const std::string &decoder, const std::string &data )
{
   int ret=-1;
   bool retry=true;
   Result *res;

   do {
      if( ! dataReceiverAlive && !timeLastDataReceiverAlive.undef() ) {
         miTime now( miTime::nowTime() );
         now.addMin( 5 );

         //If the kvalobs server receiving data is down,
         //wait 5 minutes before retrying to send data to the server.
         if( timeLastDataReceiverAlive < now )
            return -1;
      }

      dataReceiverAlive = false;
      timeLastDataReceiverAlive = miTime::nowTime();

      try {
         if( CORBA::is_nil( refDataReceiver ) ) {
            cerr << "Looking up 'kvinput' on <"<<sendToKvServer <<">." << endl;
            refDataReceiver = Data::_narrow( getRefInNS( sendToKvServer, "kvinput" ) );

            if( CORBA::is_nil( refDataReceiver ) ){
               CERR("Can't find 'kvinput' on <"<< sendToKvServer <<">\n");
               return -2;
            }
            retry = true;
         }

         res=refDataReceiver->newData(data.c_str(), decoder.c_str());
         retry = false;
         dataReceiverAlive = true;
         cerr << "Sending data to <kvinput> on <" <<sendToKvServer <<"> : ";
         switch(res->res){
            case OK:
               cout << "OK";
               cout << endl;
               ret = 0;
               break;
            case NODECODER:
               cout << "NODECODER: " << res->message << endl;
               ret = 1;
               break;
            case DECODEERROR:
               cout << "DECODEERROR: " << res->message << endl;
               ret = 2;
               break;
            case NOTSAVED:
               cout << "NOTSAVED: " << res->message << endl;
               ret = 3;
               break;
            case ERROR:
               cout << "ERROR: " << res->message << endl;
               ret = 4;
               break;
         }
         cerr << endl;
      }
      catch(CORBA::COMM_FAILURE& ex) {
         cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
               << "object." << endl;
         if( retry ) {
            refDataReceiver = Data::_nil();
            retry = false;
         }
      }
      catch(CORBA::SystemException&) {
         cerr << "Caught a CORBA::SystemException." << endl;
      }
      catch(CORBA::Exception&) {
         cerr << "Caught CORBA::Exception." << endl;
      }
      catch(omniORB::fatalException& fe) {
         cerr << "Caught omniORB::fatalException:" << endl;
         cerr << "  file: " << fe.file() << endl;
         cerr << "  line: " << fe.line() << endl;
         cerr << "  mesg: " << fe.errmsg() << endl;
      }
      catch(...) {
         cerr << "Caught unknown exception." << endl;
      }
   }while( retry );

   return ret;
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




