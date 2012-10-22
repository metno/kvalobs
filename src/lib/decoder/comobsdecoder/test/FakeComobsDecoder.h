/*
 * FakeComobsDecoder.h
 *
 *  Created on: Dec 22, 2010
 *      Author: borgem
 */

#ifndef __FAKECOMOBSDECODER_H__
#define __FAKECOMOBSDECODER_H__

#include <decoder/decoderbase/decoder.h>
#include "../smsbase.h"
#include "../comobsdecode.h"



class FakeComobsDecoder : public kvalobs::decoder::comobsdecoder::ComObsDecoder
{
   std::string saSdEm;
public:
   FakeComobsDecoder(dnmi::db::Connection     &con,
                     const ParamList        &params,
                     const std::list<kvalobs::kvTypes> &typeList,
                     const std::string &obsType,
                     const std::string &obs,
                     int                    decoderId=-1);

   kvalobs::decoder::comobsdecoder::SmsBase *smsfactory(int smscode);
   long   getStationid(long obsid, bool isWmono);

   void setSaSdEm(const std::string saSdEm_ ){ saSdEm = saSdEm_; }
   std::string getMetaSaSdEm( int stationid, int typeid_, const miutil::miTime &obstime );

   virtual std::string name()const;

   virtual kvalobs::decoder::DecoderBase::DecodeResult execute(std::string &msg);

};


#endif
