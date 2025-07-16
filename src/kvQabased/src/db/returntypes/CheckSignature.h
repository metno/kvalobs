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

#ifndef CHECKSIGNATURE_H_
#define CHECKSIGNATURE_H_

#include "DataRequirement.h"
#include <Exception.h>
#include <string>
#include <map>

namespace qabase {
class DataRequirement;

/**
 * A complete check signature, as seen in checks and algorithm tables in
 * kvalobs.
 *
 * A check signature consists of up to four parts, each of which is
 * implemented as a DataRequirement object:
 *
 * - \e obs is the list of observation data that is do be checked
 * - \e refobs is a list of observation data that will not be checked
 *             directly, but which is used in checks anyway
 * - \e model is a list of required model data for the check
 * - \e meta is required metadata, from the station_param table in kvalobs
 *
 * The signature's string representation consists of these four elements, in
 * order, separated by a '|' character. Requirements which are unused may be
 * empty.
 *
 * \ingroup group_db
 */
class CheckSignature {
 public:

  CheckSignature() {
    requirements_=std::map<std::string, DataRequirement>();
  }
  /**
   * Initialize object with the given string.
   *
   * @throw CheckSignature::Error or DataRequirement::Invalid if error
   *        occurs during parsing.
   * @param signature the signature string to parse
   * @param stationid Station identifier for the signature. All all
   *                  requirements are implicitly for this station, in
   *                  addition to any other stations explicitly mentioned in
   *                  the signature.
   */
  CheckSignature(const std::string & signature, int stationid,bool isConcreteSpecification);

  /**
   * Initialize object with the given string.
   *
   * @throw CheckSignature::Error or DataRequirement::Invalid if error
   *        occurs during parsing.
   * @param signature the signature string to parse
   * @param stationid Station identifier for the signature. All all
   *                  requirements are implicitly for this station, in
   *                  addition to any other stations explicitly mentioned in
   *                  the signature.
   */
  CheckSignature(const char * signature, int stationid, bool isConcreteSpecification);

  ~CheckSignature();

  bool isValid()const {
    return !requirements_.empty();
  }

  /**
   * Get obs requirements, or NULL if there are none.
   * @return obs part of signature
   */
  const DataRequirement * obs() const;

  /**
   * Get refobs requirements, or NULL if there are none.
   * @return refobs part of signature
   */
  const DataRequirement * refobs() const;

  /**
   * Get model requirements, or NULL if there are none.
   * @return model part of signature
   */
  const DataRequirement * model() const;

  /**
   * Get meta requirements, or NULL if there are none.
   * @return meta part of signature
   */
  const DataRequirement * meta() const;


  std::string str() const;

  /**
   * Throw on error during parse, which are not directly related to errors
   * in DataRequirment.
   */
  QABASE_EXCEPTION(Error);

 private:
  void parse_(const std::string & signature, int stationid, bool isConcreteSpecification);
  const DataRequirement * get_(const std::string & name) const;

  enum DataRequirementIdx {
    Obs,
    RefObs,
    Model,
    Meta
  };

  std::map<std::string, DataRequirement> requirements_;
};

}

#endif /* CHECKSIGNATURE_H_ */
