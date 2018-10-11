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

#ifndef MOCKDATABASEACCESS_H_
#define MOCKDATABASEACCESS_H_

#include <gmock/gmock.h>
#include "FakeDatabaseAccess.h"

class MockDatabaseAccess : public db::DatabaseAccess {
 public:

  MOCK_CONST_METHOD2(getChecks, void (CheckList * out, const qabase::Observation & obs));

  MOCK_CONST_METHOD1(getQcxFlagPosition, int (const std::string & qcx));

  MOCK_CONST_METHOD2(getParametersToCheck, void (ParameterList * out, const qabase::Observation & obs));

  MOCK_CONST_METHOD1(getAlgorithm, kvalobs::kvAlgorithms (const std::string & algorithmName));

  MOCK_CONST_METHOD3(getStationParam, std::string (const kvalobs::kvStationInfo & si, const std::string & parameter, const std::string & qcx));

  MOCK_CONST_METHOD1(getStation, kvalobs::kvStation (int stationid));

  MOCK_CONST_METHOD4(getModelData, void (ModelDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minutesBackInTime ) );

  MOCK_CONST_METHOD4(getData, void (DataList * out, const qabase::Observation & obs, const qabase::DataRequirement::Parameter & parameter, int minuteOffset));

  MOCK_CONST_METHOD1(pin, bool (const qabase::Observation & obs));

  MOCK_CONST_METHOD4(getTextData, void (TextDataList * out, const kvalobs::kvStationInfo & si, const qabase::DataRequirement::Parameter & parameter, int minuteOffset));

  MOCK_CONST_METHOD3(complete, KvalobsDataPtr (const qabase::Observation & obs, const DataList & d, const TextDataList & td));

  MOCK_METHOD1(write, void (const DataList & data));

  MOCK_METHOD0(selectDataForControl, qabase::Observation*());

  MOCK_METHOD1(markProcessDone, void (const qabase::Observation & obs));

  // Will only work if setDefaultActions() have been called
  const FakeDatabaseAccess::SavedData & savedData() const {return fake_.savedData;}

  void setDefaultActions()
  {
    using namespace testing;

    ON_CALL(*this, getChecks(_, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getChecks));
    ON_CALL(*this, getQcxFlagPosition(_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getQcxFlagPosition));
    ON_CALL(*this, getParametersToCheck(_, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getParametersToCheck));
    ON_CALL(*this, getAlgorithm(_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getAlgorithm));
    ON_CALL(*this, getStationParam(_,_,_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getStationParam));
    ON_CALL(*this, getStation(_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getStation));
    ON_CALL(*this, getModelData(_, _, _, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getModelData));
    ON_CALL(*this, getData(_, _, _, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getData));
    ON_CALL(*this, getTextData(_, _, _, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::getTextData));
    ON_CALL(*this, complete(_, _, _))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::complete));
    ON_CALL(*this, write(_))
    .WillByDefault(Invoke(&fake_, &FakeDatabaseAccess::write));
  }

private:
  FakeDatabaseAccess fake_;
};

#endif /* MOCKDATABASEACCESS_H_ */
