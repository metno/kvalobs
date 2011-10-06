/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synopdecoder.cc,v 1.18.2.5 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <puTools/miTime.h>
#include <miutil/commastring.h>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvTypes.h>
#include <fileutil/mkdir.h>
#include "synopdecoder.h"
#include "kvSynopDecoder.h"

using namespace kvalobs::decoder::synop;
using namespace std;
using namespace dnmi::db;
using namespace miutil;
using namespace boost;
using namespace kvalobs;


miutil::miTime SynopDecoder::lastStationCheck;
boost::mutex   SynopDecoder::mutex;
kvSynopDecoder SynopDecoder::synopDec;


SynopDecoder::SynopDecoder(
      dnmi::db::Connection   &con,
      const ParamList        &params,
      const std::list<kvalobs::kvTypes> &typeList,
      const miutil::miString &obsType,
      const miutil::miString &obs,
      int                    decoderId)
:DecoderBase(con, params, typeList, obsType, obs, decoderId)
{
}

SynopDecoder::~SynopDecoder()
{
}

miutil::miString 
SynopDecoder::name() const
{
   return "SynopDecoder";
}

long 
SynopDecoder::getStationId(miutil::miString & msg)
{
}

bool
SynopDecoder::saveData(list<kvalobs::kvData> &data, 
                       bool				 	 &rejected,
                       std::string			 &rejectedMessage)
{
   ostringstream logid;
   list<kvalobs::kvTextData> textData;
   list<kvalobs::kvData>::iterator it=data.begin();
   int i=0;
   int priority=10;

   rejected=false;

   if(it==data.end())
      return true;

   logid << "n" << it->stationID() << "-t" << it->typeID();

   if(it->stationID()<100000){
      //National stations that is registred in table 'station'.
      priority=6;
   }else if(it->stationID()<=10000000){
      //Foregn stations is in the range [100000, 10000000]
      priority=7;
   }else if(it->stationID()>10000000 && it->stationID()<130000000){
      //COMMENT:
      //As a quick fix to set priority. We reduce the priority
      //for ships. We now that ships get a automatic generated stationid
      //in the range [10100009, 123123599]. For

      priority=8;
   }

   for(;it!=data.end(); it++){
      if(it->obstime().undef() || it->tbtime().undef()){
         rejectedMessage="Missing obsTime or tbtime for observation!";
         rejected=true;
         return false;
      }
   }

   createLogger( logid.str() );

   int ret=true;
   it=data.begin();

   if( !addDataToDb( it->obstime(), it->stationID(), it->typeID(), data, textData, priority, logid.str() ) ) {
      ret = false;
   }

   removeLogger( logid.str() );

   return ret;
}

bool
SynopDecoder::initializeKvSynopDecoder()
{
   kvDbGate gate(getConnection());
   list<kvStation> stat;
   list<kvTypes>   synoptypes;

   //Get stations from the database

   if(!gate.select(stat, kvQueries::selectAllStations("stationid"))){
      LOGERROR("Can't get station data from table <station>!\n" <<
               gate.getErrorStr());
      return false;
   }else{
      LOGINFO("Data for " << stat.size()
              << " station is read from table <station>");
   }

   lastStationCheck=miTime::nowTime();

   int earlyobs = 20;
   int lateobs  = 30;

   if(gate.select(synoptypes,"where typeid=1"))
      if (!synoptypes.empty()) {
         earlyobs = synoptypes.begin()->earlyobs();
         lateobs  = synoptypes.begin()->lateobs();
      }

   return synopDec.initialise(stat,earlyobs,lateobs);
}

void
SynopDecoder::
writeObsToLog(const std::string &obs)
{
   string path;
   string logpath("var/log/decoders/"+name());

   char *p=getenv("KVALOBS");

   if(!p)
      return;

   path=p;

   if(path.empty())
      return;

   while(!path.empty() && path[path.length()-1]=='/')
      path.erase(path.length()-1);

   if(path.empty())
      return;

   if(!dnmi::file::mkdir(logpath, path))
      return;

   ofstream of;
   miTime now(miTime::nowTime());
   char tb[32];

   sprintf(tb, "%04d-%02d-%02d.raw", now.year(), now.month(), now.day());

   string logfile=path+"/"+logpath+"/"+tb;

   of.open(logfile.c_str(), ios::out|ios::app);

   if(!of.is_open())
      return;

   of << "[received: " << now << endl
         << " obstype:  " << obsType << endl
         << obs << "]" << endl;

   of.close();
}

kvalobs::decoder::DecoderBase::DecodeResult 
SynopDecoder::execute(miutil::miString &msg)
{
   kvalobs::kvStation       tstat;

   kvalobs::kvRejectdecode  reject;
   bool                     saveReject;
   std::string              saveRejectMessage;
   list<kvalobs::kvData>    data;
   miTime                   nowTime(miTime::nowTime());

   Lock lock(mutex);

   milog::LogContext lcontext("SynopDecoder");
   LOGINFO("New observation(s)");

   if(obs.length()==0){
      LOGERROR("Incomming message has zero size!");
      return Ok;
   }

   writeObsToLog(obs);

   if(lastStationCheck.undef() ||
         abs(miTime::hourDiff(lastStationCheck, nowTime))>3){
      LOGINFO("Initialize the station information from the database.");

      if(!initializeKvSynopDecoder()){
         LOGERROR("Can't initialize the SynopDecoder!!!");
         msg="Can't initialize the SynopDecoder!!!";
         return NotSaved;
      }
   }

   if(synopDec.decode(obs, data)){
      LOGINFO("SUCCESS: Synop decoded!");

      try{
         if(!saveData(data, saveReject, saveRejectMessage)){
            if(saveReject){
               LOGERROR("REJECTETED, Can't save synop data. ["
                     << saveRejectMessage<< "]");
               msg=saveRejectMessage;

               if(!putRejectdecodeInDb(kvRejectdecode(obs,
                                                      nowTime,
                                                      "synop",
                                                      saveRejectMessage))){
                  LOGDEBUG("Cant save rejected data in database!");
               }

               return Rejected;
            }else{
               msg="Can't save data to database!";
               return NotSaved;
            }
         }
      }
      catch(std::exception & ex) {
         LOGERROR("EXCEPTION: " << ex.what() << endl);
         msg="EXCEPTION: Can't save data to database!";
         return NotSaved;
      }


      if(milog::Logger::logger().logLevel()>=milog::DEBUG){
         ostringstream ost;
         list<kvalobs::kvData>::iterator itr=data.begin();

         for(;itr!=data.end();itr++)
            ost << itr->toSend()  << endl;

         LOGDEBUG(ost.str());
      }


      if(synopDec.tmpStation(tstat)){
         // unknown SHIPS: create temporary station

         if(milog::Logger::logger().logLevel()>=milog::DEBUG){
            LOGDEBUG("New station:------\n" << tstat.toSend() << endl);
         }

         if(!putkvStationInDb(tstat)){
            LOGDEBUG("Cant save station data in database!");
         }
      }

   }else{
      LOGWARN("Synop REJECTED: invalid format!\n");
      reject = synopDec.rejected("synop");

      LOGDEBUG("Rejected: ------------\n" << reject.toSend() << endl);
      msg="Synop REJECTED: " + reject.comment();

      /**COMMENT:
      *As long as we dont have a solution for what to
      *do with foreign stations, or we dont have foreign
      *stations in the station table dont less us eat up
      *the disk with rejected 'unknown' stations.
      *
      *This is only a quick hack.
      */

      if(reject.comment()!="unknown station/position"){
         if(!putRejectdecodeInDb(reject)){
            LOGDEBUG("Cant save rejected data in database!");
         }
      }

      return Rejected;
   }

   msg="OK!";

   LOGINFO("SUCCESS:Observation(s) decoded and saved!");

   return Ok;
}
