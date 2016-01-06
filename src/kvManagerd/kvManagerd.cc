/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvManagerd.cc,v 1.2.2.4 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <string>
#include <boost/thread/thread.hpp>
#include "lib/dnmithread/CommandQue.h"
#include "lib/miconfparser/miconfparser.h"
#include "lib/milog/milog.h"
#include "lib/fileutil/pidfileutil.h"
#include "kvalobs/kvPath.h"
#include "kvManagerd/AdminImpl.h"
#include "kvManagerd/checkedInputImpl.h"
#include "kvManagerd/CheckForMissingObsMessages.h"
#include "kvManagerd/kvCheckedDataThread.h"
#include "kvManagerd/InitLogger.h"
#include "kvManagerd/managerInputImpl.h"
#include "kvManagerd/mgrApp.h"
#include "kvManagerd/NewDataCommand.h"
#include "kvManagerd/NewDataJob.h"
#include "kvManagerd/PreProcessMissingData.h"
#include "kvManagerd/PreProcessWorker.h"
#include "kvManagerd/SelectDataToProcess.h"
#include "kvManagerd/SendDataToQa.h"
#include "kvManagerd/ServiceCheckedInputImpl.h"

using namespace std;
using namespace boost;

string getDbDriver(miutil::conf::ConfSection *conf) {
  string dbDriver;
  if (conf) {
    miutil::conf::ValElementList dbVal = conf->getValue("database.dbdriver");
    if (dbVal.size() == 1)
      dbDriver = dbVal[0].valAsString();
  }
  // Use postgreSql as a last guess.
  if (dbDriver.empty())
    dbDriver = "pgdriver.so";
  return dbDriver;
}

bool checkForMissingObs(miutil::conf::ConfSection *conf) {
  bool ret = true;
  if (conf) {
    miutil::conf::ValElementList checkVal = conf->getValue("kvManagerd.check_for_missing_obs");
    if (checkVal.size() == 1) {
      string v = checkVal[0].valAsString();
      if (!v.empty() && (v[0] == 'f' || v[0] == 'F'))
        ret = false;
    }
  }
  return ret;
}

int main(int argc, char** argv) {
  CORBA::ORB_ptr orb;
  PortableServer::POA_ptr poa;
  dnmi::thread::CommandQue selectDataQue;  // From kvDatainputd and checkForMissingObsMessages
  dnmi::thread::CommandQue preProcessQue;  // SelectDataToProcess
  dnmi::thread::CommandQue newDataQue;  // From Preprocess
  dnmi::thread::CommandQue qaDataQue;  // From kvQabased
  bool error;
  string pidfile;

  string connectionInfo(KvApp::createConnectString());
  miutil::conf::ConfSection * conf = KvApp::getConfiguration();
  string dbDriver(getDbDriver(conf));
  InitLogger(argc, argv, "kvManagerd", conf);
  bool docheckForMissingObs = checkForMissingObs(conf);
  LOGINFO("check_for_missing_obs=" << (docheckForMissingObs?"true":"false"));
  pidfile = KvApp::createPidFileName("kvManagerd");
  LOGDEBUG("pidfile: " << pidfile);
  if (dnmi::file::isRunningPidFile(pidfile, error)) {
    if (error) {
      LOGFATAL("An error occurred while reading the pidfile:" << endl
               << pidfile << " remove the file if it exist and" << endl
               << "kvManagerd is not running. " << "If it is running and there is problems. Kill kvManagerd and" << endl
               << "restart it." << endl
               << endl);
      return 1;
    } else {
      LOGFATAL("Is kvManagerd already running?" << endl
               << "If not remove the pidfile: " << pidfile);
      return 1;
    }
  }

  LOGINFO("KvManagerd: starting ....");
  ManagerApp app(argc, argv, dbDriver, connectionInfo);
  if (!app.isOk()) {
    return 1;
  }
  app.checkForMissingObs(docheckForMissingObs);
  orb = app.getOrb();
  poa = app.getPoa();

  SelectDataToProcess selectDataToProcess(app, selectDataQue, preProcessQue, newDataQue);

  PreProcessWorker preProcessWorker(app, preProcessQue, newDataQue);

  //Add job to look up new data that has arrived when we was down.
  //preProcessWorker.addOneTimeJob(new NewDataJob);

  //Add jobs to the preProcessing unit.
  preProcessWorker.addJob(new PreProcessMissingData);

  SendDataToQa newData(app, newDataQue);
  KvCheckedDataThread checkedData(app, qaDataQue);
  CheckForMissingObsMessages checkForMissingObs(app, selectDataQue);

  thread selectDataToProcessThread(selectDataToProcess);
  thread checkForMissingObsThread(checkForMissingObs);
  thread preProcessThread(preProcessWorker);  //Receives data from kvDataInputd
  thread newDataQueThread(newData);  //Receives data from preProcessThread
  thread checkedDataQueThread(checkedData);  //Receives data from kvQaBased

  try {
    ManagerInputImpl *mgrImpl = new ManagerInputImpl(app, selectDataQue);
    CheckedInputImpl *chkImpl = new CheckedInputImpl(app, qaDataQue);
    AdminImpl *admImpl = new AdminImpl(app);
    ServiceCheckedInputImpl *srvImpl = new ServiceCheckedInputImpl(app, qaDataQue);

    PortableServer::ObjectId_var mgrImplIid = poa->activate_object(mgrImpl);
    PortableServer::ObjectId_var chkImplIid = poa->activate_object(chkImpl);
    PortableServer::ObjectId_var admImplIid = poa->activate_object(admImpl);
    PortableServer::ObjectId_var srvImplIid = poa->activate_object(srvImpl);

    {
      // IDL interface: CKvalObs::manager::ManagerInput
      CORBA::Object_var ref = mgrImpl->_this();

      if (!app.putRefInNS(ref, "kvManagerInput")) {
        LOGFATAL("FATAL: can't register with CORBA nameserver!\n");
        return 1;
      }

      CORBA::String_var sior(orb->object_to_string(ref));
      cout << "IDL object kvManagerInput IOR = '" << static_cast<char*>(sior) << "'" << endl;
      // IDL interface: micutil::Admin
      ref = admImpl->_this();

      if (!app.putRefInNS(ref, "kvManagerAdmin")) {
        LOGFATAL("FATAL: can't register with CORBA nameserver!\n");
        return 1;
      }

      sior = orb->object_to_string(ref);
      cout << "IDL object micutil::Admin IOR = '" << static_cast<char*>(sior) << "'" << endl;

      // IDL interface: CKvalObs::manager::ManagerInput
      app.setCheckedInput(chkImpl->_this());
      app.setServiceCheckedInput(srvImpl->_this());

    }

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = app.getPoaMgr();
    pman->activate();

    app.createPidFile("kvManagerd");
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
    LOGFATAL("Caught omniORB::fatalException:" << endl << "  file: " << fe.file() << endl << "  line: " << fe.line() << endl << "  mesg: " << fe.errmsg());
    app.deletePidFile();
    exit(1);
  } catch (...) {
    LOGFATAL("Caught unknown exception.");
    app.deletePidFile();
    exit(1);
  }

  selectDataToProcessThread.join();
  preProcessThread.join();
  newDataQueThread.join();
  checkedDataQueThread.join();
  checkForMissingObsThread.join();

  CERR("kvManagerd: exit ....\n");
  app.deletePidFile();
}

