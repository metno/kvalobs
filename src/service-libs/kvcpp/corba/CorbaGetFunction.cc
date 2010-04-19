/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: CorbaGetFunction.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $

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
#include "CorbaGetFunction.h"
#include "CorbaKvApp.h"
#include <milog/milog.h>

using namespace std;
using namespace milog;
using namespace miutil;
using namespace CKvalObs::CService;
using namespace kvalobs;

namespace kvservice
{
namespace corba
{
namespace priv
{
CorbaKvApp *CorbaGetFunction::corbaApp = NULL;

bool CorbaGetFunction::operator()(const char *name)
{
	CKvalObs::CService::kvService_var service;
	bool forceNS = false;
	bool usedNS;

	LogContext context(name);

	if (corbaApp->shutdown())
	{
		LOGERROR("The CORBA subsystem is shutdown!!!");
		return false;
	}

	try
	{
		for (int i = 0; i < 2; i++)
		{
			service = corbaApp->lookUpManager(forceNS, usedNS);
			try
			{
				bool result = this->process(service);
				if (!result)
				{
					LOGWARN("Can't get data from kvalobs!!");
				}
				else
				{
					return result;
				}
			} catch (CORBA::TRANSIENT &ex)
			{
				LOGWARN("Exception CORBA::TRANSIENT!");
			} catch (CORBA::COMM_FAILURE &ex)
			{
				LOGWARN( "Exception CORBA::COMM_FAILURE!");
			} catch (...)
			{
				LOGERROR("Exception unknown!");
				return false;
			}
			if (usedNS)
			{
				LOGERROR("Can't connect to kvalobs!");
				return false;
			}
			forceNS = true;
		}
	} catch (LookUpException &ex)
	{
		LOGERROR("Can't lookup CORBA nameserver" << endl <<
				"Reason: " << ex.what() << endl);
		return false;
	} catch (...)
	{
		LOGERROR("Can't lookup CORBA nameserver" <<
				"Reason:  unknown!");
		return false;
	}
	LOGERROR("Cannot perform operation");
	return false;
}

getKvRejectDecodeFunc::getKvRejectDecodeFunc(
		const RejectDecodeInfo &decodeInfo, RejectDecodeIterator &it) :
	decodeInfo(decodeInfo), it(it)
{
}

bool getKvRejectDecodeFunc::process(kvService_ptr service)
{
	it.cleanup();
	return service->getRejectdecode(decodeInfo, it.getCorbaObjPtr().out());
}

getKvParamsFunc::getKvParamsFunc(list<kvParam> &paramList) :
	paramList(paramList)
{
}

bool getKvParamsFunc::process(kvService_ptr service)
{
	ParamList *params;
	bool ok = service->getParams(params);
	if (ok)
	{
		paramList.clear();
		for (CORBA::ULong i = 0; i < params->length(); i++)
		{
			paramList.push_back(kvParam((*params)[i].paramID, miString(
					(*params)[i].name), miString((*params)[i].description),
					miString((*params)[i].unit), (*params)[i].level_scale,
					miString((*params)[i].comment)));
		}
	}
	return ok;
}

getKvStationsFunc::getKvStationsFunc(list<kvStation> &stationList) :
	stationList(stationList)
{
}

bool getKvStationsFunc::process(kvService_ptr service)
{
	CKvalObs::CService::StationList *stations;
	bool ok = service->getStations(stations);
	if (ok)
	{
		stationList.clear();
		for (CORBA::ULong i = 0; i < stations->length(); i++)
			stationList.push_back(kvStation((*stations)[i].stationID,
					(*stations)[i].lat, (*stations)[i].lon,
					(*stations)[i].height, (*stations)[i].maxspeed, miString(
							(*stations)[i].name), (*stations)[i].wmonr,
					(*stations)[i].nationalnr, miString((*stations)[i].ICAOid),
					miString((*stations)[i].call_sign), miString(
							(*stations)[i].stationstr),
					(*stations)[i].environmentid, (*stations)[i].static_,
					miTime(miString((*stations)[i].fromtime))));
	}
	return ok;
}

getKvModelDataFunc::getKvModelDataFunc(list<kvModelData> &dataList,
		const WhichDataHelper &wd) :
	dataList(dataList), wd(wd)
{
}

bool getKvModelDataFunc::process(kvService_ptr service)
{
	ModelDataIterator_var it;
	ModelDataList_var data;

	bool ok = service->getModelData(*wd.whichData(), it);

	if (ok)
	{
		dataList.clear();
		while (it->next(data))
		{
			for (CORBA::ULong i = 0; i < data->length(); i++)
			{
				for (CORBA::ULong k = 0; k < data[i].dataList.length(); k++)
				{
					kvalobs::kvModelData myData(data[i].dataList[k].stationID,
							miutil::miTime(data[i].dataList[k].obstime),
							data[i].dataList[k].paramID,
							data[i].dataList[k].level,
							data[i].dataList[k].modelID,
							data[i].dataList[k].original);
					dataList.push_back(myData);
				}
			}
		}
		try
		{
			it->destroy();
		} catch (...)
		{
			LOGERROR("Can't destroy iterator!");
		}
	}

	return ok;
}

getKvReferenceStationsFunc::getKvReferenceStationsFunc(int stationid,
		int paramid, list<kvReferenceStation> &refList) :
	stationid(stationid), paramid(paramid), refList(refList)
{
}

bool getKvReferenceStationsFunc::process(kvService_ptr service)
{
	Reference_stationList *ref_;
	bool ok = service->getReferenceStation(stationid, paramid, ref_);
	if (ok)
	{
		refList.clear();
		for (CORBA::ULong i = 0; i < ref_->length(); i++)
			refList.push_back(kvReferenceStation((*ref_)[i].stationID,
					(*ref_)[i].paramsetID, miString((*ref_)[i].reference)));
	}
	return ok;
}

getKvTypesFunc::getKvTypesFunc(list<kvTypes> &typeList) :
	typeList(typeList)
{
}

bool getKvTypesFunc::process(kvService_ptr service)
{
	CKvalObs::CService::TypeList *types;
	bool ok = service->getTypes(types);
	if (ok)
	{
		typeList.clear();
		for (CORBA::ULong i = 0; i < types->length(); i++)
		{
			kvTypes type((*types)[i].typeID_, miString((*types)[i].format),
					(*types)[i].earlyobs, (*types)[i].lateobs, miString(
							(*types)[i].read), miString((*types)[i].obspgm),
					miString((*types)[i].comment));
			typeList.push_back(type);
		}
	}
	return ok;
}

getKvOperatorFunc::getKvOperatorFunc(list<kvOperator> &operatorList) :
	operatorList(operatorList)
{
}

bool getKvOperatorFunc::process(kvService_ptr service)
{
	CKvalObs::CService::OperatorList *operators;
	bool ok = service->getOperator(operators);
	if (ok)
	{
		operatorList.clear();
		for (CORBA::ULong i = 0; i < operators->length(); i++)
		{
			miString name((*operators)[i].username);
			kvOperator oper(name, (*operators)[i].userid);
			operatorList.push_back(oper);
		}
	}
	return ok;
}

getKvStationParamFunc::getKvStationParamFunc(list<kvStationParam> &stParam,
		int stationid, int paramid, int day) :
	stParam(stParam), stationid(stationid), paramid(paramid), day(day)
{
}

bool getKvStationParamFunc::process(kvService_ptr service)
{
	CKvalObs::CService::Station_paramList *stp;
	bool ok = service->getStationParam(stp, stationid, paramid, day);
	if (ok)
	{
		stParam.clear();
		for (CORBA::ULong i = 0; i < stp->length(); i++)
		{
			kvStationParam param((*stp)[i].stationid, (*stp)[i].paramid,
					(*stp)[i].level, ((char*) (*stp)[i].sensor)[0] - '0',
					(*stp)[i].fromday, (*stp)[i].today, (*stp)[i].hour,
					miString((*stp)[i].qcx), miString((*stp)[i].metadata),
					miString((*stp)[i].desc_metadata), miTime(
							(*stp)[i].fromtime));
			stParam.push_back(param);
		}
	}
	return ok;
}


getKvStationMetaDataFunc::getKvStationMetaDataFunc(list<kvStationMetadata> &stParam,
		int stationid, const miutil::miTime &obstime, const std::string & metadataName) :
	stParam(stParam), stationid(stationid), obstime(obstime), metadataName_(metadataName)
{
}

bool getKvStationMetaDataFunc::process(kvService_ptr service)
{
	CKvalObs::CService::kvServiceExt_var serviceext;

	serviceext = CKvalObs::CService::kvServiceExt::_narrow( service );

	if( CORBA::is_nil( serviceext ) ) {
		LogContext context("getKvStationMetaData");
		LOGERROR("getKvStationMetaData: Is not supported by the server.");
		return false;
	}
	CKvalObs::CService::Station_metadataList *metadata;
	string myObstime;

	if( ! obstime.undef() )
	   myObstime = obstime.isoTime();

	bool ok = serviceext->getStationMetaData( metadata, stationid, myObstime.c_str(), metadataName_.c_str() );

	if (ok )
	{
		int param_;
		int type_;
		int level_;
		int sensor_;

		int *paramPtr;
		int *typePtr;
		int *levelPtr;
		int *sensorPtr;

		stParam.clear();
		for (CORBA::ULong i = 0; i < metadata->length(); i++)
		{
			param_ = (*metadata)[i].paramid;
			type_ = (*metadata)[i].typeID_;
			level_ = (*metadata)[i].level;
			sensor_ = (*metadata)[i].sensor;

//			paramPtr  = param_ == kvDbBase::INT_NULL?0:&param_;
//			typePtr   = type_  == kvDbBase::INT_NULL?0:&type_;
//			levelPtr  = level_ == kvDbBase::INT_NULL?0:&level_;
//			sensorPtr = sensor_== kvDbBase::INT_NULL?0:&sensor_;

			paramPtr  = param_ == -32767?0:&param_;
			typePtr   = type_  == -32767?0:&type_;
			levelPtr  = level_ == -32767?0:&level_;
			sensorPtr = sensor_== -32767?0:&sensor_;


			kvStationMetadata meta( (*metadata)[i].stationid, paramPtr, typePtr, levelPtr, sensorPtr,
									 string( (*metadata)[i].metadatatypename ),
									 (*metadata)[i].metadata,
									 string( (*metadata)[i].metadataDescription ),
									 miTime( (*metadata)[i].fromtime ),
									 ( strlen( (*metadata)[i].totime ) == 0 ? miTime():miTime( (*metadata)[i].totime ) ) );
			stParam.push_back( meta );
		}
	}
	return ok;
}


getKvObsPgmFunc::getKvObsPgmFunc(list<kvObsPgm> &obsPgm,
		const list<long> &stationList, bool aUnion) :
	obsPgmList(obsPgm), stationList(stationList), aUnion(aUnion)
{
}

bool getKvObsPgmFunc::process(kvService_ptr service)
{
	CKvalObs::CService::Obs_pgmList *obspgm;
	CKvalObs::CService::StationIDList idList;
	idList.length(stationList.size());

	list<long>::const_iterator it = stationList.begin();
	for (CORBA::ULong i = 0; it != stationList.end(); it++)
		idList[i++] = *it;

	bool ok = service->getObsPgm(obspgm, idList, aUnion);
	if (ok)
	{
		obsPgmList.clear();
		for (CORBA::ULong i = 0; i < obspgm->length(); i++)
		{
			kvObsPgm obsp((*obspgm)[i].stationID, (*obspgm)[i].paramID,
					(*obspgm)[i].level, (*obspgm)[i].nr_sensor,
					(*obspgm)[i].typeID_, (*obspgm)[i].collector,
					(*obspgm)[i].kl00, (*obspgm)[i].kl01, (*obspgm)[i].kl02,
					(*obspgm)[i].kl03, (*obspgm)[i].kl04, (*obspgm)[i].kl05,
					(*obspgm)[i].kl06, (*obspgm)[i].kl07, (*obspgm)[i].kl08,
					(*obspgm)[i].kl09, (*obspgm)[i].kl10, (*obspgm)[i].kl11,
					(*obspgm)[i].kl12, (*obspgm)[i].kl13, (*obspgm)[i].kl14,
					(*obspgm)[i].kl15, (*obspgm)[i].kl16, (*obspgm)[i].kl17,
					(*obspgm)[i].kl18, (*obspgm)[i].kl19, (*obspgm)[i].kl20,
					(*obspgm)[i].kl21, (*obspgm)[i].kl22, (*obspgm)[i].kl23,
					(*obspgm)[i].mon, (*obspgm)[i].tue, (*obspgm)[i].wed,
					(*obspgm)[i].thu, (*obspgm)[i].fri, (*obspgm)[i].sat,
					(*obspgm)[i].sun,
					// UNDEF:
					miutil::miTime((*obspgm)[i].fromtime), miutil::miTime(
							(*obspgm)[i].totime));
			obsPgmList.push_back(obsp);
		}
	}
	return ok;
}

getKvDataFunc_abstract::getKvDataFunc_abstract(termfunc terminate) :
	terminate(terminate)
{
}

DataIterator_ptr getKvDataFunc_abstract::getDataIter(const WhichDataHelper &wd)
{
	DataIterator_ptr it;
	kvService_ptr service;
	bool forceNS = false;
	bool usedNS;

	LogContext context("getDataIter");

	try
	{
		for (int i = 0; i < 2; i++)
		{
			service = corbaApp->lookUpManager(forceNS, usedNS);
			try
			{
				if (!service->getData(*wd.whichData(), it))
				{
					LOGWARN("Can't get data from kvalobs!!!!");
					return DataIterator::_nil();
				}
				else
				{
					return it;
				}
			} catch (CORBA::TRANSIENT &ex)
			{
				LOGWARN("Exception CORBA::TRANSIENT!");
			} catch (CORBA::COMM_FAILURE &ex)
			{
				LOGWARN("Exception CORBA::COMM_FAILURE!");
			} catch (...)
			{
				LOGWARN("Exception unknown!");
				return DataIterator::_nil();
			}

			if (usedNS)
			{
				LOGWARN("Can't connect to kvalobs!");
				return DataIterator::_nil();
			}
			forceNS = true;
		}
	} catch (LookUpException &ex)
	{
		LOGINFO("Can't lookup CORBA nameserver"
				"Reason: " << ex.what() << endl);
		return DataIterator::_nil();
	} catch (...)
	{
		LOGINFO("Can't lookup CORBA nameserver" <<
				"Reason: Unkown!");
		return DataIterator::_nil();
	}
	return it;
}

getKvDataFunc::getKvDataFunc(KvGetDataReceiver &dataReceiver,
		const WhichDataHelper &wd, termfunc terminate) :
	getKvDataFunc_abstract(terminate), dataReceiver(dataReceiver), wd(wd)
{
}

bool getKvDataFunc::process(kvService_ptr service)
{
	DataIterator_var dataIt = getDataIter(wd);
	ObsDataList_var data;

	bool err = false;
	KvObsDataList dataList;
	bool cont = true;

	if (CORBA::is_nil(dataIt))
		return false;

	try
	{
		while (cont and not terminate() and dataIt->next(data))
		{
			for (CORBA::ULong i = 0; i < data->length(); i++)
				dataList.push_back(KvObsData(data[i]));
			cont = dataReceiver.next(dataList);
			dataList.clear();
		}
	} catch (...)
	{
		err = true;
	}

	try
	{
		dataIt->destroy();
	} catch (...)
	{
		LOGERROR("Can't destroy DataIterator!!!!");
	}
	return !err;
}

getKvDataFunc_deprecated::getKvDataFunc_deprecated(KvObsDataList &dataList,
		const WhichDataHelper &wd, termfunc terminate) :
	getKvDataFunc_abstract(terminate), dataList(dataList), wd(wd)
{
}

bool getKvDataFunc_deprecated::process(kvService_ptr service)
{
	DataIterator_var dataIt = getDataIter(wd);
	ObsDataList_var data;
	KvDataList tmpDataList;
	int sensor;
	bool err = false;

	if (CORBA::is_nil(dataIt))
		return false;

	dataList.clear();

	try
	{
		while (dataIt->next(data) and not terminate())
			for (CORBA::ULong i = 0; i < data->length(); i++)
				dataList.push_back(KvObsData(data[i]));
	} catch (std::exception & e)
	{
		std::cout << e.what() << std::endl;
		err = true;
	} catch (...)
	{
		std::cout << "Unknown error" << std::endl;
		err = true;
	}

	try
	{
		dataIt->destroy();
	} catch (...)
	{
		LOGERROR("Can't destroy DataIterator!!!!");
	}
	return not err;
}
}
}
}
