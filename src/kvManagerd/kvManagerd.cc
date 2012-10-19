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
#include <boost/thread/thread.hpp>
#include <dnmithread/CommandQue.h>
#include <milog/milog.h>
#include <miconfparser/miconfparser.h>
#include <fileutil/pidfileutil.h>
#include "AdminImpl.h"
#include "mgrApp.h"
#include "managerInputImpl.h"
#include "checkedInputImpl.h"
#include "NewDataCommand.h"
#include "PreProcessWorker.h"
#include "InitLogger.h"
#include "PreProcessMissingData.h"
#include "kvCheckedDataThread.h"
#include "CheckForMissingObsMessages.h"
#include "NewDataJob.h"
#include "SelectDataToProcess.h"
#include "SendDataToQa.h"
#include "ServiceCheckedInputImpl.h"
#include <kvalobs/kvPath.h>
using namespace std;
using namespace boost;

int 
main(int argc, char** argv)
{
   CORBA::ORB_ptr orb;
   PortableServer::POA_ptr poa;
   dnmi::thread::CommandQue selectDataQue; //From kvDatainputd and
   //checkForMissingObsMessages
   dnmi::thread::CommandQue preProcessQue; //SelectDataToProcess
   dnmi::thread::CommandQue newDataQue;    //From preprosess
   dnmi::thread::CommandQue qaDataQue;  //From kvQabased
   string                   dbdriver;
   bool                     docheckForMissingObs=true;
   miutil::conf::ConfSection *conf=KvApp::getConfiguration();
   bool error;
   string pidfile;

   //Read all connection information from the $KVALOBS/etc/kvalobs.conf
   //if it exist. Otherwise use the environment varibales:
   //KVDB, KVDBUSER, PGHOST, PGPORT
   string constr(KvApp::createConnectString());


   if(conf){
      miutil::conf::ValElementList val=conf->getValue("database.dbdriver");

      if(val.size()==1)
         dbdriver=val[0].valAsString();
   }

   if(conf){
      miutil::conf::ValElementList val=conf->getValue("kvManagerd.check_for_missing_obs");

      if(val.size()==1){
         string v=val[0].valAsString();

         if(!v.empty() && (v[0]=='f' || v[0]=='F'))
            docheckForMissingObs=false;
      }
   }

   //Use postgresql as a last guess.
   if(dbdriver.empty())
      dbdriver="pgdriver.so";

   InitLogger(argc, argv, "kvManagerd");

   LOGINFO("check_for_missing_obs=" << (docheckForMissingObs?"true":"false"));

   pidfile = KvApp::createPidFileName( "kvManagerd" );

   LOGDEBUG("pidfile: " << pidfile);

   if(dnmi::file::isRunningPidFile(pidfile, error)){
      if(error){
         LOGFATAL("An error occured while reading the pidfile:" << endl
                  << pidfile << " remove the file if it exist and"
                  << endl << "kvManagerd is not running. " <<
                  "If it is running and there is problems. Kill kvManagerd and"
                  << endl << "restart it." << endl << endl);
         return 1;
      }else{
         LOGFATAL("Is kvManagerd allready running?" << endl
                  << "If not remove the pidfile: " << pidfile);
         return 1;
      }
   }

   LOGINFO("KvManagerd: starting ....");

   ManagerApp app(argc, argv, dbdriver, constr);

   if(!app.isOk()){
      return 1;
   }

   app.checkForMissingObs(docheckForMissingObs);

   orb=app.getOrb();
   poa=app.getPoa();

   SelectDataToProcess selectDataToProcess(app,
                                           selectDataQue,
                                           preProcessQue,
                                           newDataQue);

   PreProcessWorker    preProcessWorker(app, preProcessQue, newDataQue);

   //Add job to look up new data that has arrived when we was down.
   //preProcessWorker.addOneTimeJob(new NewDataJob);

   //Add jobs to the preProcessing unit.
   preProcessWorker.addJob(new PreProcessMissingData);


   SendDataToQa             newData(app, newDataQue);
   KvCheckedDataThread      checkedData(app, qaDataQue);
   CheckForMissingObsMessages checkForMissingObs(app, selectDataQue);

   thread selectDataToProcessThread(selectDataToProcess);
   thread checkForMissingObsThread(checkForMissingObs);
   thread preProcessThread(preProcessWorker); //Receives data from kvDataInputd
   thread newDataQueThread(newData);  //Receives data from preProcessThread
   thread checkedDataQueThread(checkedData); //Receives data from kvQaBased

   try {
      ManagerInputImpl *mgrImpl = new ManagerInputImpl(app, selectDataQue);
      CheckedInputImpl *chkImpl = new CheckedInputImpl(app, qaDataQue);
      AdminImpl        *admImpl = new AdminImpl(app);
      ServiceCheckedInputImpl *srvImpl=
            new ServiceCheckedInputImpl(app,qaDataQue);

      PortableServer::ObjectId_var mgrImplIid=poa->activate_object(mgrImpl);
      PortableServer::ObjectId_var chkImplIid=poa->activate_object(chkImpl);
      PortableServer::ObjectId_var admImplIid=poa->activate_object(admImpl);
      PortableServer::ObjectId_var srvImplIid=poa->activate_object(srvImpl);

      {
         // IDL interface: CKvalObs::manager::ManagerInput
         CORBA::Object_var ref = mgrImpl->_this();

         if(!app.putRefInNS(ref, "kvManagerInput")){
            LOGFATAL("FATAL: can't register with CORBA nameserver!\n");
            return 1;
         }

         CORBA::String_var sior(orb->object_to_string(ref));
         cout << "IDL object kvManagerInput IOR = '" << (char*)sior << "'" << endl;
         // IDL interface: micutil::Admin
         ref = admImpl->_this();

         if(!app.putRefInNS(ref, "kvManagerAdmin")){
            LOGFATAL("FATAL: can't register with CORBA nameserver!\n");
            return 1;
         }

         sior=orb->object_to_string(ref);
         cout << "IDL object micutil::Admin IOR = '" << (char*)sior << "'" << endl;

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
   }
   catch(CORBA::SystemException&) {
      LOGFATAL( "Caught CORBA::SystemException." );
      app.deletePidFile();
      exit(1);
   }
   catch(CORBA::Exception&) {
      LOGFATAL("Caught CORBA::Exception.");
      app.deletePidFile();
      exit(1);
   }
   catch(omniORB::fatalException& fe) {
      LOGFATAL("Caught omniORB::fatalException:" << endl
               << "  file: " << fe.file() << endl
               << "  line: " << fe.line() << endl
               << "  mesg: " << fe.errmsg());
      app.deletePidFile();
      exit(1);
   }
   catch(...) {
      LOGFATAL( "Caught unknown exception.");
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

