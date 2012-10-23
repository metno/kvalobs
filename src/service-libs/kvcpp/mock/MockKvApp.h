/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: KvApp.h,v 1.3.2.4 2007/09/27 09:02:45 paule Exp $

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
#ifndef MOCKKVAPP_H_
#define MOCKKVAPP_H_

#include "FakeKvApp.h"
#include <gmock/gmock.h>

namespace testing
{

class MockKvApp : public kvservice::KvApp
{
public:
	MOCK_METHOD2(getKvData, bool( kvservice::KvGetDataReceiver &dataReceiver, const kvservice::WhichDataHelper &wd ));
	MOCK_METHOD2(getKvRejectDecode, bool( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, kvservice::RejectDecodeIterator &it ));
	MOCK_METHOD1(getKvParams, bool( std::list<kvalobs::kvParam> &paramList ));
	MOCK_METHOD1(getKvStations, bool( std::list<kvalobs::kvStation> &stationList ));
	MOCK_METHOD2(getKvModelData, bool( std::list<kvalobs::kvModelData> &dataList, const kvservice::WhichDataHelper &wd ));
	MOCK_METHOD3(getKvReferenceStations, bool( int stationid, int paramid, std::list<kvalobs::kvReferenceStation> &refList ));
	MOCK_METHOD1(getKvTypes, bool( std::list<kvalobs::kvTypes> &typeList ));
	MOCK_METHOD1(getKvOperator, bool( std::list<kvalobs::kvOperator> &operatorList ));
	MOCK_METHOD4(getKvStationParam, bool( std::list<kvalobs::kvStationParam> &stParam, int stationid, int paramid, int day ));
	MOCK_METHOD4(getKvStationMetaData, bool( std::list<kvalobs::kvStationMetadata> &stMeta, int stationid, const miutil::miTime &obstime, const std::string & metadataName ));
	MOCK_METHOD4(getKvWorkstatistik, bool(CKvalObs::CService::WorkstatistikTimeType timeType, const miutil::miTime &from, const miutil::miTime &to, kvservice::WorkstatistikIterator &it));

	MOCK_METHOD3(getKvObsPgm, bool( std::list<kvalobs::kvObsPgm> &obsPgm, const std::list<long> &stationList, bool aUnion ));
	MOCK_METHOD2(getKvData, bool( kvservice::KvObsDataList &dataList, const kvservice::WhichDataHelper &wd ));

	MOCK_METHOD2(sendDataToKv, const CKvalObs::CDataSource::Result_var( const char *data, const char *obsType ));

	MOCK_METHOD2(subscribeDataNotify, SubscriberID( const kvservice::KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que ));
	MOCK_METHOD2(subscribeData, SubscriberID( const kvservice::KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que ));
	MOCK_METHOD1(subscribeKvHint, SubscriberID( dnmi::thread::CommandQue &que ));
	MOCK_METHOD1(unsubscribe, void( const SubscriberID &subscriberid ));
	MOCK_METHOD0(unsubscribeAll, void());

	MOCK_CONST_METHOD0(shutdown, bool());
	MOCK_METHOD0(doShutdown, void());
	MOCK_METHOD0(run, void());

	void setupDefaultReturnValues()
	{
		ON_CALL(*this, getKvData(An<kvservice::KvGetDataReceiver &>(),_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvRejectDecode(_,_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvParams(_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvStations(_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvStations(_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvModelData(_,_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvReferenceStations(_,_,_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvTypes(_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvOperator(_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvStationParam(_,_,_,_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvStationMetaData(_,_,_,_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvWorkstatistik(_,_,_,_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvObsPgm(_,_,_)).WillByDefault(Return(true));
		ON_CALL(*this, getKvData(An<kvservice::KvObsDataList &>(),_)).WillByDefault(Return(true));

		ON_CALL(*this, sendDataToKv(_,_)).WillByDefault(Invoke(this, & MockKvApp::defaultImplementationSendDataToKv));

		ON_CALL(*this, subscribeDataNotify(_,_)).WillByDefault(Return("#id"));
		ON_CALL(*this, subscribeData(_,_)).WillByDefault(Return("#id"));
		ON_CALL(*this, subscribeKvHint(_)).WillByDefault(Return("#id"));
	}



	const CKvalObs::CDataSource::Result_var defaultImplementationSendDataToKv(const char *data, const char *obsType)
	{
		CKvalObs::CDataSource::Result_var ret(new CKvalObs::CDataSource::Result);
		ret->res = CKvalObs::CDataSource::OK;
		ret->message = "FakeKvApp default response";
		return ret;
	}

};

}

#endif /* MOCKKVAPP_H_ */
