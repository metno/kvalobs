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


#ifndef FILTEREDDATABASEACCESS_H_
#define FILTEREDDATABASEACCESS_H_

#include "DatabaseAccess.h"

namespace db
{

/**
 * Base adapter class for DatabaseAccess. Subclass this class in order to
 * modify parts of this a DatabaseAccess class' behaviour.
 *
 * \ingroup group_db
 */
class FilteredDatabaseAccess : public DatabaseAccess
{
public:
	/**
	 * @param baseImplementation Any calls to this object will be redirected
	 * to this DatabaseAccess.
	 */
	FilteredDatabaseAccess(DatabaseAccess * baseImplementation) :
		baseImplementation_(baseImplementation)
	{}

	virtual void getChecks(CheckList * out, const kvalobs::kvStationInfo & si) const
	{
		baseImplementation_->getChecks(out, si);
	}

	virtual int getQcxFlagPosition(const std::string & qcx) const
	{
		return baseImplementation_->getQcxFlagPosition(qcx);
	}

	virtual void getExpectedParameters(ParameterList * out, const kvalobs::kvStationInfo & si) const
	{
		baseImplementation_->getExpectedParameters(out, si);
	}

	virtual kvalobs::kvAlgorithms getAlgorithm(const std::string & algorithmName) const
	{
		return baseImplementation_->getAlgorithm(algorithmName);
	}

	virtual std::string getStationParam(const kvalobs::kvStationInfo & si, const std::string & parameter, const std::string & qcx) const
	{
		return baseImplementation_->getStationParam(si, parameter, qcx);
	}

	virtual void getModelData(ModelDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minutesBackInTime ) const
	{
		return baseImplementation_->getModelData(out, si, parameter, minutesBackInTime);
	}

	virtual void getData(DataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const
	{
		baseImplementation_->getData(out, si, parameter, minuteOffset);
	}

	virtual void getTextData(TextDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const
	{
		baseImplementation_->getTextData(out, si, parameter, minuteOffset);
	}

	virtual void write(const DataList & data)
	{
		baseImplementation_->write(data);
	}

private:
	DatabaseAccess * baseImplementation_;
};

}

#endif /* FILTEREDDATABASEACCESS_H_ */