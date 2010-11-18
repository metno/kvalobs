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

#ifndef DATASTORE_H_
#define DATASTORE_H_

#include <db/returntypes/CheckSignature.h>
#include <Exception.h>
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvTextData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvStationParam.h>
#include <vector>
#include <deque>
#include <map>

namespace db
{
class DatabaseAccess;
}

namespace qabase
{
class ScriptResultIdentifier;

/**
 * Storage for data for a single check. Contains all relevant data for checks,
 * and also any modifications to the data.
 *
 * \ingroup group_scriptcreate
 */
class DataStore
{
public:
	typedef kvalobs::kvData Data;
	typedef std::deque<Data> DataList;
	typedef std::map<DataRequirement::Parameter, DataList> ParameterSortedDataList;

	typedef kvalobs::kvTextData RefData;
	typedef std::vector<RefData> RefDataList;
	typedef std::map<DataRequirement::Parameter, RefDataList> ParameterSortedRefDataList;

	typedef kvalobs::kvModelData ModelData;
	typedef std::vector<ModelData> ModelDataList;
	typedef std::map<DataRequirement::Parameter, ModelDataList> ParameterSortedModelDataList;

	//typedef kvalobs::kvStationParam MetaData;
	typedef float MetaData;
	typedef std::vector<MetaData> MetaDataList;
	typedef std::map<DataRequirement::Parameter, MetaDataList> ParameterSortedMetaDataList;

	DataStore(
			const db::DatabaseAccess & db,
			const kvalobs::kvStationInfo & observation,
			const std::string & qcx,
			const qabase::CheckSignature & abstractSignature,
			const qabase::CheckSignature & concreteSignature);

	// For creating tests
	DataStore(const ParameterSortedDataList & data, int flagPosition);

	~DataStore();

	const ParameterSortedDataList & getObsData() const { return data_; }
	const ParameterSortedRefDataList & getRefData() const { return refData_; }
	const ParameterSortedModelDataList & getModelData() const { return modelData_; }
	const ParameterSortedMetaDataList & getMetaData() const { return metaData_; }

	const kvalobs::kvStationInfo & observation() const { return observation_; }

	void apply(const ScriptResultIdentifier & resultType, double value);

	template<class Iterator>
	void getModified(Iterator out) const;

	QABASE_EXCEPTION(UnableToGetData);
	QABASE_EXCEPTION(InvalidParameter);
	QABASE_EXCEPTION(NoSuchData);

private:
	void populateObs_(
			const db::DatabaseAccess & db,
			const kvalobs::kvStationInfo & observation,
			const qabase::DataRequirement & abstractObsRequirement,
			const qabase::DataRequirement & concreteObsRequirement);
	void populateRefObs_(
				const db::DatabaseAccess & db,
				const kvalobs::kvStationInfo & observation,
				const qabase::DataRequirement & abstractRefObsRequirement,
				const qabase::DataRequirement & concreteRefObsRequirement);
	void populateModel_(
			const db::DatabaseAccess & db,
			const kvalobs::kvStationInfo & observation,
			const qabase::DataRequirement & abstractModelRequirement,
			const qabase::DataRequirement & concreteModelRequirement);
	void populateMeta_(
			const db::DatabaseAccess & db,
			const kvalobs::kvStationInfo & observation,
			const std::string & qcx,
			const qabase::DataRequirement & abstractMetaRequirement,
			const qabase::DataRequirement & concreteMetaRequirement);

	ParameterSortedDataList data_;
	ParameterSortedRefDataList refData_;
	ParameterSortedModelDataList modelData_;
	ParameterSortedMetaDataList metaData_;
	kvalobs::kvStationInfo observation_;
	std::string qcx_;
	int flagPosition_;
	typedef std::set<kvalobs::kvData *, kvalobs::compare::lt_kvData> ModificationList;
	ModificationList modified_;
};

template<class Iterator>
void DataStore::getModified(Iterator out) const
{
	for ( ModificationList::const_iterator it = modified_.begin(); it != modified_.end(); ++ it )
		* out ++ = ** it;
}

/**
 * \ingroup group_scriptcreate
 */
void fillMissing(DataStore::ParameterSortedDataList & dataList);

/**
 * \ingroup group_scriptcreate
 */
void fillMissing(DataStore::ParameterSortedRefDataList & dataList);


}

#endif /* DATASTORE_H_ */
