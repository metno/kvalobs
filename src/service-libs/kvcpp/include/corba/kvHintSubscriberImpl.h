/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvHintSubscriberImpl.h,v 1.2.2.3 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvHintSubscriberImpl_h__
#define __kvHintSubscriberImpl_h__

#include <kvskel/kvHintSubscriber.hh>
#include <dnmithread/CommandQue.h>

namespace kvservice{
  namespace priv{

class HintSubscriber: 
  public POA_CKvalObs::CService::kvHintSubscriber,
  public PortableServer::RefCountServantBase {
  dnmi::thread::CommandQue &que;
  PortableServer::ObjectId_var id;
public:

  HintSubscriber(dnmi::thread::CommandQue &que);
  virtual ~HintSubscriber();

  void setId(PortableServer::ObjectId *id_){ id=id_;}
  PortableServer::ObjectId getId()const{ return id;}
  
  void kvUp();
  void kvDown();

};
  }
}

#endif


#if 0
// End of example implementational code



int main(int argc, char** argv)
{
  try {
    // Initialise the ORB.
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "omniORB3");

    // Obtain a reference to the root POA.
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    // We allocate the objects on the heap.  Since these are reference
    // counted objects, they will be deleted by the POA when they are no
    // longer needed.
    CKvalObs_CService_kvHintSubscriber_i* myCKvalObs_CService_kvHintSubscriber_i = new CKvalObs_CService_kvHintSubscriber_i();


    // Activate the objects.  This tells the POA that the objects are
    // ready to accept requests.
    PortableServer::ObjectId_var myCKvalObs_CService_kvHintSubscriber_iid = poa->activate_object(myCKvalObs_CService_kvHintSubscriber_i);


    // Obtain a reference to each object and output the stringified
    // IOR to stdout
    {
      // IDL interface: CKvalObs::CService::kvHintSubscriber
      CORBA::Object_var ref = myCKvalObs_CService_kvHintSubscriber_i->_this();
      CORBA::String_var sior(orb->object_to_string(ref));
      cout << "IDL object CKvalObs::CService::kvHintSubscriber IOR = '" << (char*)sior << "'" << endl;
    }



    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
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

#endif
