/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataInputd.cc,v 1.28.2.3 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <signal.h> 
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <milog/milog.h>
#include "InitLogger.h"
#include "DataSrcImpl.h"
#include "DataSrcApp.h"
#include <dnmithread/ThrPoolQue.h>
#include "DecodeCommand.h"
#include <miconfparser/miconfparser.h>
#include <fileutil/pidfileutil.h>
#include <kvalobs/kvPath.h>
using namespace std;
using namespace dnmi::thread;
using namespace boost;

extern volatile sig_atomic_t sigTerm;
static void  sig_term(int);
static void  setSigHandlers();
static bool  killThreadOnSignal();
static bool  threadAfter(CommandBase *cmd);

//Følgende globale variabler brukes av funksjonen
//killThreadOnSignal for å stoppe CORBA.
static mutex          m;
static CORBA::ORB_ptr orb;
static bool           hintKilled=false;


int 
main(int argn, char** argv)
{
   bool error;
   string pidfile;

   InitLogger(argn, argv);

   pidfile= KvApp::createPidFileName( "kvDataInputd" );

   if(dnmi::file::isRunningPidFile(pidfile, error)){
      if(error){
         LOGFATAL("An error occured while reading the pidfile:" << endl
                  << pidfile << " remove the file if it exists and"
                  << endl << "kvDataInputd is not running. " <<
                  "If it is running and there are problems. Kill kvDataInputd and"
                  << endl << "restart it." << endl << endl);
         return 1;
      }else{
         LOGFATAL("Is kvDataInputd allready running?" << endl
                  << "If not remove the pidfile: " << pidfile);
         return 1;
      }
   }


   //Read all connection information from the config file
   //$KVALOBS/etc/kvalobs.conf or environment.
   //ie: KVDB, KVDBUSER, PGHOST, PGPORT


   //  miutil::conf::ConfSection *conf;
   int          nWorkerThreads=3;
   CKvalObs::CDataSource::Data_ptr dataSource;

   //Look up the dbdriver from conf file.


   setSigHandlers();

   KvApp::createPidFile("kvDataInputd");

   DataSrcApp app(argn, argv, nWorkerThreads);

   if(!app.isOk()){
      LOGFATAL("Problems with initializing of kvDataInputd!\n");
   }

   //We create as many worker threads as we have database connections.
   nWorkerThreads=app.getDbConnections();

   if(nWorkerThreads<1){
      LOGFATAL("No db connections. We cant do anything, so we quit!\n");
      app.deletePidFile();
      return 1;
   }

   LOGINFO("Db connections: " << nWorkerThreads << endl);
   LOGINFO("Creates " << nWorkerThreads << " worker threads!\n");

   ThreadPoolQue thrPool((unsigned int)nWorkerThreads, *app.getQue(), 2);
   thrPool.setKillFunc(killThreadOnSignal);
   thrPool.setAfterFunc(threadAfter);
   thrPool.run();

   orb=app.getOrb();
   PortableServer::POA_ptr poa=app.getPoa();

   try{

      // We allocate the objects on the heap.  Since these are reference
      // counted objects, they will be deleted by the POA when they are no
      // longer needed.
      DataSrcImpl   *dataSrcImpl  = new DataSrcImpl(app);


      // Activate the objects.  This tells the POA that the objects are
      // ready to accept requests.
      PortableServer::ObjectId_var dataSrcIid = poa->activate_object(dataSrcImpl);

      // Obtain a reference to each object and output the stringified
      // IOR to std::cerr
      {
         dataSource = dataSrcImpl->_this();

         if(!app.putRefInNS( dataSource , "kvinput")){
            LOGFATAL("Can't register with CORBA nameservice!\n" <<
                     "Is the CORBA nameservice running!\n");
            app.deletePidFile();
            return 1;
         }
      }

      // Obtain a POAManager, and tell the POA to start accepting
      // requests on its objects.
      PortableServer::POAManager_ptr pman=app.getPoaMgr();
      pman->activate();

      app.createPidFile("kvDataInputd");

      orb->run();
      orb->destroy();
   }
   catch(CORBA::SystemException&) {
      LOGERROR("main: Caught CORBA::SystemException." << endl);
   }
   catch(CORBA::Exception&) {
      LOGERROR("main: Caught CORBA::Exception." << endl);
   }
   catch(omniORB::fatalException& fe) {
      LOGERROR("main: Caught omniORB::fatalException:" << endl
               << "  file: " << fe.file() << endl
               << "  line: " << fe.line() << endl
               << "  mesg: " << fe.errmsg() << endl);
   }
   catch(...) {
      LOGERROR("main: Caught unknown exception." << endl);
   }

   app.shutdown();

   thrPool.join();

   app.deletePidFile();
   return 0;
}


void
setSigHandlers()
{
   sigset_t     oldmask;
   struct sigaction act, oldact;


   act.sa_handler=sig_term;
   sigemptyset(&act.sa_mask);
   act.sa_flags=0;

   if(sigaction(SIGTERM, &act, &oldact)<0){
      LOGFATAL("ERROR: Can't install signal handler for SIGTERM\n");
      exit(1);
   }

   act.sa_handler=sig_term;
   sigemptyset(&act.sa_mask);
   act.sa_flags=0;

   if(sigaction(SIGINT, &act, &oldact)<0){
      LOGFATAL("ERROR: Can't install signal handler for SIGTERM\n");
      exit(1);
   }
}

bool 
killThreadOnSignal()
{
   if(sigTerm){
      LOGINFO("killThreadOnSignal: dying ....\n");

      mutex::scoped_lock scoped_lock(m);

      if(!hintKilled){
         orb->shutdown(0);
         hintKilled=true;
      }
   }

   return sigTerm;
}


bool 
threadAfter(CommandBase *cmd)
{
   DecodeCommand *dec = dynamic_cast<DecodeCommand *>(cmd);;

   if( ! dec ) {
      LOGFATAL("kvDataInputd: threadAfter: failed dynamic_cast!\n");
      return false;
   }

   try{
      LOGDEBUG("kvDataInputd: threadAfter: BEFORE call too: dec->signal()!\n");
      dec->signal();
      LOGDEBUG("kvDataInputd: threadAfter: AFTER call too: dec->signal()!\n");
      return true;
   }
   catch(std::exception& ex) {
      LOGFATAL("kvDataInputd: exception in dec->signal: " << ex.what());
   }
   catch(...) {
      LOGFATAL("kvDataInputd: unknown exception in dec->signal");
   }

   return false;
}

void 
sig_term(int)
{
   sigTerm=1;
}
