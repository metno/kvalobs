/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceSubscriber.cc,v 1.12.2.2 2007/09/27 09:02:39 paule Exp $                                                       

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

#include <map>
#include <sstream>
#include <cstring>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvWorkelement.h>
#include <miutil/timeconvert.h>
#include <milog/milog.h>
#include "ServiceSubscriber.h"



using namespace std;
using namespace kvalobs;
using namespace miutil;


namespace{
const unsigned char QC1_mask=0x01;
const unsigned char QC2d_mask=0x02;
const unsigned char QC2m_mask=0x04;
const unsigned char HQC_mask=0x08;

string
buildDataQuery( const kvalobs::kvStationInfoExt &st )
{
   ostringstream q;
   list<kvalobs::kvStationInfoExt::Param> params=st.params();

   q << " WHERE stationid=" << st.stationID()
        << " AND typeid="      << st.typeID()
        << " AND obstime=\'"   << to_kvalobs_string(st.obstime()) << "\' " ;

   if( params.size() !=0 ) {
      list<kvalobs::kvStationInfoExt::Param>::const_iterator it = params.begin();
      q << " AND ((paramid=" << it->paramid << "AND sensor='"<< it->sensor <<"' AND level=" << it->level << ")";
      ++it;

      for( ; it != params.end(); ++it )
         q << " OR (paramid=" << it->paramid << "AND sensor='"<< it->sensor <<"' AND level="<< it->level << ")";

      q << ")";
   }

   q << " ORDER BY paramid, level, sensor";
   return q.str();
}

string
buildTextDataQuery( const kvalobs::kvStationInfoExt &st )
{
   ostringstream q;
   list<kvalobs::kvStationInfoExt::Param> params=st.params();

   q << " WHERE stationid=" << st.stationID()
        << " AND typeid="      << st.typeID()
        << " AND obstime=\'"   << to_kvalobs_string(st.obstime()) << "\' " ;

   if( params.size() !=0 ) {
      list<kvalobs::kvStationInfoExt::Param>::const_iterator it = params.begin();
      q << " AND ( paramid=" << it->paramid ;
      ++it;

      for( ; it != params.end(); ++it )
         q << " OR paramid=" << it->paramid;

      q << ")";
   }

   q << " ORDER BY paramid";
   return q.str();
}



};


ServiceSubscriber::
ServiceSubscriber(ServiceApp &app_,
                  dnmi::thread::CommandQue &que_)
:app(app_), inputque(que_), dbCon(0)
{
}

ServiceSubscriber::
ServiceSubscriber(const ServiceSubscriber &s)
:app(s.app), inputque(s.inputque), dbCon(s.dbCon)
{
}

ServiceSubscriber::
~ServiceSubscriber()
{
}


void 
ServiceSubscriber::
updateWorkelementServiceStart(const kvalobs::kvStationInfoExt &st,
                              dnmi::db::Connection *con,
                              const std::string &logid)
{
   kvDbGate gate(con);
   ostringstream ost;

   ost << "UPDATE workque SET service_start='"
         << miTime::nowTime()
   << "' WHERE stationid=" << st.stationID()
   << "  AND obstime='" << to_kvalobs_string(st.obstime())
   << "' AND typeid=" << st.typeID();


   if(!gate.exec(ost.str())){
      LOGERROR("DBERROR: Cant update workque!" << endl <<
               "Reason: " << gate.getErrorStr());

      if( ! logid.empty() ){
         IDLOGERROR(logid, "DBERROR: Cant update workque!" << endl <<
                    "Reason: " << gate.getErrorStr());
      }
   }

}

void 
ServiceSubscriber::
updateWorkelementServiceStop(const kvalobs::kvStationInfoExt &st,
                             dnmi::db::Connection *con,
                             const std::string &logid)
{
   kvDbGate gate(con);
   ostringstream ost;
   list<kvWorkelement> workList;

   ost << "UPDATE workque SET service_stop='"
         << miTime::nowTime()
   << "' WHERE stationid=" << st.stationID()
   << "  AND obstime='" << to_kvalobs_string(st.obstime())
   << "' AND typeid=" << st.typeID();


   if(!gate.exec(ost.str())){
      LOGERROR("DBERROR: Cant update workque!" << endl <<
               "Reason: " << gate.getErrorStr());
      if( !logid.empty() ) {
         IDLOGERROR(logid, "DBERROR: Cant update workque!" << endl <<
                    "Reason: " << gate.getErrorStr());
      }
      return;
   }
}


void 
ServiceSubscriber::
callDataNotifySubscribers(const kvalobs::kvStationInfoExt &si,
                          const std::string &logid )
{
   if(!app.subscribers.hasDataNotifySubscribers())
      return;

   if(!dbCon){
      LOGERROR("DataNotify: No database connection! (dbCon==0)");

      if( !logid.empty() ) {
         IDLOGERROR(logid, "DataNotify: No database connection! (dbCon==0)");
      }

      return;
   }

   kvalobs::kvDbGate gate(dbCon);

   list<kvData> dataList;

   if(!gate.select(dataList,
                   kvQueries::selectDataFromType(si.stationID(),
                                                 si.typeID(),
                                                 si.obstime()))){
      if( logid.empty() ) {
         LOGWARN( "NODATA (notifysub) source <?>: stationid: " << si.stationID() << " typeid: " << si.typeID() << " obstime: " << si.obstime() );
      } else {
         LOGWARN( "NODATA (notifysub) source <" << logid << ">: stationid: " << si.stationID() << " typeid: " << si.typeID() << " obstime: " << si.obstime() );
      }

      if( !logid.empty() ) {
         IDLOGWARN( logid, "NODATA (notifysub): stationid: " << si.stationID() << " typeid: " << si.typeID() << " obstime: " << si.obstime() );
      }
      return;
   }

   DataNotifyFunc dataNotify(si, dataList);
   LOGDEBUG("CALL DataNotifySubscribers: stationID(" << si.stationID() <<
            ")\n" << si);

   if( !logid.empty() ) {
      IDLOGDEBUG(logid, "CALL DataNotifySubscribers: stationID(" << si.stationID() <<
                 ")\n" << si);
   }

   app.subscribers.forAllDataNotifySubscribers(dataNotify, si.stationID());
   removeDeadConnections();
}

void 
ServiceSubscriber::
callDataSubscribers(const kvalobs::kvStationInfoExt &si,
                    const std::string &logid)
{
   long stationID;
   std::list<kvalobs::kvData>       dataList;
   std::list<kvalobs::kvTextData>   textDataList;
   DataToSendList                   dataToSend;

   if(!app.subscribers.hasDataSubscribers())
      return;

   if(!dbCon){
      LOGERROR("callDataSubscribers: dbCon==0!");

      if( !logid.empty() ) {
         IDLOGERROR(logid, "callDataSubscribers: dbCon==0!");
      }

      return;
   }

   kvalobs::kvDbGate gate(dbCon);

   if(!gate.select(dataList, buildDataQuery( si ) ) ){
      dataList.clear();
   }

   if(!gate.select(textDataList, buildTextDataQuery( si ) ) ) {
      textDataList.clear();
   }

   if(dataList.empty() && textDataList.empty()) {
      if( logid.empty() ) {
         LOGWARN( "NODATA (datasub) source <?>: stationid: " << si.stationID() << " typeid: " << si.typeID() << " obstime: " << si.obstime() );
      } else {
         LOGWARN( "NODATA (datasub) source <" << logid << ">: stationid: " << si.stationID() << " typeid: " << si.typeID() << " obstime: " << si.obstime() );
      }

      if( ! logid.empty() ) {
         IDLOGWARN( logid, "NODATA (datasub): stationid: " << si.stationID() << " typeid: " << si.typeID() << " obstime: " << si.obstime() );
      }

      return;
   }

   dataToSend.push_back(DataToSend(dataList, textDataList, si.stationID()));

   DataFunc dataf(dataToSend);

   stationID=si.stationID();

   LOGDEBUG("CALL DataSubscribers: stationID: "<< si.stationID()
            << " obstime: " << si.obstime()
            << " typeID: " << si.typeID());
   if( !logid.empty() ) {
      IDLOGDEBUG( logid, "CALL DataSubscribers: stationID: "<< si.stationID()
                  << " obstime: " << si.obstime()
                  << " typeID: " << si.typeID());
   }
   app.subscribers.forAllDataSubscribers(dataf, stationID);
   removeDeadConnections();
}

void       
ServiceSubscriber::
removeDeadConnections()
{
   app.subscribers.removeDeadSubscribers(60);
}

void       
ServiceSubscriber::
operator()()
{
   const                     int CON_IDLE_TIME=60;
   const                     int WAIT_ON_QUE_TIMEOUT=1;
   int                       conIdleTime=0;
   DataReadyCommand          *stInfoCmd=0;
   dnmi::thread::CommandBase *cmd=0;
   bool                      fromKvManager;
   std::string               logName;

   milog::LogContext logContext("ServiceSubscriber");

   while(!app.shutdown()){
      cmd=inputque.get(WAIT_ON_QUE_TIMEOUT);

      if(!cmd){
         conIdleTime+=WAIT_ON_QUE_TIMEOUT;

         if(conIdleTime>CON_IDLE_TIME){
            if(dbCon){
               LOGDEBUG("Closing the database connection!");
               app.releaseDbConnection(dbCon);
               dbCon=0;
            }

            conIdleTime=0;
         }

         continue;
      }

      if(!dbCon){
         //Will try to create a new connection to the database, we will
         //not continue before a connection is created or the application
         //is shutdown.

         do{
            dbCon=app.getNewDbConnection();

            if(dbCon){
               LOGDEBUG("Created a new connection to the database!");
               break;
            }

            LOGINFO("Can't create a connection to the database, retry in 5 seconds ..");
            sleep(5);
         }while(!app.shutdown());

         if(!dbCon){
            //We have failed to create a new database connection and we have got a
            //shutdown condition. We use continue to evaluate the outer while loop
            //that in turn will end this thread.
            continue;
         }
      }

      try{
         stInfoCmd=dynamic_cast<DataReadyCommand*>(cmd);

         if(!stInfoCmd){
            delete cmd;
            LOGERROR("Unexpected command!");
            continue;
         }
      }
      catch(...){
         LOGERROR("Exception: unexpected command!");
         continue;
      }

      if( stInfoCmd->source() == "__##kvManagerd@@very_secret_hash:-)##__" )
         fromKvManager = true;
      else
         fromKvManager = false;

      if( fromKvManager ) {
         LOGDEBUG("DataReady received from <kvManager>!");
         logName = "kvManagerd";
      } else if( stInfoCmd->source().empty() ) {
         LOGDEBUG("DataReady received from <> (Unknown)!");
         logName = "UNKNOWN";
      } else {
         LOGDEBUG("DataReady received from <" << stInfoCmd->source() << ">!");
         logName = stInfoCmd->source();
      }

      string savedLogger( logName );

      if( ! app.createGlobalLogger( logName, milog::DEBUG ) ) {
         LOGERROR( "Failed to create logger <" << savedLogger << ">!");
      }

      if( ! logName.empty() ) {
         IDLOGDEBUG( logName, "Number of datasets: " << stInfoCmd->getStationInfo().size()  );
      }

      conIdleTime=0;
      kvalobs::IkvStationInfoExtList it=stInfoCmd->getStationInfoExt().begin();

      for(;it!=stInfoCmd->getStationInfoExt().end(); it++){
         if( !logName.empty() ) {
            IDLOGDEBUG( logName, "Observation: stationid: " << it->stationID() << " typeid: " << it->typeID() << " obstime: " << it->obstime() );
         }

         if( fromKvManager )
            updateWorkelementServiceStart(*it, dbCon, logName);

         callDataNotifySubscribers(*it, logName);
         callDataSubscribers(*it, logName);

         if( fromKvManager )
            updateWorkelementServiceStop(*it, dbCon, logName);
      }

      if( fromKvManager )
         app.sendToManager( stInfoCmd->getStationInfo(),
                            stInfoCmd->getCallback() );

      delete stInfoCmd;
   }

   LOGINFO("ServiceSubscriber terminated!");
} 




void 
DataNotifyFunc::func(KvDataNotifySubscriberPtr ptr)
{
   using namespace CKvalObs::CService;
   kvDataNotifySubscriber::WhatList wl;

   if(!checkStatusAndQc(ptr)){
      return;
   }


   if(!buildWhatList(wl)){
      LOGERROR("DataNotifyFunc::func: buildWhatList failed!\n");
      return;
   }

   try{
      CKvalObs::CService::kvDataNotifySubscriber_var ref=ptr->subscriber();

      ref->callback(wl);
      ptr->connection(true);
   }
   catch(CORBA::TRANSIENT &ex){
      ptr->connection(false, true);
      LOGERROR("EXCEPTION: (timeout?) Can't send <DataNotify> event to subscriber!" <<
               endl << "Subscriberid: " << ptr->subscriberid() << ">!");
   }
   catch(...){
      ptr->connection(false);
      LOGERROR("EXCEPTION: Can't send <DataNotify> event to subscriber!" <<
               endl << "Subscriberid: " << ptr->subscriberid() << ">!");
   }
}

bool
DataNotifyFunc::buildWhatList(
      CKvalObs::CService::kvDataNotifySubscriber::WhatList &wl
)
{
   unsigned char        qcLevel=0x00;
   unsigned char        flag;
   char                 b[100];
   CORBA::Long          i=0;
   CORBA::Long          wli=0;
   //  kvalobs::CIkvParamInfoList it;

   wl.length(wli+1);
   wl[wli].stationID=stationInfo.stationID();
   wl[wli].typeID_=stationInfo.typeID();
   wl[wli].obsTime=to_kvalobs_string(stationInfo.obstime()).c_str();


   for(list<kvData>::const_iterator it=dataList.begin();
         it!=dataList.end();
         it++){
      flag=it->controlinfo().cflag(0);
      qcLevel |=flag;
   }

   i=0;

   if(qcLevel & QC1_mask){
      wl[wli].qc.length(i+1);
      wl[wli].qc[i]=CKvalObs::CService::QC1;
      i++;
   }

   if(qcLevel & QC2d_mask){
      wl[wli].qc.length(i+1);
      wl[wli].qc[i]=CKvalObs::CService::QC2d;
      i++;
   }

   if(qcLevel & QC2m_mask){
      wl[wli].qc.length(i+1);
      wl[wli].qc[i]=CKvalObs::CService::QC2m;
      i++;
   }

   if(qcLevel & HQC_mask){
      wl[wli].qc.length(i+1);
      wl[wli].qc[i]=CKvalObs::CService::HQC;
      i++;
   }

   return true;
}

bool
DataNotifyFunc::fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag)
{
   switch(qcId){
   case CKvalObs::CService::QC1:
      if(flag & QC1_mask)
         return true;
      break;

   case CKvalObs::CService::QC2d:
      if(flag & QC2d_mask)
         return true;
      break;

   case CKvalObs::CService::QC2m:
      if(flag & QC2m_mask)
         return true;
      break;

   case CKvalObs::CService::HQC:
      if(flag & HQC_mask)
         return true;
      break;
   }

   return false;
}


bool
DataNotifyFunc::checkStatusAndQc(KvDataNotifySubscriberPtr ptr)
{
   unsigned char flag;

   if(!ptr->subscriberInfo().qcAll()){
      //Check if the subscriber is interested in the qcLevels that
      //is set for this observation. We return true if we find a
      //qcLevel that match for any parameter in the observation.
      bool qcLevel=false;

      for(list<kvData>::const_iterator it=dataList.begin();
            it!=dataList.end() && !qcLevel; it++){
         flag=it->controlinfo().cflag(0);

         for(int i=0; i<ptr->subscriberInfo().qc().length(); i++){
            if(fqcLevel(ptr->subscriberInfo().qc()[i], flag)){
               qcLevel=true;
               break;
            }
         }
      }

      if(!qcLevel)
         return false;
   }


   if(ptr->subscriberInfo().status()!=CKvalObs::CService::All){
      bool hasFailed=false;

      for(list<kvData>::const_iterator it=dataList.begin();
            it!=dataList.end() && !hasFailed ; it++){
         flag=it->useinfo().cflag(0);

         //Check if bit 1 in useinfo is set. This bit is set to 1 if
         //the value is useless.
         //CODE: Check if this select the correct bit.
         if(flag & 0x01)
            hasFailed=true;
      }

      if(hasFailed &&
            ptr->subscriberInfo().status()==CKvalObs::CService::OnlyFailed)
         return true;
      else if(!hasFailed &&
            ptr->subscriberInfo().status()==CKvalObs::CService::OnlyOk)
         return true;
      else
         return false;
   }

   return true;

}


bool 
DataFunc::fqcLevel(CKvalObs::CService::QcId qcId, unsigned char flag)
{
   switch(qcId){
   case CKvalObs::CService::QC1:
      if(flag & QC1_mask)
         return true;
      break;

   case CKvalObs::CService::QC2d:
      if(flag & QC2d_mask)
         return true;
      break;

   case CKvalObs::CService::QC2m:
      if(flag & QC2m_mask)
         return true;
      break;

   case CKvalObs::CService::HQC:
      if(flag & HQC_mask)
         return true;
      break;
   }

   return false;
}

bool 
DataFunc::checkStatusAndQc(KvDataSubscriberPtr ptr)
{
   CORBA::Long   it;
   CORBA::Long   i;
   unsigned char flag;

   if(!ptr->subscriberInfo().qcAll()){
      //Check if the subscriber is interested in the qcLevels that
      //is set for this observation. We return true if we find a
      //qcLevel that match for any parameter in the observation.
      bool qcLevel=false;

      for(i=0; i<data.length(); i++){
         for(it=0;it<data[i].dataList.length() && !qcLevel; it++){
            flag=kvControlInfo(string(data[i].dataList[it].controlinfo)).cflag(0);

            for(int i=0; i<ptr->subscriberInfo().qc().length(); i++){
               if(fqcLevel(ptr->subscriberInfo().qc()[i], flag)){
                  qcLevel=true;
                  break;
               }
            }
         }

         if(!qcLevel)
            return false;
      }
   }


   if(ptr->subscriberInfo().status()!=CKvalObs::CService::All){
      bool hasFailed=false;

      for(i=0; i<data.length(); i++){
         for(it=0;it<data[i].dataList.length() && !hasFailed ; it++){
            flag=kvUseInfo(string(data[i].dataList[it].useinfo)).cflag(0);

            //Check if bit 1 in useinfo is set. This bit is set to 1 if
            //the value is useless.
            //CODE: Check if this select the correct bit.
            if(flag & 0x01)
               hasFailed=true;
         }
      }

      if(hasFailed && 
            ptr->subscriberInfo().status()==CKvalObs::CService::OnlyFailed)
         return true;
      else if(!hasFailed && 
            ptr->subscriberInfo().status()==CKvalObs::CService::OnlyOk)
         return true;
      else
         return false;
   }

   return true;

}


DataFunc::DataFunc(const DataToSendList &dataList)
{
   CORBA::Long            obsi=0;
   CORBA::Long            datai;
   char                   *sTmp;
   char                   buf[64];
   bool                   hasData;

   data.length(dataList.size());

   for(CIDataToSendList itd=dataList.begin();
         itd!=dataList.end();
         itd++){
      hasData=false;

      if(itd->dataList.size()==0 && itd->textDataList.size()==0)
         continue;

      data[obsi].stationid=itd->stationid;

      if(itd->dataList.size()>0){
         datai=0;
         data[obsi].dataList.length(itd->dataList.size());
         hasData=true;

         for( list<kvalobs::kvData>::const_iterator it=itd->dataList.begin();
               it!=itd->dataList.end();
               it++) {
            /* DEBUG
      	  LOGDEBUG("DS: " << *it );
             */
            data[obsi].dataList[datai].stationID   = it->stationID();
            data[obsi].dataList[datai].obstime     = to_kvalobs_string_without_decimal_secound(it->obstime()).c_str();
            data[obsi].dataList[datai].original    = it->original();
            data[obsi].dataList[datai].paramID     = it->paramID();
            data[obsi].dataList[datai].tbtime      = to_kvalobs_string_without_decimal_secound(it->tbtime()).c_str();
            data[obsi].dataList[datai].typeID_     = it->typeID();
            data[obsi].dataList[datai].level       = it->level();
            data[obsi].dataList[datai].corrected   = it->corrected();
            data[obsi].dataList[datai].controlinfo = it->controlinfo().flagstring().c_str();
            data[obsi].dataList[datai].useinfo     = it->useinfo().flagstring().c_str();
            data[obsi].dataList[datai].cfailed     = it->cfailed().c_str();

            sprintf(buf, "%d", it->sensor());
            sTmp=CORBA::string_dup(buf);

            if(sTmp){
               data[obsi].dataList[datai].sensor=sTmp;
            }else{
               LOGERROR("DataFunc (CTOR): NOMEM for <kvData::sensor>!");
            }

            datai++;
         }

         if(datai!=itd->dataList.size()){
            LOGERROR("Datafunc (CTOR): Inconsistent size, dataList!");
            data[obsi].dataList.length(datai);
         }

      }else{
         data[obsi].dataList.length(0);
      }

      if(itd->textDataList.size()>0){
         datai=0;
         data[obsi].textDataList.length(itd->textDataList.size());
         hasData=true;

         for( list<kvalobs::kvTextData>::const_iterator it=itd->textDataList.begin();
               it!=itd->textDataList.end();
               it++) {
            data[obsi].textDataList[datai].stationID=it->stationID();
            data[obsi].textDataList[datai].obstime=to_kvalobs_string_without_decimal_secound(it->obstime()).c_str();
            data[obsi].textDataList[datai].original=it->original().c_str();
            data[obsi].textDataList[datai].paramID=it->paramID();
            data[obsi].textDataList[datai].tbtime=to_kvalobs_string_without_decimal_secound(it->tbtime()).c_str();
            data[obsi].textDataList[datai].typeID_=it->typeID();

            datai++;
         }

         if(datai!=itd->textDataList.size()){
            LOGERROR("Datafunc (CTOR): Inconsistent size, textDataList!");
            data[obsi].textDataList.length(datai);
         }

      }else{
         data[obsi].textDataList.length(0);
      }


      if(hasData)
         obsi++;
   }

   data.length(obsi);
}

void 
DataFunc::func(KvDataSubscriberPtr ptr)
{
   using namespace CKvalObs::CService;

   if(!checkStatusAndQc(ptr)){
      return;
   }

   /* DEBUG
 ostringstream ost;
 for( CORBA::Long i=0; i<data.length(); ++i) {
	 for( CORBA::Long k=0; k<data[i].dataList.length(); ++k ) {
		 ost << "CORBA["
			     	<< "sid: "    << data[i].dataList[k].stationID
			     	<< " otime: " << data[i].dataList[k].obstime
			     	<< " tid: "   << data[i].dataList[k].typeID_
					<< " pid: "   << data[i].dataList[k].paramID
					<< " lvl: "   << data[i].dataList[k].level
					<< " sen: "   << data[i].dataList[k].sensor
					<< " orig: "  << data[i].dataList[k].original
					<< " cor: "   << data[i].dataList[k].corrected
					<< " cinfo: " << data[i].dataList[k].controlinfo
					<< " uinfo: " << data[i].dataList[k].useinfo
					<< "]" << endl;
	 }	 
 }

 LOGDEBUG( ost.str() );
    */

   try{
      kvDataSubscriber_var ref=ptr->subscriber();
      ref->callback(data);
      ptr->connection(true);
   }
   catch(CORBA::TRANSIENT &ex){
      ptr->connection(false, true);
      LOGERROR("EXCEPTION: (timeout?) Can't send <Data> event to subscriber!" <<
               endl << "Subscriberid: " << ptr->subscriberid() << ">!");
   }
   catch(...){
      ptr->connection(false);
      LOGERROR("EXCEPTION: Can't send <Data> event to subscriber!" <<
               endl << "Subscriberid: " << ptr->subscriberid() << ">!");
   }
}
