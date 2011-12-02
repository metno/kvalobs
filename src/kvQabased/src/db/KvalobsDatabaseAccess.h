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

#ifndef KVALOBSDATABASEACCESS_H_
#define KVALOBSDATABASEACCESS_H_

#include "DatabaseAccess.h"
#include <string>

namespace dnmi
{
namespace db
{
class Connection;
}
}

namespace db
{

/**
 * Direct access to kvalobs database. All function calls here result in a call
 * to the database. Caching is provided elsewhere.
 *
 * \ingroup group_db
 */
class KvalobsDatabaseAccess: public db::DatabaseAccess
{
public:
	/**
	 * Construct, creating a database connection with this object
	 *
	 * @param databaseConnect postgresql database connection string, as
	 *                        specified in postgresql documentation.
	 */
	explicit KvalobsDatabaseAccess(const std::string & databaseConnect);

	/**
	 * Construct object, using the provided connection.
	 *
	 * @param connection Use this connection when accessing kvalobs
	 * @param takeOwnershipOfConnection If true, the provided connection will
	 *                                  be deleted when this object's
	 *                                  destructor is called.
	 */
	KvalobsDatabaseAccess(dnmi::db::Connection * connection, bool takeOwnershipOfConnection);

	virtual ~KvalobsDatabaseAccess();

	virtual bool commitIsNeccessary() const { return true; }

	virtual void beginTransaction();

	virtual void commit();

	virtual void rollback();

	virtual void getChecks(CheckList * out, const kvalobs::kvStationInfo & si) const;

	virtual int getQcxFlagPosition(const std::string & qcx) const;

	virtual void getExpectedParameters(ParameterList * out, const kvalobs::kvStationInfo & si) const;

	virtual kvalobs::kvAlgorithms getAlgorithm(const std::string & algorithmName) const;

	virtual std::string getStationParam(const kvalobs::kvStationInfo & si, const std::string & parameter, const std::string & qcx) const;

	virtual kvalobs::kvStation getStation(int stationid) const;

	virtual void getModelData(ModelDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minutesBackInTime ) const;

	virtual void getData(DataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const;

	virtual void getTextData(TextDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const;

	virtual void write(const DataList & data);

private:
	class TransactionEnforcingDatabaseConnection;

	TransactionEnforcingDatabaseConnection * connection_;
	//dnmi::db::Connection * connection_;
	//bool ownsConnection_;
};

}

#endif /* KVALOBSDATABASEACCESS_H_ */
