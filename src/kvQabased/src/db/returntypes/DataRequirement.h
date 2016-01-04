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

#ifndef DATAREQUIREMENT_H_
#define DATAREQUIREMENT_H_

#include <Exception.h>
#include <string>
#include <vector>
#include <map>
#include <ostream>

namespace qabase {

/**
 * Specification for what data of a specific type is needed for a particular
 * check. A requirement may either be abstract or concrete. Abstract
 * requirements contain parameter specifications as they are used in scripts,
 * while concrete requirements contain parameter specifications as they are
 * in the database. Translations between requirements may be generated by
 * calling getTranslation().
 *
 * The format of the string is as follows:
 *
 * IDENTIFIER;PARAMETERS[;STATIONLIST[;TIME_RANGE]]
 *
 * - \e IDENTIFIER specifies the type of requirement. See the CheckSignature
 *                 documentation for details.
 * - \e PARAMETERS specifies what parameters we want. For abstract signatures,
 *                 these may be any strings. With concrete requirements, each
 *                 parameter should represent an entry in the kvalobs \e param
 *                 table.
 * - \e STATIONLIST is a list of stations we want. The station functionality
 *                  is currently defunct.
 * - \e TIME_RANGE specifies how far back in time we want data for, as a range
 *                 of minutes. For example, 0,-120 means we want data for
 *                 observation time, and two hours back in time. If this is
 *                 ommitted the time range [0,0] is assumed.
 *
 * Examples of requirements:
 *
 * <code>
 *   obs;TA;;0,-180
 * </code>
 *
 * This means that we want to check data for temperature (TA), for the past three hours.
 *
 * <code>
 *   meta;X_MIN,X_MAX
 * </code>
 *
 * Since there is no parameter called X in the kvalobs param table, this is
 * most likely an abstract signature. A corresponding concrete signature may
 * look like this:
 *
 * <code>
 *   meta;TA_low,TA_high;;
 * </code>
 *
 * This means temperature low and high from the station_param table.
 *
 * \see getTranslation() and CheckSignature.
 *
 * \ingroup group_db
 */
class DataRequirement {
 public:

  /**
   * Create an uninitialized object, similar to what you would get by
   * providing an empty initializer string.
   */
  DataRequirement();

  /**
   * Initialize with the given string. See the class documentation for
   * string format.
   *
   * @throw DataRequirement::Invalid if the specification was not valid.
   * @param signature Specification of requirements
   * @param stationid Station id for the station we are interested in. Data
   *                  mentioned in the specification is implicitly from this
   *                  station, regardless of whether the specification
   *                  contains any explicit stations.
   */
  DataRequirement(const char * signature, int stationid);
  ~DataRequirement();

  /**
   * Does the object contain any objects at all?
   *
   * @return True if there is no requirements.
   */
  bool empty() const;

  /**
   * Get the requirement type. (obs, refobs, model or meta).
   *
   * @return requirement type, as a string
   */
  const std::string & requirementType() const {
    return requirementType_;
  }

  class Parameter;
  typedef std::vector<Parameter> ParameterList;

  /**
   * Get a list of required parameters.
   * @return Reference to a list of parameters
   */
  const ParameterList & parameter() const {
    return parameter_;
  }

  /**
   * Does the given parameter exist in the requirement?
   *
   * @param baseParameter base parameter name, excluding any
   *                      level/sensor/typeid specifications
   * @return True if the given parameter is wanted
   */
  bool haveParameter(const std::string & baseParameter) const;

  typedef std::vector<int> Station;
  /**
   * Get a list of all required stations.
   * @return A list of station identifiers.
   */
  const Station & station() const {
    return station_;
  }

  /**
   * Do we want the given station?
   *
   * @param what The station identifier we want to query
   * @return True if we want the given station for the checks
   */
  bool haveStation(int what) const;

  /**
   * Get earliest time offset we are interested in
   *
   * @return time offset, in minutes
   */
  int firstTime() const {
    return firstTime_;
  }

  /**
   * Get latest time offset we are interested in.
   *
   * @return time offset, in minutes
   */
  int lastTime() const {
    return lastTime_;
  }

  /**
   * Thrown if the DataRequirement string is syntactically wrong.
   */
  QABASE_EXCEPTION(Invalid);

 private:
  std::string requirementType_;
  ParameterList parameter_;
  Station station_;
  int firstTime_;
  int lastTime_;
};

/**
 * Thrown if calling getTranslation() with requirements that do not match.
 *
 * \ingroup group_db
 */
QABASE_EXCEPTION(NonmatchingDataRequirements);

typedef std::map<DataRequirement::Parameter, DataRequirement::Parameter> ParameterTranslation;

/**
 * Create a translation map between an abstract and a concrete requirement.
 * Signatures must be compatible
 *
 * @throw NonmatchingDataRequirements if requirements do not match.
 * @param from these will become transaltion key
 * @param to will be source of translation values
 * @return Translation map
 *
 * \ingroup group_db
 */
ParameterTranslation getTranslation(const DataRequirement & from,
                                    const DataRequirement & to);

/**
 * %Parameter specification. May include required level, sensor or typeid for
 * concrete requirements.
 *
 * \ingroup group_db
 */
class DataRequirement::Parameter {
 public:
  /**
   * Create uninitalized parameter object
   */
  Parameter();

  /**
   * Initialize object with the given string
   * @param signature string to use as parameter specification
   */
  Parameter(const char * signature);

  /**
   * Initialize object with the given string range
   * @param start start of string to interpret
   * @param stop end of string to interpret
   */
  Parameter(const char * start, const char * stop);

  /**
   * Initialize object with the given string
   * @param signature string to use as parameter specification
   */
  Parameter(const std::string & signature);

  /**
   * Get the identifier for the parameter itself. (eg. TA or RR_24)
   *
   * @return Parameter identifier string
   */
  const std::string & baseName() const {
    return name_;
  }

  /**
   * Get a string representation of the required parameter.
   * @return Required parameter, possibly with level, sensor and typeid spec.
   */
  std::string str() const;

  /**
   * Get required level. If specification does not contain a specific level,
   * the return value may be anything.
   *
   * \see haveLevel()
   *
   * @return required level
   */
  int level() const {
    return level_;
  }

  /**
   * Get required sensor. If specification does not contain a specific
   * sensor, the return value may be anything.
   *
   * \see haveSensor()
   *
   * @return required sensor
   */
  int sensor() const {
    return sensor_;
  }

  /**
   * Get required typeid. If specification does not contain a specific
   * typeid, the return value may be anything.
   *
   * \see haveType()
   *
   * @return required typeid
   */
  int type() const {
    return typeid_;
  }

  /**
   * Does the specification contain a specific level?
   * @return True if parameter have an explicit level.
   */
  bool haveLevel() const {
    return level_ != NULL_PARAMETER_;
  }

  /**
   * Does the specification contain a specific sensor?
   * @return True if parameter have an explicit sensor.
   */
  bool haveSensor() const {
    return sensor_ != NULL_PARAMETER_;
  }

  /**
   * Does the specification contain a specific typeid?
   * @return True if parameter have an explicit typeid.
   */
  bool haveType() const {
    return typeid_ != NULL_PARAMETER_;
  }

 private:
  void parse_(const std::string & parameterString);
  static const int NULL_PARAMETER_;
  std::string name_;
  int level_;
  int sensor_;
  int typeid_;
};

inline bool operator ==(const DataRequirement::Parameter & a,
                        const DataRequirement::Parameter & b) {
  return a.str() == b.str();
}

inline bool operator <(const DataRequirement::Parameter & a,
                       const DataRequirement::Parameter & b) {
  return a.str() < b.str();
}

inline std::ostream & operator <<(std::ostream & s,
                                  const DataRequirement::Parameter & p) {
  return s << p.str();
}

}

#endif /* DATAREQUIREMENT_H_ */
