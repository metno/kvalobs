/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CorbaThread.cc,v 1.1.6.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#include "CorbaThread.h"
#include "AdminImpl.h"

using namespace std;

CorbaThread::CorbaThread(App &app_)
  :omni_thread(), app(app_), isInitialized_(false)
{
  start_undetached();
}

void*
CorbaThread::run_undetached(void*)
{
  LOGINFO("CorbaThread: created!\n");
  CORBA::ORB_var orb=app.getOrb();
  PortableServer::POA_ptr poa=app.getPoa();

  try{
    AdminImpl        *admImpl = new AdminImpl(app);

    PortableServer::ObjectId_var admImplIid=poa->activate_object(admImpl);

    CORBA::Object_var ref = admImpl->_this();
    
    if(!app.putRefInNS(ref, "norcom2kv")){
      LOGFATAL("FATAL: can't register with CORBA nameserver!\n");
      return 0;
    }
    
    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    // PortableServer::POAManager_var pman = poa->the_POAManager();
    PortableServer::POAManager_var pman = app.getPoaMgr();
    pman->activate();

    isInitialized_=true;
    orb->run();
    orb->destroy();
  }
  catch(CORBA::SystemException&) {
    LOGFATAL( "Caught CORBA::SystemException." << endl);
  }
  catch(CORBA::Exception&) {
    LOGFATAL("Caught CORBA::Exception." << endl);
  }
  catch(omniORB::fatalException& fe) {
    LOGFATAL("Caught omniORB::fatalException:" << endl
	 << "  file: " << fe.file() << endl
	 << "  line: " << fe.line() << endl
	 << "  mesg: " << fe.errmsg() << endl);
  }
  catch(...) {
    LOGFATAL("Caught unknown exception." << endl);
  }

  LOGFATAL("KvCorbaThread: return from thread!\n");

  return 0;
}

