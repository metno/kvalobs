/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvCorbaThread.cc,v 1.4.6.2 2007/09/27 09:02:46 paule Exp $                                                       

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
#include <dnmithread/mtcout.h>
#include "kvDataNotifySubscriberImpl.h"
#include "kvDataSubscriberImpl.h"
#include "kvHintSubscriberImpl.h"
#include "kvCorbaThread.h"

using namespace std;
using namespace CKvalObs::CService;

namespace kvservice{
  namespace priv{

KvQtCorbaThread::KvQtCorbaThread(int          argn, 
				 char       **argv,
				 const char  *options[0][2])
  :omni_thread(), app(argn, argv, options), isInitialized_(false)
{
  start_undetached();
}

void*
KvQtCorbaThread::run_undetached(void*)
{
  CERR("KvQtCorbaThread: created!\n");
  CORBA::ORB_var orb=app.getOrb();
  PortableServer::POA_ptr poa=app.getPoa();

  try{
    DataNotifySubscriber *subscriber    =new DataNotifySubscriber();
    DataSubscriber       *dataSubscriber=new DataSubscriber();
    KvHintSubscriber     *hintSubscriber=new KvHintSubscriber();

    PortableServer::ObjectId_var subscriberIid 
      = poa->activate_object(subscriber);
    
    PortableServer::ObjectId_var dataSubscriberIid 
      = poa->activate_object(dataSubscriber);
    
    PortableServer::ObjectId_var hintSubscriberIid 
      = poa->activate_object(hintSubscriber);

    {
      kvDataNotifySubscriber_ptr  ref=subscriber->_this();
      kvDataSubscriber_ptr    refData=dataSubscriber->_this();
      kvHintSubscriber_ptr    refHint=hintSubscriber->_this();
      
      app.setDataNotifySubscriber(ref);
      app.setDataSubscriber(refData);
      app.setKvHintSubscriber(refHint);
      

      CORBA::String_var sior(orb->object_to_string(ref));
      COUT("IDL object CKvalObs::CService::kvDataNotifySubscriber IOR = '" << 
	   (char*)sior << "'" << endl);

      sior=orb->object_to_string(refData);
      COUT("IDL object CKvalObs::CService::kvDataSubscriber IOR = '" << 
	   (char*)sior << "'" << endl);

      sior=orb->object_to_string(refHint);
      COUT("IDL object CKvalObs::CService::kvHintSubscriber IOR = '" << 
	   (char*)sior << "'" << endl);
      
    }

  
    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    isInitialized_=true;
    orb->run();
    orb->destroy();
  }
  catch(CORBA::SystemException&) {
    CERR( "Caught CORBA::SystemException." << endl);
  }
  catch(CORBA::Exception&) {
    CERR("Caught CORBA::Exception." << endl);
  }
  catch(omniORB::fatalException& fe) {
    CERR("Caught omniORB::fatalException:" << endl
	 << "  file: " << fe.file() << endl
	 << "  line: " << fe.line() << endl
	 << "  mesg: " << fe.errmsg() << endl);
  }
  catch(...) {
    CERR("Caught unknown exception." << endl);
  }



  CERR("KvQtCorbaThread: return from thread!\n");

  return 0;
}



}
}
