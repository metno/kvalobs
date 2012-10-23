

#include "FakeComobsDecoder.h"

FakeComobsDecoder::
FakeComobsDecoder(dnmi::db::Connection     &con,
                  const ParamList        &params,
                  const std::list<kvalobs::kvTypes> &typeList,
                  const std::string &obsType,
                  const std::string &obs,
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

std::string
FakeComobsDecoder::
name()const
{
   return "Fake";
}

kvalobs::decoder::DecoderBase::DecodeResult
FakeComobsDecoder::
execute(std::string &msg)
{
  return kvalobs::decoder::DecoderBase::Ok;
}



