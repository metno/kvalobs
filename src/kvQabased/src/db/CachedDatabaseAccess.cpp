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

#include "CachedDatabaseAccess.h"
#include "returntypes/Observation.h"
#include <milog/milog.h>
#include <map>


namespace db {
std::map<std::string, int> CachedDatabaseAccess::qcxFlagPositions_;
std::map<std::string, boost::shared_ptr<kvalobs::kvAlgorithms> > CachedDatabaseAccess::algorithms_;

CachedDatabaseAccess::CachedDatabaseAccess(DatabaseAccess * baseImplementation,
                                           const qabase::Observation & obs)
    : FilteredDatabaseAccess(baseImplementation),
      modelDataCache_(obs.stationInfo()),
      dataCache_(obs.stationInfo()),
      textDataCache_(obs.stationInfo()) {
}

CachedDatabaseAccess::~CachedDatabaseAccess() {
}

int CachedDatabaseAccess::getQcxFlagPosition(const std::string & qcx) const {
  std::map<std::string, int>::const_iterator flagPosition = qcxFlagPositions_
      .find(qcx);
  if (flagPosition == qcxFlagPositions_.end()) {
    int val;
    try {
      val = FilteredDatabaseAccess::getQcxFlagPosition(qcx);
    } catch (std::exception &) {
      val = -32767;
    }
    flagPosition = qcxFlagPositions_.insert(std::make_pair(qcx, val)).first;
  }
  if (flagPosition->second < 0) {
    std::string mediumQcx(qcx, 0, qcx.find_last_of('-'));
    throw std::runtime_error(mediumQcx + ": No such check type");
  }
  return flagPosition->second;
}

kvalobs::kvAlgorithms CachedDatabaseAccess::getAlgorithm(
    const std::string & algorithmName) const {
  boost::shared_ptr<kvalobs::kvAlgorithms> & algorithm =
      algorithms_[algorithmName];
  if (!algorithm)
    algorithm = boost::shared_ptr<kvalobs::kvAlgorithms>(
        new kvalobs::kvAlgorithms(
            FilteredDatabaseAccess::getAlgorithm(algorithmName)));
  return *algorithm;
}

std::string CachedDatabaseAccess::getStationParam(
    const kvalobs::kvStationInfo & si, const std::string & parameter, int sensor, int level, 
    const std::string & qcx) const {
  if (si != lastStationParamQuery_.si
      || parameter != lastStationParamQuery_.parameter
      || sensor != lastStationParamQuery_.sensor
      || level != lastStationParamQuery_.level
      || qcx != lastStationParamQuery_.qcx) {
    lastStationParamQuery_.result = FilteredDatabaseAccess::getStationParam(
        si, parameter, sensor, level, qcx);
    lastStationParamQuery_.si = si;
    lastStationParamQuery_.parameter = parameter;
    lastStationParamQuery_.sensor = sensor;
    lastStationParamQuery_.level = level;
    lastStationParamQuery_.qcx = qcx;
  }
  return lastStationParamQuery_.result;
}

void CachedDatabaseAccess::getStationParamAll( qabase::StationParamList &result,
                                   const kvalobs::kvStationInfo & si,
                                   const std::string & parameter, 
                                   const std::string & qcx) const{
  result.clear();                                    
  stationParams_.filterStationParam(result, qcx, si.stationID(), parameter);

  if( ! result.empty() ) {
    return;
  }

  FilteredDatabaseAccess::getStationParamAll(result, si, parameter, qcx);
  stationParams_.insertFrom(result);
}


kvalobs::kvStation CachedDatabaseAccess::getStation(int stationid) const {
  std::shared_ptr<kvalobs::kvStation> station;

  auto find = stations_.find(stationid);
  if (find == stations_.end()) {
    try {
      const kvalobs::kvStation s = FilteredDatabaseAccess::getStation(stationid);
      station = std::make_shared<kvalobs::kvStation>(s);
    } catch (std::exception & e) {
      LOGDEBUG(e.what());
    }
    stations_[stationid] = station;
  } else {
    station = find->second;
  }

  if (!station) {
    std::ostringstream s;
    s << "Unable to find station information for station id " << stationid;
    throw std::runtime_error(s.str());
  }
  return *station;
}

void CachedDatabaseAccess::getModelData(
    ModelDataList * out, const kvalobs::kvStationInfo & si,
    const qabase::DataRequirement::Parameter & parameter,
    int minuteOffset) const {
  if (!modelDataCache_.getData(out, si, parameter, minuteOffset)) {
    FilteredDatabaseAccess::getModelData(out, si, parameter, minuteOffset);
    modelDataCache_.setData(*out, si, parameter, minuteOffset);
  }
}

void CachedDatabaseAccess::getData(
    DataList * out, const qabase::Observation & obs,
    const qabase::DataRequirement::Parameter & parameter,
    int minuteOffset) const {
  if (!dataCache_.getData(out, obs.stationInfo(), parameter, minuteOffset)) {
    FilteredDatabaseAccess::getData(out, obs, parameter, minuteOffset);
    dataCache_.setData(*out, obs.stationInfo(), parameter, minuteOffset);
  }
}

void CachedDatabaseAccess::getTextData(
    TextDataList * out, const qabase::Observation & obs,
    const qabase::DataRequirement::Parameter & parameter,
    int minuteOffset) const {
  if (!textDataCache_.getData(out, obs.stationInfo(), parameter, minuteOffset)) {
    FilteredDatabaseAccess::getTextData(out, obs, parameter, minuteOffset);
    textDataCache_.setData(*out, obs.stationInfo(), parameter, minuteOffset);
  }
}

}
