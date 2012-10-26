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

#ifndef KVDATACACHE_H_
#define KVDATACACHE_H_

#include <puTools/miTime.h>
#include <db/DatabaseAccess.h>
#include <kvalobs/kvStationInfo.h>
#include <boost/noncopyable.hpp>

namespace db
{

/**
 * Generic implementation of kvalobs data cache.
 *
 * @see CachedDatabaseAccess
 *
 * \ingroup group_db
 */
template<class DataList>
class DataCache : boost::noncopyable
{
public:
	explicit DataCache(const kvalobs::kvStationInfo & si) : si_(si) {}

	bool getData(DataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const
	{
		if ( si != si_ )
			return false;

		typename CachedData::const_iterator find = cache_.find(parameter);
		if ( find == cache_.end() )
			return false;

		const Data & cache = find->second;

		if ( cache.timeOffset > minuteOffset)
			return false;

		miutil::miTime earliestWantedObsTime = si_.obstime();
		earliestWantedObsTime.addMin(minuteOffset);

		for ( typename DataList::const_iterator it = cache.data.begin(); it != cache.data.end(); ++ it )
			if ( it->obstime() >= earliestWantedObsTime)
				out->push_back(* it);

		return true;
	}

	void setData(const DataList & toSet, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset)
	{
		if ( si != si_ )
			return;

		Data & d = cache_[parameter];
		d.timeOffset = minuteOffset;
		d.data = toSet;
	}



	const kvalobs::kvStationInfo & stationInfo() const { return si_; }

private:

	struct Data
	{
		Data() : timeOffset(0) {}
		Data(int timeOffset) : timeOffset(timeOffset) {}

		int timeOffset;
		DataList data;
	};

	typedef std::map<qabase::DataRequirement::Parameter, Data> CachedData;
	CachedData cache_;

	const kvalobs::kvStationInfo si_;
};

}

#endif /* KVDATACACHE_H_ */
