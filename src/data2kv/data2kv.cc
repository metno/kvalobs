#include <milog/milog.h>
#include <fileutil/pidfileutil.h>
#include "App.h"
#include "NewData.h"
#include "CollectData.h"
#include "newfileImpl.h"
#include "AdminImpl.h"

using namespace std;

int main(int argn, char **argv) {
  NewFileImpl *newFileImpl;
  AdminImpl *adminImpl;
  NewData *newDataThread;
  TKvDataReceiverList servers;
  TKvDataReceiverList::iterator its;
  std::string pidfile;

  omniORB::setClientCallTimeout(120000);

  App app(argn, argv);
  CORBA::ORB_ptr orb = app.getOrb();
  PortableServer::POA_ptr poa = app.getPoa();

  servers = app.highpriServers();

  for (its = servers.begin(); its != servers.end(); its++) {
    CollectData *collectDataThread;

    try {
      collectDataThread = new CollectData(app, false,
                                          app.datadir() + its->dirName, *its);
      app.addCollectDataThread(collectDataThread);
    } catch (...) {
      LOGFATAL("NOMEM: Cant initialize the application.");
      cerr << "\n\t(NOMEM): Cant initialize the application.\n\n";
      exit(1);
    }
  }

  servers = app.lowpriServers();

  for (its = servers.begin(); its != servers.end(); its++) {
    CollectData *collectDataThread;

    try {
      collectDataThread = new CollectData(app, true,
                                          app.datadir() + its->dirName, *its);
      app.addCollectDataThread(collectDataThread);
    } catch (...) {
      LOGFATAL("NOMEM: Cant initialize the application.");
      cerr << "\n\t(NOMEM): Cant initialize the application.\n\n";
      exit(1);
    }
  }

  try {
    newDataThread = new NewData(app);
  } catch (...) {
    LOGFATAL("NOMEM: Cant initialize the application.");
    cerr << "\n\t(NOMEM): Cant initialize the application.\n\n";
    exit(1);
  }

  try {
    newFileImpl = new NewFileImpl(app);
    adminImpl = new AdminImpl(app);

    CORBA::Object_var ref = newFileImpl->_this();

    if (!app.putRefInNS(
        ref, "/" + app.aopath() + "/" + app.newfileInterfaceName())) {
      LOGFATAL("NO CORBA NAMESERVER: cant register with the CORBA nameserver.");
      cerr
          << "\n\tNO CORBA NAMESERVER: cant register with the CORBA nameserver.\n\n";
      exit(1);
    }

    std::cerr << "Checkpoint 1\n";

    ref = adminImpl->_this();

    if (!app.putRefInNS(ref, app.kvServer() + "/autoobs2kv")) {
      LOGFATAL("NO CORBA NAMESERVER: cant register with the CORBA nameserver.");
      cerr
          << "\n\tNO CORBA NAMESERVER: cant register with the CORBA nameserver.\n\n";
      exit(1);
    }

    std::cerr << "Checkpoint 2\n";

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = app.getPoaMgr();
    pman->activate();

    pidfile = dnmi::file::createPidFileName(app.piddir(), "data2kv");
    bool error;

    if (dnmi::file::isRunningPidFile(pidfile, error)) {
      if (error) {
        LOGFATAL(
            "An error occured while reading the pidfile:" << endl << pidfile << " remove the file if it exist and" << endl << "data2kv is not running. " << "If it is running and there is problems. Kill data2kv and" << endl << "restart it." << endl << endl);
        return 1;
      } else {
        LOGFATAL(
            "Is data2kv allready running?" << endl << "If not remove the pidfile: " << pidfile);
        return 1;
      }
    }

    dnmi::file::createPidFile(pidfile);
    LOGINFO("autoobs2kv ready!\n");
    orb->run();
    orb->destroy();
  } catch (CORBA::SystemException&) {
    LOGFATAL("Caught CORBA::SystemException.");
    dnmi::file::deletePidFile(pidfile);
    exit(1);
  } catch (CORBA::Exception&) {
    LOGFATAL("Caught CORBA::Exception.");
    dnmi::file::deletePidFile(pidfile);
    exit(1);
  } catch (omniORB::fatalException& fe) {
    LOGFATAL(
        "Caught omniORB::fatalException:" << endl << "  file: " << fe.file() << endl << "  line: " << fe.line() << endl << "  mesg: " << fe.errmsg());
    dnmi::file::deletePidFile(pidfile);
    exit(1);
  } catch (...) {
    LOGFATAL("Caught unknown exception.");
    dnmi::file::deletePidFile(pidfile);
    exit(1);
  }

  newDataThread->join(0);
  app.joinAllCollectDataThread();

  dnmi::file::deletePidFile(pidfile);

  LOGINFO("Terminating....");
  cerr << "Terminating ......\n";

  return 0;
}
