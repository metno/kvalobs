/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: comobsdecode.cc,v 1.7.2.6 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <stdlib.h>
#include <sstream>
#include <milog/milog.h>
#include <puTools/miTime.h>
#include <miutil/trimstr.h>
#include <kvalobs/kvDbGate.h>
#include <decoder/decoderbase/DataUpdateTransaction.h>
#include "smsmeldingparser.h"
#include "comobsdecode.h"
#include "sms2.h"
#include "sms12.h"

using namespace std;
using namespace kvalobs; 
using namespace miutil;
using namespace kvalobs::decodeutil;

kvalobs::decoder::comobsdecoder::
ComObsDecoder::
ComObsDecoder(
      dnmi::db::Connection   &con,
      const ParamList        &params,
      const std::list<kvalobs::kvTypes> &typeList,
      const miutil::miString &obsType,
      const miutil::miString &obs,
      int                    decoderId)
:DecoderBase(con, params, typeList, obsType, obs, decoderId)
{
}

kvalobs::decoder::comobsdecoder::
ComObsDecoder::
~ComObsDecoder()
{
}

miutil::miString 
kvalobs::decoder::comobsdecoder::
ComObsDecoder::
name() const
{
   return "ComObsDecoder";
}


kvalobs::decoder::comobsdecoder::SmsBase*
kvalobs::decoder::comobsdecoder::
ComObsDecoder::
smsfactory(int smscode) 
{
   try{
      if(smscode==2)
         return new Sms2(paramList, *this);
#if 0
      if(smscode==12)
         return new Sms12(paramList, *this);
#endif

   }
   catch(...){
      LOGERROR("NOMEM: smsfactory, cant allocate an sms decoder for smscode: " <<
               smscode << endl);
      return 0;
   }

   return 0;
}

long   
kvalobs::decoder::comobsdecoder::
ComObsDecoder::
getStationid(long obsid, bool isWmono)
{
   ostringstream ost;
   string key;
   string val;

   if(isWmono)
      key="wmonr";
   else
      key="nationalnr";

   ost << obsid;
   val=ost.str();

   return DecoderBase::getStationId(key, val);
}

std::string 
kvalobs::decoder::comobsdecoder::
ComObsDecoder::
getMetaSaSdEm( int stationid, int typeid_, const miutil::miTime &obstime )
{
   string sasdem="000";

   if( obsPgm.obstime.undef() || obsPgm.obstime != obstime ){
      if( ! loadObsPgmParamInfo( stationid, typeid_, obstime, obsPgm ) ){
         return "000";
         LOGDEBUG("DBERROR: SaSdEm:  000");
      }
   }

   kvalobs::decoder::Active state;

   if( obsPgm.isActive( stationid, typeid_, 112, 0, 0, obstime, state ) ) {
      if( state == kvalobs::decoder::YES )
         sasdem[0]='1';
   }

   if( obsPgm.isActive( stationid, typeid_, 18, 0, 0, obstime, state ) ) {
      if( state == kvalobs::decoder::YES )
         sasdem[1]='1';
   }

   if( obsPgm.isActive( stationid, typeid_, 7, 0, 0, obstime, state ) ) {
      if( state == kvalobs::decoder::YES )
         sasdem[2]='1';
   }

   LOGDEBUG("SaSdEm: " << sasdem);

   return sasdem;
}


kvalobs::decoder::DecoderBase::DecodeResult 
kvalobs::decoder::comobsdecoder::
ComObsDecoder::
execute(miutil::miString &msg)
{
   SmsMeldingParser mParser;
   SmsMelding       *smsMelding;
   SmsBase          *decoder;
   long   obsid;
   bool   isWmono;
   long   stationid;
   bool   hasRejected;
   DecodedData *data;
   ostringstream logid;

   list<kvRejectdecode> rejected;
   milog::LogContext lcontext(name());

   LOGINFO("ComObsDecoder:  " << miutil::miTime::nowTime());

   smsMelding=mParser.parse(obs);

   if(!smsMelding){
      kvRejectdecode myrejected=kvRejectdecode(obs, miTime::nowTime(), "comobs/typeid=<UNKNOWN>",
                                               mParser.getErrMsg());


      putRejectdecodeInDb(myrejected);

      LOGWARN("Rejected!" << endl << mParser.getErrMsg() << endl);
      return Rejected;
   }

   if(smsMelding->getClimano()>0){
      stationid=getStationid(smsMelding->getClimano(), false);
   }else if(smsMelding->getSynopno()>0){
      stationid=getStationid(smsMelding->getSynopno(), true);
   }else{
      ostringstream ost;

      ost << "comobs/typeid=" << smsMelding->getCode() +300;

      kvRejectdecode myrejected=kvRejectdecode(obs,
                                               miTime::nowTime(),
                                               ost.str(),
                                               "No identification!");

      putRejectdecodeInDb(myrejected);

      LOGWARN("Rejected!" << endl <<"No identification!" << endl);
      return Rejected;
   }

   if(stationid<=0){
      ostringstream ost;

      ost << "comobs/typeid=" << smsMelding->getCode() +300;

      kvRejectdecode myrejected=kvRejectdecode(obs,
                                               miTime::nowTime(),
                                               ost.str(),
                                               "Uknown station!");


      putRejectdecodeInDb(myrejected);

      LOGWARN("Rejected!" << endl <<"Unknown station!" << endl);
      return Rejected;
   }


   decoder=smsfactory(smsMelding->getCode());

   logid << "n" << stationid << "-t" << smsMelding->getCode() + 300 << ".log";
   createLogger( logid.str() );

   if(!decoder){
      ostringstream ost;
      ostringstream ost1;

      ost1 << "comobs/typeid=" << smsMelding->getCode() +300;
      ost << "No decoder for SMS code <" << smsMelding->getCode() << ">!";

      kvRejectdecode myrejected=kvRejectdecode(obs, miTime::nowTime(), ost1.str(),
                                               ost.str());


      putRejectdecodeInDb(myrejected);

      LOGWARN("Rejected!" << endl << ost.str() << endl);
      IDLOGERROR( logid.str(), "Rejected!" << endl << ost.str() << endl );
      removeLogger( logid.str() );
      return Rejected;
   }
   decoder->logid = logid.str();

   data=decoder->decode(stationid,
                        smsMelding->getCode(),
                        smsMelding->getMeldingList(),
                        msg,
                        rejected,
                        hasRejected);

   if(!data){
      msg="No mem!";
      removeLogger( logid.str() );
      return Error;
   }


   if(hasRejected){
      list<kvRejectdecode>::iterator it=rejected.begin();

      for(;it!=rejected.end(); it++)
         putRejectdecodeInDb(*it);

      LOGWARN("Rejected!" << endl << msg << endl);
      removeLogger( logid.str() );
      return Rejected;
   }


   int nData=0;
   TDecodedDataElem *theData=data->data();
   bool         error=false;

   for(CITDecodedDataElem it=theData->begin();
         it!=theData->end();
         it++){
      list<kvData>      d=it->data();
      list<kvTextData> td=it->textData();
      int priority;

      if(smsMelding->getCode()==2 ||
            smsMelding->getCode()==13 ||
            smsMelding->getCode()==3)
         priority=8;
      else
         priority=4;

      LOGDEBUG3(*it);

      if( it->dataSize()>0 || it->textDataSize()>0 ) {
         try {
            kvalobs::decoder::DataUpdateTransaction work( it->getDate(),
                                                          it->stationID(), it->typeID(),
                                                          priority, &d, &td, logid.str() );

            con.perform( work );
            updateStationInfo( work.stationInfoList() );
            nData += it->dataSize() + it->textDataSize();
         }
         catch( const dnmi::db::SQLException &ex ) {
            LOGERROR("Failed to save data: Reason: " << ex.what() );
         }
         catch( const std::exception &ex ) {
            LOGERROR("Failed to save data: Reason: " << ex.what() << "(*)" );
         }
         catch( ... ) {
            LOGERROR("Failed to save data. Reason: Unknown.");
         }
      }
   }

   if(error){
      if(nData>0){
         LOGERROR("Not all data was saved to the data base!" << endl <<
                  "#saved: " << nData);

         msg="Not all data could be saved to the database!";
      }else{
         msg="Can't save data to the database!";
      }
      removeLogger( logid.str() );
      return NotSaved;
   }

   removeLogger( logid.str() );
   LOGDEBUG("Saved " << nData << " data element to the database!");

   return Ok;

}


