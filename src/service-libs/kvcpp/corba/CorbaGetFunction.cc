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
	CKvalObs::CService::kvServiceExt_var service;
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
			service = corbaApp->lookUpService(forceNS, usedNS);
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

bool getKvRejectDecodeFunc::process(kvServiceExt_ptr service)
{
	it.cleanup();
	return service->getRejectdecode(decodeInfo, it.getCorbaObjPtr().out());
}


getKvWorkstatisticFunc::
getKvWorkstatisticFunc(
         const CKvalObs::CService::WorkstatistikTimeType timeType_,
         const boost::posix_time::ptime &from_,
         const boost::posix_time::ptime &to_,
         kvservice::WorkstatistikIterator &it_ )
   : timeType( timeType_ ), from( from_ ), to( to_ ),
     it( it_ )
{
}

bool
getKvWorkstatisticFunc::
process(CKvalObs::CService::kvServiceExt_ptr service_)
{
   CKvalObs::CService::kvServiceExt2_var service;

   it.cleanup();
   service = CKvalObs::CService::kvServiceExt2::_narrow( service_ );

   if( CORBA::is_nil( service ) ) {
      LogContext context("getKvStationMetaData");
      LOGERROR("getKvStationMetaData: Is not supported by the server.");
      return false;
   }

   //getWorkstatistik(in WorkstatistikTimeType timeType, in string fromTime, in string toTime, out WorkstatistikIterator it);
   return service->getWorkstatistik( timeType, to_simple_string(from).c_str(),
		   to_simple_string(to).c_str(), it.getCorbaObjPtr().out() );
};


getKvParamsFunc::getKvParamsFunc(list<kvParam> &paramList) :
	paramList(paramList)
{
}

bool getKvParamsFunc::process(kvServiceExt_ptr service)
{
	ParamList *params;
	bool ok = service->getParams(params);
	if (ok)
	{
		paramList.clear();
		for (CORBA::ULong i = 0; i < params->length(); i++)
		{
			paramList.push_back(kvParam((*params)[i].paramID, std::string(
					(*params)[i].name), std::string((*params)[i].description),
					std::string((*params)[i].unit), (*params)[i].level_scale,
					std::string((*params)[i].comment)));
		}
	}
	return ok;
}

getKvStationsFunc::getKvStationsFunc(list<kvStation> &stationList) :
	stationList(stationList)
{
}

bool getKvStationsFunc::process(kvServiceExt_ptr service)
{
	CKvalObs::CService::StationList *stations;
	bool ok = service->getStations(stations);
	if (ok)
	{
		stationList.clear();
		for (CORBA::ULong i = 0; i < stations->length(); i++)
			stationList.push_back(kvStation((*stations)[i].stationID,
					(*stations)[i].lat, (*stations)[i].lon,
					(*stations)[i].height, (*stations)[i].maxspeed, std::string(
							(*stations)[i].name), (*stations)[i].wmonr,
					(*stations)[i].nationalnr, std::string((*stations)[i].ICAOid),
					std::string((*stations)[i].call_sign), std::string(
							(*stations)[i].stationstr),
					(*stations)[i].environmentid, (*stations)[i].static_,
					boost::posix_time::time_from_string(std::string((*stations)[i].fromtime))));
	}
	return ok;
}

getKvModelDataFunc::getKvModelDataFunc(list<kvModelData> &dataList,
		const WhichDataHelper &wd) :
	dataList(dataList), wd(wd)
{
}

bool getKvModelDataFunc::process(kvServiceExt_ptr service)
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
							boost::posix_time::time_from_string((const char *) data[i].dataList[k].obstime),
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

bool getKvReferenceStationsFunc::process(kvServiceExt_ptr service)
{
	Reference_stationList *ref_;
	bool ok = service->getReferenceStation(stationid, paramid, ref_);
	if (ok)
	{
		refList.clear();
		for (CORBA::ULong i = 0; i < ref_->length(); i++)
			refList.push_back(kvReferenceStation((*ref_)[i].stationID,
					(*ref_)[i].paramsetID, std::string((*ref_)[i].reference)));
	}
	return ok;
}

getKvTypesFunc::getKvTypesFunc(list<kvTypes> &typeList) :
	typeList(typeList)
{
}

bool getKvTypesFunc::process(kvServiceExt_ptr service)
{
	CKvalObs::CService::TypeList *types;
	bool ok = service->getTypes(types);
	if (ok)
	{
		typeList.clear();
		for (CORBA::ULong i = 0; i < types->length(); i++)
		{
			kvTypes type((*types)[i].typeID_, std::string((*types)[i].format),
					(*types)[i].earlyobs, (*types)[i].lateobs, std::string(
							(*types)[i].read), std::string((*types)[i].obspgm),
					std::string((*types)[i].comment));
			typeList.push_back(type);
		}
	}
	return ok;
}

getKvOperatorFunc::getKvOperatorFunc(list<kvOperator> &operatorList) :
	operatorList(operatorList)
{
}

bool getKvOperatorFunc::process(kvServiceExt_ptr service)
{
	CKvalObs::CService::OperatorList *operators;
	bool ok = service->getOperator(operators);
	if (ok)
	{
		operatorList.clear();
		for (CORBA::ULong i = 0; i < operators->length(); i++)
		{
			std::string name((*operators)[i].username);
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

bool getKvStationParamFunc::process(kvServiceExt_ptr service)
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
					std::string((*stp)[i].qcx), std::string((*stp)[i].metadata),
					std::string((*stp)[i].desc_metadata),
					boost::posix_time::time_from_string(std::string((*stp)[i].fromtime)));
			stParam.push_back(param);
		}
	}
	return ok;
}


getKvStationMetaDataFunc::getKvStationMetaDataFunc(list<kvStationMetadata> &stParam,
		int stationid, const boost::posix_time::ptime &obstime, const std::string & metadataName) :
	stParam(stParam), stationid(stationid), obstime(obstime), metadataName_(metadataName)
{
}

bool getKvStationMetaDataFunc::process(kvServiceExt_ptr serviceext)
{
   CKvalObs::CService::Station_metadataList *metadata;
	string myObstime;

	if( ! obstime.is_not_a_date_time() )
	   myObstime = to_simple_string(obstime);

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
									 boost::posix_time::time_from_string( std::string((*metadata)[i].fromtime) ),
									 ( strlen( (*metadata)[i].totime ) == 0 ?
											 boost::posix_time::ptime() :
											 boost::posix_time::time_from_string( std::string( (*metadata)[i].totime ) ) ));
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

bool getKvObsPgmFunc::process(kvServiceExt_ptr service)
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
					boost::posix_time::time_from_string((const char *) (*obspgm)[i].fromtime),
					boost::posix_time::time_from_string((const char *) (*obspgm)[i].totime));
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
	kvServiceExt_ptr service;
	bool forceNS = false;
	bool usedNS;

	LogContext context("getDataIter");

	try
	{
		for (int i = 0; i < 2; i++)
		{
			service = corbaApp->lookUpService(forceNS, usedNS);
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

bool getKvDataFunc::process(kvServiceExt_ptr service)
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

bool getKvDataFunc_deprecated::process(kvServiceExt_ptr service)
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
