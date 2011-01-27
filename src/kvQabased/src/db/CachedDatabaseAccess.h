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

#ifndef CACHEDDATABASEACCESS_H_
#define CACHEDDATABASEACCESS_H_

#include "FilteredDatabaseAccess.h"
#include "cache/DataCache.h"
#include <boost/shared_ptr.hpp>

namespace db
{

/**
 * A caching filter for use with, other DatabaseAccess classes. Any methods
 * reimplemented here use some form of caching.
 *
 * \ingroup group_db
 */
class CachedDatabaseAccess: public FilteredDatabaseAccess
{
public:
	/**
	 * Construct cached database access
	 *
	 * @param baseImplementation The database we want to contact when we cannot use the cahce
	 * @param obs The observation we are checking now.
	 */
	CachedDatabaseAccess(DatabaseAccess * baseImplementation, const kvalobs::kvStationInfo & obs);
	virtual ~CachedDatabaseAccess();

	virtual int getQcxFlagPosition(const std::string & qcx) const;

	virtual kvalobs::kvAlgorithms getAlgorithm(const std::string & algorithmName) const;

	virtual std::string getStationParam(const kvalobs::kvStationInfo & si, const std::string & parameter, const std::string & qcx) const;

	virtual kvalobs::kvStation getStation(int stationid) const;

	virtual void getModelData(ModelDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset ) const;
	virtual void getData(DataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const;
	virtual void getTextData(TextDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const;

private:
	static std::map<std::string, int> qcxFlagPositions_;
	static std::map<std::string, boost::shared_ptr<kvalobs::kvAlgorithms> > algorithms_;

	struct StationParamSpec
	{
		StationParamSpec() : si(0, "", 0) {}
		kvalobs::kvStationInfo si;
		std::string parameter;
		std::string qcx;

		std::string result;
	};
	mutable StationParamSpec lastStationParamQuery_;

	mutable std::map<int, kvalobs::kvStation *> stations_;

	mutable DataCache<ModelDataList> modelDataCache_;
	mutable DataCache<DataList> dataCache_;
	mutable DataCache<TextDataList> textDataCache_;

};

}

#endif /* CACHEDDATABASEACCESS_H_ */
