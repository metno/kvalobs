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

namespace db {

/**
 * Base adapter class for DatabaseAccess. Subclass this class in order to
 * modify parts of this a DatabaseAccess class' behaviour.
 *
 * \ingroup group_db
 */
class FilteredDatabaseAccess : public DatabaseAccess {
 public:
  /**
   * @param baseImplementation Any calls to this object will be redirected
   * to this DatabaseAccess.
   */
  FilteredDatabaseAccess(DatabaseAccess * baseImplementation)
      : baseImplementation_(baseImplementation) {
  }

  virtual bool commitIsNeccessary() const {
    return baseImplementation_->commitIsNeccessary();
  }

  virtual void beginTransaction() {
    baseImplementation_->beginTransaction();
  }

  virtual void commit() {
    baseImplementation_->commit();
  }

  virtual void rollback() {
    baseImplementation_->rollback();
  }

  virtual void getChecks(CheckList * out,
                         const qabase::Observation & obs) const {
    baseImplementation_->getChecks(out, obs);
  }

  virtual int getQcxFlagPosition(const std::string & qcx) const {
    return baseImplementation_->getQcxFlagPosition(qcx);
  }

  virtual void getParametersToCheck(ParameterList * out,
                                     const qabase::Observation & obs) const {
    baseImplementation_->getParametersToCheck(out, obs);
  }

  virtual kvalobs::kvAlgorithms getAlgorithm(
      const std::string & algorithmName) const {
    return baseImplementation_->getAlgorithm(algorithmName);
  }

  virtual std::string getStationParam(const kvalobs::kvStationInfo & si,
                                      const std::string & parameter,
                                      const std::string & qcx) const {
    return baseImplementation_->getStationParam(si, parameter, qcx);
  }

  virtual kvalobs::kvStation getStation(int stationid) const {
    return baseImplementation_->getStation(stationid);
  }

  virtual void getModelData(
      ModelDataList * out, const kvalobs::kvStationInfo & si,
      const qabase::DataRequirement::Parameter & parameter,
      int minutesBackInTime) const {
    return baseImplementation_->getModelData(out, si, parameter,
                                             minutesBackInTime);
  }

  virtual void getData(DataList * out, const qabase::Observation & obs,
                       const qabase::DataRequirement::Parameter & parameter,
                       int minuteOffset) const {
    baseImplementation_->getData(out, obs, parameter, minuteOffset);
  }

  virtual void getTextData(TextDataList * out,
                           const kvalobs::kvStationInfo & si,
                           const qabase::DataRequirement::Parameter & parameter,
                           int minuteOffset) const {
    baseImplementation_->getTextData(out, si, parameter, minuteOffset);
  }

  virtual KvalobsDataPtr complete(const kvalobs::kvStationInfo & si, const DataList & d = DataList(), const TextDataList & td = TextDataList()) const {
    return baseImplementation_->complete(si, d, td);
  }

  virtual void write(const DataList & data) {
    baseImplementation_->write(data);
  }

  virtual qabase::Observation * selectDataForControl() {
    return baseImplementation_->selectDataForControl();
  }

  virtual void markProcessDone(const qabase::Observation & obs) {
    baseImplementation_->markProcessDone(obs);
  }

 private:
  DatabaseAccess * baseImplementation_;
};

}

#endif /* FILTEREDDATABASEACCESS_H_ */
