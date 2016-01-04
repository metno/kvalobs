/*
 * MockComobsDecoder.h
 *
 *  Created on: Dec 21, 2010
 *      Author: borgem
 */

#ifndef __MOCKCOMOBSDECODER_H__
#define __MOCKCOMOBSDECODER_H__

#include <gmock/gmock.h>

#include "FakeComobsDecoder.h"

class MockComobsDecoder : public kvalobs::decoder::comobsdecoder::ComObsDecoder {
 public:
  MOCK_METHOD1( smsfactory, SmsBase * (int smscode) );
  MOCK_METHOD2(getStationid, long (long obsid, bool isWmono) );
  MOCK_METHOD3( getMetaSaSdEm, std::string ( int stationid, int typeid_, const miutil::miTime &obstime ) );
  MOCK_CONST_METHOD0( name(), std::string () );
  MOCK_METHOD1( execute, DecodeResult (std::string &msg) );
  void setDefaultActions()
  {
    using namespace testing;

    ON_CALL(*this, getChecks(_, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getChecks));
    ON_CALL(*this, getQcxFlagPosition(_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getQcxFlagPosition));
    ON_CALL(*this, getExpectedParameters(_, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getExpectedParameters));
    ON_CALL(*this, getAlgorithm(_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getAlgorithm));
    ON_CALL(*this, getStationParam(_,_,_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getStationParam));
    ON_CALL(*this, getModelData(_, _, _, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getModelData));
    ON_CALL(*this, getData(_, _, _, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getData));
    ON_CALL(*this, getTextData(_, _, _, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getTextData));
    ON_CALL(*this, write(_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::write));
  }

private:
  FakeComobsDecoder fake_;
};

#endif /* MOCKCOMOBSDECODER_H_ */
