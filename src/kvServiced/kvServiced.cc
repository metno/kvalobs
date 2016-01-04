/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvServiced.cc,v 1.4.2.7 2007/09/27 09:02:39 paule Exp $                                                       

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
#include "ServiceApp.h"
#include "DataReadyInputImpl.h"
#include "kvServiceImpl.h"
#include "ServiceSubscriber.h"
#include <boost/thread/thread.hpp>
#include <dnmithread/CommandQue.h>
#include <milog/milog.h>
#include "InitLogger.h"
#include <miconfparser/miconfparser.h>
#include "ObjReaper.h"
#include <fileutil/pidfileutil.h>
#include <kvalobs/kvPath.h>

using namespace std;
using namespace boost;

int main(int argc, char** argv) {
  CORBA::ORB_ptr orb;
  PortableServer::POA_ptr poa;
  string sior;
  dnmi::thread::CommandQue dataReadyQue;
  string dbdriver;
  miutil::conf::ConfSection *conf = KvApp::getConfiguration();
  bool error;
  string pidfile;

  //Read all connection information from the $KVALOBS/etc/kvalobs.conf
  //if it exist. Otherwise use the environment varibales:
  //KVDB, KVDBUSER, PGHOST, PGPORT
  string constr(KvApp::createConnectString());

  if (conf) {
    miutil::conf::ValElementList val = conf->getValue("database.dbdriver");

    if (val.size() == 1)
      dbdriver = val[0].valAsString();
  }

  //Use postgresql as a last guess.
  if (dbdriver.empty())
    dbdriver = "pgdriver.so";

  InitLogger(argc, argv, "kvServiced", conf);

  pidfile = KvApp::createPidFileName("kvServiced");

  if (dnmi::file::isRunningPidFile(pidfile, error)) {
    if (error) {
      LOGFATAL(
          "An error occured while reading the pidfile:" << endl << pidfile << " remove the file if it exist and" << endl << "kvServiced is not running. " << "If it is running and there is problems. Kill kvServiced and" << endl << "restart it." << endl << endl);
      return 1;
    } else {
      LOGFATAL(
          "Is kvServiced allready running?" << endl << "If not remove the pidfile: " << pidfile);
      return 1;
    }
  }

  LOGINFO("KvServiced: starting ....");

  ServiceApp app(argc, argv, dbdriver, constr);

  if (!app.isOk()) {
    return 1;
  }

  orb = app.getOrb();
  poa = app.getPoa();

  //set timeout to 5 minutes
  //  omniORB::setClientCallTimeout(300000);

  //set timeout to 1 minute
  omniORB::setClientCallTimeout(60000);

  ServiceSubscriber serviceSubscriber(app, dataReadyQue);
  ObjReaper reaper(app);

  thread serviceSubscriberThread(serviceSubscriber);
  thread objReaperThread(reaper);

  try {
    DataReadyInputImpl *dataReadyImpl = new DataReadyInputImpl(dataReadyQue,
                                                               &app);
    KvServiceImpl *serviceImpl = new KvServiceImpl(app);

    PortableServer::ObjectId_var mgrImplIid = poa->activate_object(
        dataReadyImpl);
    PortableServer::ObjectId_var serviceImplIid = poa->activate_object(
        serviceImpl);
    {
      // IDL interface: CKvalObs::manager::ManagerInput
      CORBA::Object_var ref = dataReadyImpl->_this();

      if (!app.putRefInNS(ref, "kvServiceDataReady")) {
        LOGFATAL("FATAL: can't register with CORBA nameserver!\n");
        return 1;
      }
      sior = orb->object_to_string(ref);
      cout << "IDL object kvServiceDataReady IOR = '" << sior << "'" << endl;

      // IDL interface: CKvalObs::CService::kvService
      ref = serviceImpl->_this();

      if (!app.putRefInNS(ref, "kvService")) {
        LOGFATAL("FATAL: can't register with CORBA nameserver!\n");
        return 1;
      }

      sior = orb->object_to_string(ref);
      cout << "IDL object kvService IOR = '" << sior << "'" << endl;
    }

    app.createPidFile("kvServiced");

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = app.getPoaMgr();
    pman->activate();

    app.notifyAllKvHintSubscribers();

    orb->run();
    orb->destroy();
  } catch (CORBA::SystemException&) {
    LOGFATAL("Caught CORBA::SystemException.");
    app.deletePidFile();
    exit(1);
  } catch (CORBA::Exception&) {
    LOGFATAL("Caught CORBA::Exception.");
    app.deletePidFile();
    exit(1);
  } catch (omniORB::fatalException& fe) {
    LOGFATAL(
        "Caught omniORB::fatalException:" << endl << "  file: " << fe.file() << endl << "  line: " << fe.line() << endl << "  mesg: " << fe.errmsg());
    app.deletePidFile();
    exit(1);
  } catch (...) {
    LOGFATAL("Caught unknown exception.");
    app.deletePidFile();
    exit(1);
  }

  serviceSubscriberThread.join();
  objReaperThread.join();

  CERR("kvServiced: exit ....\n");
  app.deletePidFile();
}

