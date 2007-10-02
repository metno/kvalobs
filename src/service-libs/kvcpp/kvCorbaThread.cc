/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvCorbaThread.cc,v 1.3.2.1 2007/09/27 09:02:44 paule Exp $                                                       

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
#include "kvCorbaThread.h"

using namespace std;
using namespace CKvalObs::CService;

namespace kvservice{
  namespace priv{

KvCorbaThread::KvCorbaThread(int          argn, 
			     char       **argv,
			     miutil::conf::ConfSection *conf,
			 const char  *options[0][2])
  :omni_thread(), app(argn, argv, conf, options), isInitialized_(false)
{
  start_undetached();
}

void*
KvCorbaThread::run_undetached(void*)
{
  LOGDEBUG1("KvCorbaThread: created!\n");
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
    LOGERROR( "Caught CORBA::SystemException." << endl);
  }
  catch(CORBA::Exception&) {
    LOGERROR("Caught CORBA::Exception." << endl);
  }
  catch(omniORB::fatalException& fe) {
    LOGFATAL("Caught omniORB::fatalException:" << endl
	     << "  file: " << fe.file() << endl
	     << "  line: " << fe.line() << endl
	     << "  mesg: " << fe.errmsg() << endl);
  }
  catch(...) {
    LOGERROR("Caught unknown exception." << endl);
  }



  LOGDEBUG1("KvCorbaThread: return from thread!\n");

  return 0;
}



}
}
