

#include "FakeComobsDecoder.h"

FakeComobsDecoder::
FakeComobsDecoder(dnmi::db::Connection     &con,
                  const ParamList        &params,
                  const std::list<kvalobs::kvTypes> &typeList,
                  const miutil::miString &obsType,
                  const miutil::miString &obs,
                  int                    decoderId )
   : kvalobs::decoder::comobsdecoder::ComObsDecoder( con, params, typeList, obsType, obs, decoderId)
{

}



kvalobs::decoder::comobsdecoder::SmsBase*
FakeComobsDecoder::
smsfactory(int smscode)
{
   return kvalobs::decoder::comobsdecoder::ComObsDecoder::smsfactory( smscode );
}

long
FakeComobsDecoder::
getStationid(long obsid, bool isWmono)
{
   return obsid;
}

std::string
FakeComobsDecoder::
getMetaSaSdEm( int stationid, int typeid_, const miutil::miTime &obstime )
{
   if( saSdEm.empty() )
      return "111";

   return saSdEm;
}

miutil::miString
FakeComobsDecoder::
name()const
{
   return "Fake";
}

kvalobs::decoder::DecoderBase::DecodeResult
FakeComobsDecoder::
execute(miutil::miString &msg)
{
  return kvalobs::decoder::DecoderBase::Ok;
}



