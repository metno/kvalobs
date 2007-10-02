/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvsynopCorbaThread.cc,v 1.2.2.2 2007/09/27 09:02:23 paule Exp $                                                       

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
//#include <kvcpp/kvDataNotifySubscriberImpl.h>
#include "kvsynopCorbaThread.h"

using namespace std;

SynopCltCorbaThread::SynopCltCorbaThread(int          argn, 
					 char       **argv)
  :omni_thread(), app(argn, argv), isInitialized_(false)
{
  start_undetached();
}

void*
SynopCltCorbaThread::run_undetached(void*)
{
  //CERR("CorbaThread: created!\n");
  CORBA::ORB_var orb=app.getOrb();
  PortableServer::POA_ptr poa=app.getPoa();

  try{
    
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



  //CERR("CorbaThread: return from thread!\n");

  return 0;
}

