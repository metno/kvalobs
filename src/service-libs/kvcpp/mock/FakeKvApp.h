/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2010 met.no

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

#ifndef FAKEKVAPP_H_
#define FAKEKVAPP_H_

#include "../KvApp.h"

namespace testing
{

class FakeKvApp : public kvservice::KvApp
{
public:
	virtual bool getKvData(kvservice::KvGetDataReceiver &dataReceiver,
			const kvservice::WhichDataHelper &wd)
	{
		return true;
	}

	virtual bool getKvRejectDecode(
			const CKvalObs::CService::RejectDecodeInfo &decodeInfo,
			kvservice::RejectDecodeIterator &it)
	{
		return true;
	}

	virtual bool getKvParams(std::list<kvalobs::kvParam> &paramList)
	{
		return true;
	}

	virtual bool getKvStations(std::list<kvalobs::kvStation> &stationList)
	{
		return true;
	}

	virtual bool getKvModelData(std::list<kvalobs::kvModelData> &dataList,
			const kvservice::WhichDataHelper &wd)
	{
		return true;
	}

	virtual bool getKvReferenceStations(int stationid, int paramid, std::list<
			kvalobs::kvReferenceStation> &refList)
	{
		return true;
	}

	virtual bool getKvTypes(std::list<kvalobs::kvTypes> &typeList)
	{
		return true;
	}

	virtual bool getKvOperator(std::list<kvalobs::kvOperator> &operatorList)
	{
		return true;
	}

	virtual bool getKvStationParam(std::list<kvalobs::kvStationParam> &stParam,
			int stationid, int paramid = -1, int day = -1)
	{
		return true;
	}

    virtual bool getKvStationMetaData( std::list<kvalobs::kvStationMetadata> &stMeta, int stationid, const boost::posix_time::ptime &obstime, const std::string & metadataName = "")
    {
    	return true;
    }

	virtual bool getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
			const std::list<long> &stationList, bool aUnion)
	{
		return true;
	}

	virtual bool getKvWorkstatistik(CKvalObs::CService::WorkstatistikTimeType timeType,
	                                    const boost::posix_time::ptime &from, const boost::posix_time::ptime &to,
	                                    kvservice::WorkstatistikIterator &it
	                                    )
	{
		return true;
	}

	virtual bool getKvData(kvservice::KvObsDataList &dataList, const kvservice::WhichDataHelper &wd)
	{
		return true;
	}

	virtual const CKvalObs::CDataSource::Result_var sendDataToKv(const char *data, const char *obsType)
	{
		CKvalObs::CDataSource::Result_var ret(new CKvalObs::CDataSource::Result);
		ret->res = CKvalObs::CDataSource::OK;
		ret->message = "FakeKvApp default response";
		return ret;
	}

	virtual SubscriberID subscribeDataNotify(
			const kvservice::KvDataSubscribeInfoHelper &info,
			dnmi::thread::CommandQue &que)
	{
		return "#id";
	}

	virtual SubscriberID subscribeData(const kvservice::KvDataSubscribeInfoHelper &info,
			dnmi::thread::CommandQue &que)
	{
		return "#id";
	}

	virtual SubscriberID subscribeKvHint(dnmi::thread::CommandQue &que)
	{
		return "#id";
	}

	virtual void unsubscribe(const SubscriberID &subscriberid)
	{}

	virtual void unsubscribeAll()
	{}

	virtual bool shutdown() const
	{
		return false;
	}

	virtual void doShutdown()
	{}

	virtual void run()
	{}
};

}

#endif /* FAKEKVAPP_H_ */
