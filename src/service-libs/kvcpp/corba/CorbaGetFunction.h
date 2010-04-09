/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: CorbaGetFunction.h,v 1.3.2.4 2007/09/27 09:02:45 paule Exp $

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
#ifndef __kvservice__corba__priv__CorbaGetFunction_h__
#define __kvservice__corba__priv__CorbaGetFunction_h__

#include "../WhichDataHelper.h"
#include <kvskel/kvService.hh>
#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvReferenceStation.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvOperator.h>
#include "../RejectdecodeIterator.h"
#include "../KvGetDataReceiver.h"

namespace kvservice
{
/**
 * \addtogroup kvcpp
 * @{
 */
namespace corba
{
/**
 * \addtogroup corba
 * @{
 */
class CorbaKvApp;

namespace priv
{
/**
 * \brief An abstract functor, used by CorbaKvApp, for fetching data from kvalobs.
 *
 * Subclasses of CorbaGetFunction does the actual implementation of
 * fetching data from kvalobs. This class does error handling related to
 * communication with CORBA.
 *
 * If you wish to create a new function for KvApp to use, you should (in
 * addition to making the relevant changes to KvApp and CorbaKvApp)
 * create a subclass of CorbaKvApp, and then implement the protected
 * <code> process </code> function, to get data from kvalobs. No change
 * to <code> operator() </code> should be neccessary.
 */
class CorbaGetFunction
{
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service) =0;
public:
	static CorbaKvApp *corbaApp;

	virtual ~CorbaGetFunction()
	{
	}

	/**
	 * \brief Call <code> process </code> and handle all CORBA-specific errors related to the call.
	 *
	 * \param name The name of the called method, to be displayed in error
	 * messages.
	 * \return True on success, false otherwise
	 */
	virtual bool operator()(const char *name);
};

class getKvRejectDecodeFunc: public CorbaGetFunction
{
	const CKvalObs::CService::RejectDecodeInfo &decodeInfo;
	kvservice::RejectDecodeIterator &it;
public:
	getKvRejectDecodeFunc(
			const CKvalObs::CService::RejectDecodeInfo &decodeInfo,
			kvservice::RejectDecodeIterator &it);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvParamsFunc: public CorbaGetFunction
{
	std::list<kvalobs::kvParam> &paramList;
public:
	getKvParamsFunc(std::list<kvalobs::kvParam> &paramList);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvStationsFunc: public CorbaGetFunction
{
	std::list<kvalobs::kvStation> &stationList;
public:
	getKvStationsFunc(std::list<kvalobs::kvStation> &stationList);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvModelDataFunc: public CorbaGetFunction
{
	std::list<kvalobs::kvModelData> &dataList;
	const kvservice::WhichDataHelper &wd;
public:
	getKvModelDataFunc(std::list<kvalobs::kvModelData> &dataList,
			const kvservice::WhichDataHelper &wd);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvReferenceStationsFunc: public CorbaGetFunction
{
	int stationid;
	int paramid;
	std::list<kvalobs::kvReferenceStation> &refList;
public:
	getKvReferenceStationsFunc(int stationid, int paramid, std::list<
			kvalobs::kvReferenceStation> &refList);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvTypesFunc: public CorbaGetFunction
{
	std::list<kvalobs::kvTypes> &typeList;
public:
	getKvTypesFunc(std::list<kvalobs::kvTypes> &typeList);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvOperatorFunc: public CorbaGetFunction
{
	std::list<kvalobs::kvOperator> &operatorList;
public:
	getKvOperatorFunc(std::list<kvalobs::kvOperator> &operatorList);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvStationParamFunc: public CorbaGetFunction
{
	std::list<kvalobs::kvStationParam> &stParam;
	int stationid;
	int paramid;
	int day;
public:
	getKvStationParamFunc(std::list<kvalobs::kvStationParam> &stParam,
			int stationid, int paramid, int day);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvObsPgmFunc: public CorbaGetFunction
{
	std::list<kvalobs::kvObsPgm> &obsPgmList;
	const std::list<long> &stationList;
	bool aUnion;
public:
	getKvObsPgmFunc(std::list<kvalobs::kvObsPgm> &obsPgm,
			const std::list<long> &stationList, bool aUnion);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvDataFunc_abstract: public CorbaGetFunction
{
public:
	typedef bool (*termfunc)();

	getKvDataFunc_abstract(termfunc terminate);
protected:
	virtual CKvalObs::CService::DataIterator_ptr getDataIter(
			const WhichDataHelper &wd);
	termfunc terminate;
};

class getKvDataFunc: public getKvDataFunc_abstract
{
	kvservice::KvGetDataReceiver &dataReceiver;
	const WhichDataHelper &wd;
public:

	getKvDataFunc(KvGetDataReceiver &dataReceiver, const WhichDataHelper &wd,
			termfunc terminate);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};

class getKvDataFunc_deprecated: public getKvDataFunc_abstract
{
	kvservice::KvObsDataList &dataList;
	const WhichDataHelper &wd;
public:
	getKvDataFunc_deprecated(kvservice::KvObsDataList &dataList,
			const WhichDataHelper &wd, termfunc terminate);
protected:
	virtual bool process(CKvalObs::CService::kvService_ptr service);
};
}
}
}

#endif // __kvservice__corba__priv__CorbaGetFunction_h__
