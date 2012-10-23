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

#include "kvStationMetadata.h"
#include <limits>
#include <sstream>

namespace kvalobs
{

kvStationMetadata::kvStationMetadata() :
		station_(0), param_(INT_NULL), type_(INT_NULL), level_(INT_NULL), sensor_(
				INT_NULL), name_(""), metadata_(
				std::numeric_limits<float>::quiet_NaN()), description_(
				"invalid")
{
}

kvStationMetadata::kvStationMetadata(int station, const int * param,
		const int * type, const int * level, const int * sensor,
		const std::string & name, float metadata,
		const std::string & description, const miutil::miTime & fromtime,
		const miutil::miTime & totime) :
		station_(station), param_(param ? *param : INT_NULL), type_(
				type ? *type : INT_NULL), level_(level ? *level : INT_NULL), sensor_(
				sensor ? *sensor : INT_NULL), name_(name), metadata_(metadata), description_(
				description), fromtime_(fromtime), totime_(totime)
{
}

kvStationMetadata::kvStationMetadata(const kvStationMetadata & d) :
		station_(d.station_), param_(d.param_), type_(d.type_), level_(
				d.level_), sensor_(d.sensor_), name_(d.name_), metadata_(
				d.metadata_), description_(d.description_), fromtime_(
				d.fromtime_), totime_(d.totime_)
{
}

kvStationMetadata::~kvStationMetadata()
{

}

kvStationMetadata&
kvStationMetadata::operator =(const kvStationMetadata & rhs)
{
	if (&rhs != this)
	{
		station_ = rhs.station_;
		param_ = rhs.param_;
		type_ = rhs.type_;
		level_ = rhs.level_;
		sensor_ = rhs.sensor_;
		name_ = rhs.name_;
		metadata_ = rhs.metadata_;
		description_ = rhs.description_;
		fromtime_ = rhs.fromtime_;
		totime_ = rhs.totime_;
	}
	return *this;
}

namespace
{
std::ostream & possiblyNull(std::ostream & s, int i)
{
	if (i == kvStationMetadata::INT_NULL)
		return s << "NULL";
	return s << i;
}
}

std::string kvStationMetadata::toSend() const
{
	std::ostringstream q;
	q << "(" << station_ << ", ";
	possiblyNull(q, paramID()) << ", ";
	possiblyNull(q, typeID()) << ", ";
	possiblyNull(q, level()) << ", ";
	possiblyNull(q, sensor()) << ", ";
	q << quoted(name()) << ", ";
	q << metadata() << ", ";
	q << quoted(fromtime().isoTime()) << ", ";
	if (totime().undef())
		q << "NULL";
	else
		q << quoted(totime());
	q << ")";

	return q.str();
}

namespace
{
std::ostream & maybeNullKey(std::ostream & s, const char * key, int value)
{
	if (value == kvStationMetadata::INT_NULL)
		return s << key << " IS NULL";
	return s << key << " = " << value;
}
}

std::string kvStationMetadata::uniqueKey() const
{
	std::ostringstream q;

	q << " WHERE ";
	q << "stationid = " << station_ << " AND ";
	maybeNullKey(q, "paramid", param_) << " AND ";
	maybeNullKey(q, "typeid", type_) << " AND ";
	maybeNullKey(q, "level", level_) << " AND ";
	maybeNullKey(q, "sensor", sensor_) << " AND ";
	q << "metadatatypename = " << quoted(name()) << " AND ";
	q << "fromtime = " << quoted(fromtime().isoTime());

	return q.str();
}

const char* kvStationMetadata::tableName() const
{
	return "station_metadata";
}

}
