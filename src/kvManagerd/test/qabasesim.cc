/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: qabasesim.cc,v 1.2.6.2 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <boost/thread/thread.hpp>
#include <dnmithread/mtcout.h>
#include "QaWorkThread.h"
#include "qabaseApp.h"
#include "qabaseInputImpl.h"

using namespace std;
using namespace boost;

int 
main(int argc, char** argv)
{
  CORBA::ORB_ptr orb;  
  PortableServer::POA_ptr poa;

  QaBaseApp app(argc, argv);

  if(!app.isOk()){
    CERR("FATAL: can't initialize " << argv[0] << "!\n");
    return 1;
  }

  orb=app.getOrb();
  poa=app.getPoa();

  QaWork qaWork(app);
  thread qaWorkThread(qaWork);


  try {
    
    QaBaseInputImpl* qabaseImpl = new QaBaseInputImpl(app);

    PortableServer::ObjectId_var mgrImplIid=poa->activate_object(qabaseImpl);
    {
      // IDL interface: CKvalObs::CQabase::QabaseInput
      CORBA::Object_var ref = qabaseImpl->_this();
      
      if(!app.putRefInNS(ref, "kvQabaseInput")){
	CERR("FATAL: can't register with CORBA nameserver!\n");
	return 1;
      }
      CORBA::String_var sior(orb->object_to_string(ref));
      cout << "IDL object kvQabaseInput IOR = '" << (char*)sior << "'" << endl;
    }


    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = app.getPoaMgr();
    pman->activate();

    orb->run();
    orb->destroy();
  }
  catch(CORBA::SystemException&) {
    cerr << "Caught CORBA::SystemException." << endl;
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

  return 0;
}


