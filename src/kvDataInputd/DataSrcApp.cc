/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataSrcApp.cc,v 1.26.2.3 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <boost/lexical_cast.hpp>
#include <milog/milog.h>
#include "DataSrcApp.h"
#include <miutil/timeconvert.h>
#include <kvalobs/bitmanip.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvPath.h>

using namespace dnmi::file;
using namespace std;
using namespace dnmi::db;
using namespace kvalobs::decoder;
using namespace miutil;
using namespace boost;

volatile sig_atomic_t sigTerm=0;

DataSrcApp::DataSrcApp(int argn, char **argv,
                       int               nConnections_,
                       const char *opt[][2])
:KvApp(argn, argv, opt), refMgr(CKvalObs::CManager::ManagerInput::_nil()),
 ok(false), shutdown_(false)
{
   miutil::conf::ConfSection *conf;
   string logdir( kvPath("logdir") );
   string myPath=kvPath("pkglibdir");
   myPath+="/decode";

   conf=KvApp::getConfiguration();

   if(!conf){
      LOGFATAL("Cant read configuration file: " << getConfFile() << endl);
      cerr << "Cant read configuration file: " << getConfFile() << endl;
      exit(1);
   }

   miutil::conf::ValElementList val=conf->getValue("database.dbdriver");

   if(val.size()==1)
      dbDriver=val[0].valAsString();

   connectStr=KvApp::createConnectString();

   if(dbDriver.empty())
      dbDriver="pgdriver.so";

   decoderMgr.setDecoderPath(myPath);

   nConnections=registerDb(nConnections_);

   if(nConnections<1)
      return;

   if(!registerAllDecoders())
      return;

   if(!registerParams())
      return;

   if(!registerTypes())
      return;

   milog::createGlobalLogger( logdir, "kvDataInputd_transaction", "failed", milog::DEBUG );
   milog::createGlobalLogger( logdir, "kvDataInputd_transaction", "duplicates", milog::DEBUG );
   milog::createGlobalLogger( logdir, "kvDataInputd_transaction", "updated", milog::DEBUG );
   milog::createGlobalLogger( logdir, "kvDataInputd_transaction", "retry", milog::DEBUG );
   milog::createGlobalLogger( logdir, "kvDataInputd", "transaction", milog::DEBUG,
                              200, 1,  new milog::StdLayout1() );

   ok=true;
}

/*
 *lookUpManager will either return the refMgr or look up kvManagerInput'
 *in the CORBA nameservice.
 */ 
CKvalObs::CManager::ManagerInput_ptr 
DataSrcApp::lookUpManager(bool forceNS, bool &usedNS)
{
   CORBA::Object_var obj;
   usedNS=false;

   while(true){
      if(forceNS){
         usedNS=true;

         obj=getRefInNS("kvManagerInput");

         if(CORBA::is_nil(obj))
            throw LookUpException("EXCEPTION: Can't obtain a reference for 'kvManagerInput'\n           from the CORBA nameserver!");

         refMgr=CKvalObs::CManager::ManagerInput::_narrow(obj);

         if(CORBA::is_nil( refMgr ))
            throw LookUpException("EXCEPTION: Can't narrow reference for 'kvManagerInput'!");

         return refMgr;
      }

      if(CORBA::is_nil(refMgr))
         forceNS=true;
      else
         return refMgr;
   }
}

bool 
DataSrcApp::sendInfoToManager(const kvalobs::kvStationInfoList &info_)
{
   bool forceNS=false;
   bool usedNS=false;
   CORBA::Long  infoIndex;
   CORBA::Long  paramIndex;
   kvalobs::kvStationInfoList &info=const_cast<kvalobs::kvStationInfoList&>(info_);

   CKvalObs::CManager::ManagerInput_ptr mngr;
   CKvalObs::StationInfoList infoList;
   kvalobs::IkvStationInfoList it;

   infoList.length(info.size());

   LOGDEBUG("sendInfoToManager: \n" <<
            "info::size=" << info.size() << "\n" <<
            "infoList::length=" << infoList.length() << "\n");

   it=info.begin();

   if(it==info.end()){
      LOGDEBUG("sendInfoToManager: \n"<<
               "info_::size=" << info_.size() << "\n" <<
               "No data to send to kvManagerd!\n");
      return true;
   }

   for(infoIndex=0; it!=info.end(); it++, infoIndex++){
      infoList[infoIndex].stationId=it->stationID();
      infoList[infoIndex].obstime=to_kvalobs_string(it->obstime()).c_str();
      infoList[infoIndex].typeId_=it->typeID();
   }

   LOGDEBUG("sendInfoToManager: StationInfo packed!\n");


   try{
      for(int i=0; i<2; i++){
         mngr=lookUpManager(forceNS, usedNS);

         try{
            mngr->newData(infoList);
            return true;
         }
         catch(CORBA::TRANSIENT &ex){
            LOGINFO("(kvmanager) Exception CORBA::TRANSIENT!\n");
         }
         catch(CORBA::COMM_FAILURE &ex){
            LOGERROR("(kvmanager) Exception CORBA::COMM_FAILURE!\n" <<
                     "Is kvManager running\n");
         }
         catch(...){
            LOGERROR("(kvmanager) Exception unknown!\n");
            return false;
         }

         if(usedNS){
            LOGWARN("can't send data to kvManagerInput!\n");
            return false;
         }
         forceNS=true;

      }
   }
   catch(LookUpException &ex){
      LOGWARN("LookUpException: " << ex.what() << endl);
      return false;
   }
   catch(...){
      LOGWARN("Unknown Exception: endInfoToManager: \n" <<
              "hmmm, very strange, a unkown exception!\n");
      return false;
   }

   //Shall newer happend!
   return false;

}



DataSrcApp::~DataSrcApp()
{
}


int 
DataSrcApp::registerDb(int nConn)
{
   Connection *con;
   string driver( kvPath("pkglibdir")+"/db/"+dbDriver);
   string drvId;
   int    n=0;

   LOGINFO("registerDb: loading driver <" << dbMgr.fixDriverName( driver ) << ">!\n");

   if(!dbMgr.loadDriver(driver, drvId)){
      //LOGFATAL("registerDb: Can't load driver <" << driver << ">\n");
      LOGFATAL(dbMgr.getErr());
      return 0;
   }

   if( setAppNameForDb && !appName.empty() )
      dbMgr.setAppName( appName );

   LOGINFO("registerDb: Driver <" << drvId<< "> loaded!\n");

   for(int i=0; i<nConn; i++){
      con=0;

      while(!con && !inShutdown()){
         con=dbMgr.connect(drvId, connectStr);

         if(!con){
            LOGFATAL("registerDb: Can't create connection to <"
                  << drvId << ">\n ERROR message: " << dbMgr.getErr());
            sleep(1);
         }else{
            conCache.addConnection(con);
            n++;
         }

      }
   }


   return n;
}

bool 
DataSrcApp::registerAllDecoders()
{
   if(decoderMgr.numberOfDecoders()<1){
      LOGERROR("registerAllDecoders: Can't registers decoders!\n");
      return false;
   }

   return true;
}



bool
DataSrcApp::registerParams()
{
   Result     *res=0;
   Connection *con;
   string    msg;
   string    kode;
   int         id;
   con=conCache.findFreeConnection();

   if(!con){
      LOGDEBUG("registerParams: SIGNAL: Stopped on signal!\n");
      return false;
   }

   try{

      res=con->execQuery("SELECT paramid,name FROM param");

      if(res && res->size()>0){

         while(res->hasNext()){
            DRow row=res->next();

            try{
               id=lexical_cast<int>(row[0]);
               paramList.insert(Param(row[1], id));
            }
            catch(bad_lexical_cast &){
               LOGERROR("registerParams: BADNUM: kvnumber is not a number\n" <<
                        "   kvnumber(" << row[0] << ")\n");
            }
         }

         delete res;
      }
   }
   catch(SQLException &ex){
      delete res;
      conCache.freeConnection(con);
      LOGERROR("registerParams: Exception: " << ex.what() << endl);
      return false;
   }
   catch(...){
      delete res;
      conCache.freeConnection(con);
      LOGERROR("registerParams: Exception: Unkown\n");
   }

   conCache.freeConnection(con);

   //CERR(paramList);

   return true;
}

bool
DataSrcApp::registerTypes()
{
   Connection *con=conCache.findFreeConnection();

   if(!con){
      LOGDEBUG("registerTypes: SIGNAL: Stopped on signal!\n");
      return false;
   }

   kvalobs::kvDbGate gate(con);

   if(!gate.select(typeList)){
      LOGFATAL("registerTypes failed: " << gate.getErrorStr());
      conCache.freeConnection(con);
      return false;
   }

   conCache.freeConnection(con);

   return true;
}

DecodeCommand*
DataSrcApp::create(const char  *obsType_, 
                   const char  *obs,
                   long        timoutIn_msec,
                   ErrCode     &errCode,
                   std::string &err)
{
   DecoderBase   *dec;
   DecodeCommand *decCmd;
   Connection    *con;
   string    myErr;

   //The call to conCache will block until a connection object
   //is ready in the cache.
   con=conCache.findFreeConnection();

   if(!con){
      errCode=NoDbConnection;
      err="SIGNAL: Stopped on signal!";
      return 0;
   }

   //lookup the decoder based on obsType.
   {
      Lock lck(mutex);

      dec=decoderMgr.findDecoder(*con,
                                 paramList,
                                 typeList,
                                 obsType_,
                                 obs,
                                 myErr);
   }

   if(!dec){
      conCache.freeConnection(con);
      errCode=NoDecoder;
      err=myErr;
      return 0;
   }

   try{
      decCmd = new DecodeCommand(dec);
   }
   catch(...){
      errCode=NoMem;
      err="Out of memmory!";

      {
         Lock lck(mutex);
         decoderMgr.releaseDecoder(dec);
      }

      conCache.freeConnection(con);
   }

   return decCmd;

}

DecodeCommand*
DataSrcApp::create(CORBA::Long report_id, 
                   const char* report_type,
                   const char* obstime,
                   const char* data,
                   long        timoutIn_msec,
                   ErrCode     &errCode,
                   std::string &err)
{
}

void 
DataSrcApp::releaseDecodeCommand(DecodeCommand *command)
{
   DecoderBase   *dec;

   if(!command){
      LOGDEBUG("releaseDecodeCommand: ERROR command==0!");
      return;
   }

   dec=command->getDecoder();

   if(!dec){
      LOGDEBUG("releaseDecodeCommand: ERROR decoder==0!");
      delete command;
      return;
   }

   conCache.freeConnection(dec->getConnection());

   {
      Lock lck(mutex);
      decoderMgr.releaseDecoder(dec);
   }

   delete command;
}

bool                      
DataSrcApp::put(dnmi::thread::CommandBase *command)
{
   try{
      que.postAndBrodcast(command);
   }
   catch(...){
      return false;
   }

   return true;
}


dnmi::thread::CommandBase*                      
DataSrcApp::remove(dnmi::thread::CommandBase *command)
{
   dnmi::thread::CommandBase *cmd;

   return que.remove(command);
}




bool                      
DataSrcApp::isOk()const 
{ 
   if(KvApp::isOk() && ok)
      return true;

   return false;
}

bool 
DataSrcApp::inShutdown()const
{ 
   return shutdown_ || sigTerm;
}
