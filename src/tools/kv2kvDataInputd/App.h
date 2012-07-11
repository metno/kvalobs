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


#ifndef __APP_H__
#define __APP_H__

#include <string>
#include <utility>
#include <dnmithread/Thread.h>
#include <kvcpp/corba/CorbaKvApp.h>
#include <kvskel/datasource.hh>
#include "DataReceiver.h"
#include "CorbaServerConf.h"


class App :
public kvservice::corba::CorbaKvApp
{
   typedef dnmi::thread::Thread<DataReceiver> DataReceiverThread;

   std::string dataid;
   std::string notifyid;
   std::string hintid;
   CorbaServerConf receiveFromKvServer;
   CorbaServerConf sendToKvServer;
   dnmi::thread::CommandQue eventQue;
   DataReceiverThread dataReceiverThread;
   CKvalObs::CDataSource::Data_var refDataReceiver;
   CorbaHelper::CorbaApp *corbaApp;
   ParamDefsPtr paramdefs;
   miutil::miTime timeLastTryToSendData;
   bool           dataReceiverAlive;

   bool subscribeSetup();
   bool dataReceiverSetup();

public:
   App( int argn, char **argv, miutil::conf::ConfSection *conf );
   ~App();

   static void createLogger( miutil::conf::ConfSection *conf );

   CORBA::Object_ptr getRefInNS(const CorbaServerConf &serverSpec,
                                const std::string &interface );
   ParamDefsPtr getParamdefs();

   /**
    * Return 0 if it succeed to send the data.
    *        1 No decoder
    *        2 Decode error.
    *        3 Not saved (My retry)
    *        4 General error (Do not retry).
    *        -1 if the data receiving kvalobs server is down.
    *        -2 Failed to look up the server in CORBA nameserver.
    * @param header The decoder header URI
    * @param data The data.
    * @return a status code, see above.
    */
   int sendData( const std::string &header, const std::string &data );

   void run();
};      


#endif 
