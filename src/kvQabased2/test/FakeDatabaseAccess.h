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

#ifndef FAKEDATABASEACCESS_H_
#define FAKEDATABASEACCESS_H_

#include <db/DatabaseAccess.h>
#include <kvalobs/kvDataOperations.h>

class FakeDatabaseAccess: public db::DatabaseAccess
{
public:
	FakeDatabaseAccess();
	virtual ~FakeDatabaseAccess();

	virtual void getChecks(CheckList * out, const kvalobs::kvStationInfo & si) const;

	virtual int getQcxFlagPosition(const std::string & qcx) const;

	virtual void getExpectedParameters(ParameterList * out, const kvalobs::kvStationInfo & si) const;

	virtual kvalobs::kvAlgorithms getAlgorithm(const std::string & algorithmName) const;

	virtual void getModelData(ModelDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minutesBackInTime ) const;

	virtual void getData(DataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const;

	virtual std::string getStationParam(const kvalobs::kvStationInfo & si, const std::string & parameter, const std::string & qcx) const;

	virtual void getTextData(TextDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const;

	virtual void write(const DataList & data);

	typedef std::set<kvalobs::kvData, kvalobs::compare::lt_kvData> SavedData;
	SavedData savedData;
};

#endif /* FAKEDATABASEACCESS_H_ */
