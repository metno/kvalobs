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

#ifndef CHECKCONTROL_H_
#define CHECKCONTROL_H_

#include <ostream>
#include <db/DatabaseAccess.h>
#include <memory>

namespace qabase {

/**
 * \defgroup group_control Controlling classes
 *
 * Classes for controlling check running.
 */

/**
 * Controls the running of checks.
 *
 * Once this class has been created, the function newObservation() may be
 * called repeatedly on it, to run checks. Except for output to the given
 * script log, no effects of having called this function is detectable from
 * this class.
 *
 * \ingroup group_control
 */
class CheckRunner {
 public:

  /**
   * Create CheckRunner instance.
   *
   * @param database The database connection, where data will both be read
   * and written, as required by the checks. Note that the CheckRunner class
   * will itself take care of database caching, so from here there is no
   * point in thinking of that.
   */
  explicit CheckRunner(std::shared_ptr<db::DatabaseAccess> database);

  static std::shared_ptr<CheckRunner> create(const std::string & dbConnect);

  ~CheckRunner();

  typedef std::list<kvalobs::kvData> DataList;
  typedef std::shared_ptr<DataList> DataListPtr;


  typedef db::DatabaseAccess::KvalobsDataPtr KvalobsDataPtr;

  /**
   * Signal that a new observation is ready for being checked. Checks will
   * immediately start running.
   *
   * @param obs The observation that we want to check.
   * @param scriptLog Where to log scripts and script results. Nothing will
   *                  be logged if scripLog is NULL.
   * @return A list of all modified data. Also, all data belonging to the
   *         given obs will be returned, even if it is unmodified.
   */
  KvalobsDataPtr newObservation(const qabase::Observation & obs,
                             std::ostream * scriptLog = 0);

  KvalobsDataPtr newObservation(const kvalobs::kvStationInfo & st,
                             std::ostream * scriptLog = 0);

  /**
   * Set a filter for checks to run. The given input should be an iterator
   * pair to a list of strings. Only checks with qcx (e.g QC1-1-42) in this
   * list will be run.
   *
   * @param qcxFrom start iterator to a list of strings with qcx sepcification
   * @param qcxTo end iterator to a list of strings with qcx sepcification
   */
  template<class Iterator>
  void setQcxFilter(Iterator qcxFrom, Iterator qcxTo) {
    qcxFilter_.clear();
    qcxFilter_.insert(qcxFrom, qcxTo);
  }

  /**
   * Determine if attempts to run a particular check should be performed.
   *
   * @param obs The observation which is to be checked.
   * @param check The check we want to consider for checking
   * @param expectedParameters The observation's expected parameter list
   * @return true if the given check should run. False otherwise
   */
  bool shouldRunCheck(
      const kvalobs::kvStationInfo & obs, const kvalobs::kvChecks & check,
      const db::DatabaseAccess::ParameterList & expectedParameters) const;

 private:
  bool shouldMarkStartAndStop_();

  KvalobsDataPtr checkObservation(const qabase::Observation & obs,
                               std::ostream * scriptLog);

  bool shouldRunAnyChecks(const qabase::Observation & obs) const;

  void resetObservationDataFlags(
      db::DatabaseAccess::DataList & observationData);

  void resetCFailed(db::DatabaseAccess::DataList & observationData);

  /**
   * @brief Determines if any parameters required by a check are present in the observation data.
   *
   * This function checks whether any of the parameters required by the given check
   * are present in the provided set of parameters (`parametersInData`) and whether
   * the corresponding sensor and level in the observation data match the requirements
   * specified by the check's signature.
   *
   * @param obs The observation being checked.
   * @param check The check whose parameter requirements are being evaluated.
   * @param parametersInData Set of parameter names present in the observation data.
   * @param observationData List of observation data elements to check for matching sensor/level.
   * @return true if at least one required parameter (with matching sensor/level) is present; false otherwise.
   *
   * If the check does not specify any parameter requirements (i.e., `obsRequirement` is null),
   * the function returns true, assuming all parameters in the observation data are valid for the check.
   */
  bool haveAnyParametersRequiredByCheck(
          const qabase::Observation & obs,
          const kvalobs::kvChecks & check,
          const std::set<std::string> &parametersInData,
          const db::DatabaseAccess::DataList & observationData, std::ostream * scriptLog=nullptr) const;

  bool haveAnyHqcCorrectedElements(
      const db::DatabaseAccess::DataList & observationData) const;

  std::shared_ptr<db::DatabaseAccess> db_;

  // If this is nonempty, only qcx checks listed here will run
  std::set<std::string> qcxFilter_;
};

/*
std::ostream &operator << (std::ostream &o, const CheckRunner::DataList &dl) {
  for( auto &e : dl) {
    o << e << std::endl;
  }
  return o;
}
*/

}

#endif /* CHECKCONTROL_H_ */
