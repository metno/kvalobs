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
#include <dnmithread/Thread.h>
#include <kvcpp/corba/CorbaKvApp.h>
#include "DataReceiver.h"


class App :
   public kvservice::corba::CorbaKvApp
{
   typedef dnmi::thread::Thread<DataReceiver> DataReceiverThread;
   
   std::string dataid;
   std::string notifyid;
   std::string hintid;
   std::string m_fileFromTime;
   std::string m_fileObsDataList;
   dnmi::thread::CommandQue eventQue;
   DataReceiverThread dataReceiverThread;
   
   bool subscribeSetup();
   bool dataReceiverSetup();

   public:
      App( int argn, char **argv, miutil::conf::ConfSection *conf );
      ~App();
      
      static void printObsDataList( kvservice::KvObsDataList& dataList );
      static void storeToFile( const std::string& filename, const boost::posix_time::ptime& toTime );
      boost::posix_time::ptime lastFromTime( const std::string& filename );
      
      void run();
};      


#endif 
