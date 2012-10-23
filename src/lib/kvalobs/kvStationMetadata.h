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
 
 You should have received a copy of the GNU General Public License aint
 with KVALOBS; if not, write to the Free Software Foundation Inc., 
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KVSTATIONMETADATA_H_
#define KVSTATIONMETADATA_H_

#include "kvDbBase.h"
#include <string>

namespace kvalobs
{

class kvStationMetadata: public kvDbBase
{
public:
	kvStationMetadata();

	/**
	 * @warning This object will _not_ take ownership over pointers passed to it.
	 */
	kvStationMetadata(int station, const int * param, const int * type,
			const int * level, const int * sensor, const std::string & name,
			float metadata, const std::string & description,
			const miutil::miTime & fromtime, const miutil::miTime & totime);

	kvStationMetadata(const kvStationMetadata & d);

	virtual ~kvStationMetadata();

	kvStationMetadata & operator =(const kvStationMetadata & d);

	int stationID() const
	{
		return station_;
	}
	int paramID() const
	{
		return param_;
	}
	int typeID() const
	{
		return type_;
	}
	int level() const
	{
		return level_;
	}
	int sensor() const
	{
		return sensor_;
	}

	const std::string & name() const
	{
		return name_;
	}
	float metadata() const
	{
		return metadata_;
	}

	const std::string & metadataDescription() const
	{
		return description_;
	}

	const miutil::miTime & fromtime() const
	{
		return fromtime_;
	}
	const miutil::miTime & totime() const
	{
		return totime_;
	}

	bool haveSpecificParam() const
	{
		return param_ != INT_NULL;
	}
	bool haveSpecificType() const
	{
		return type_ != INT_NULL;
	}
	bool haveSpecificLevel() const
	{
		return level_ != INT_NULL;
	}
	bool haveSpecificSensor() const
	{
		return sensor_ != INT_NULL;
	}

	virtual std::string toSend() const;
	virtual std::string uniqueKey() const;
	virtual const char* tableName() const;

private:
	int station_;
	int param_;
	int type_;
	int level_;
	int sensor_;
	std::string name_;
	float metadata_;
	std::string description_;
	miutil::miTime fromtime_;
	miutil::miTime totime_;
};

}

#endif /* KVSTATIONMETADATA_H_ */
