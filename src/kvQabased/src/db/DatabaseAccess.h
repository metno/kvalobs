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

#ifndef DATABASEACCESS_H_
#define DATABASEACCESS_H_

#include "returntypes/DataRequirement.h"
#include <kvalobs/kvChecks.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvAlgorithms.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvStation.h>
#include <string>
#include <list>
#include <set>

namespace db
{

/**
 * \defgroup group_db Database access
 *
 * Functionality for accessing the kvalobs database. Also classes for
 * interpreting the occasionally weird returned strings from the database.
 *
 * The most important class here is DatabaseAccess.
 */

/**
 * Interface for accessing a kvalobs database.
 *
 * A "real" implementation is provided in KvalobsDatabaseAccess
 *
 * \ingroup group_db
 */
class DatabaseAccess
{
public:
	virtual ~DatabaseAccess() {};

	typedef std::vector<kvalobs::kvChecks> CheckList;
	/**
	 * Get all checks for the given observation
	 *
	 * @param[out] out Checks for observation goes into this list
	 * @param si The observation we wants checks for
	 */
	virtual void getChecks(CheckList * out, const kvalobs::kvStationInfo & si) const =0;

	/**
	 * Find index of controlinfo flag for the given qcx.
	 *
	 * @param qcx The medium qcx we want to find flag position for. Example: "QC1-4"
	 * @return index in controlinfo string.
	 */
	virtual int getQcxFlagPosition(const std::string & qcx) const =0;

	typedef std::set<std::string> ParameterList;
	/**
	 * What parameters are expected in this observation?
	 *
	 * @param[out] out parameters goes into this list
	 * @param si Observation we want the parameter list from
	 */
	virtual void getExpectedParameters(ParameterList * out, const kvalobs::kvStationInfo & si) const =0;

	/**
	 * Get an algorithm with the specified name
	 *
	 * @param algorithmName Identifier of the wanted algorithm
	 * @return A base algorithm for a check, which must be populated with data
	 *         to be runnable
	 */
	virtual kvalobs::kvAlgorithms getAlgorithm(const std::string & algorithmName) const =0;

	/**
	 * Get station's parameter's metadata.
	 *
	 * @param si Observation we are interested in. Note that obstime must be
	 *           correct in order to get a correct answer.
	 * @param parameter Meteorological phenomenon we are interested in
	 * @param qcx Identifier for the check we are going to run.
	 *
	 * @return An entry from the station_param table. As the format of some of
	 *         this table's contents is a bit weird, you must probably parse
	 *         the result later, using the function
	 *         resultfilter::parseStationParam()
	 */
	virtual std::string getStationParam(const kvalobs::kvStationInfo & si, const std::string & parameter, const std::string & qcx) const =0;

	/**
	 * Get information on the given station
	 *
	 * @param stationid identifier for the station we are interested in
	 *
	 * @return information about the station with the given id
	 */
	virtual kvalobs::kvStation getStation(int stationid) const =0;

	typedef std::vector<kvalobs::kvModelData> ModelDataList;
	/**
	 * Get forecast for a particular observation and parameter
	 * @param[out] out Return data goes here
	 * @param si Observation we are interested in.
	 * @param parameter Meteorological phenomenon we are interested in
	 * @param minutesBackInTime For what point in time do we want data. This
	 *                          is expressed as an offset in minutes from
	 *                          observation time.
	 */
	virtual void getModelData(ModelDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minutesBackInTime ) const =0;

	typedef std::list<kvalobs::kvData> DataList;
	/**
	 * Get observation data from database
	 *
	 * @param[out] out Return data goes here
	 * @param si Observation we are interested in.
	 * @param parameter Meteorological phenomenon we are interested in
	 * @param minuteOffset How far back in time from obstime do we want data
	 *                     for?
	 */
	virtual void getData(DataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const =0;

	typedef std::list<kvalobs::kvTextData> TextDataList;
	/**
	 * Get observation data from database - data which is not representable as a float
	 *
	 * @param[out] out Return data goes here
	 * @param si Observation we are interested in.
	 * @param parameter Meteorological phenomenon we are interested in
	 * @param minuteOffset How far back in time from obstime do we want data
	 *                     for?
	 */
	virtual void getTextData(TextDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset) const =0;

	/**
	 * Write data back to database
	 *
	 * @param data The data that we are interested in writing.
	 */
	virtual void write(const DataList & data) = 0;
};

}

#endif /* DATABASEACCESS_H_ */
