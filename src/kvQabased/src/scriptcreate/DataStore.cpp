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

#include "DataStore.h"
#include "ScriptResultIdentifier.h"
#include <db/DatabaseAccess.h>
#include <db/databaseResultFilter.h>

namespace qabase {
namespace {
template<class MissingFactory, class ParameterSortedList>
void fillMissing(ParameterSortedList & dataList) {
  typedef typename ParameterSortedList::mapped_type DataList;

  std::set<boost::posix_time::ptime> obsTimes;
  for (typename ParameterSortedList::const_iterator parameter =
      dataList.begin(); parameter != dataList.end(); ++parameter)
    for (typename DataList::const_iterator it = parameter->second.begin();
        it != parameter->second.end(); ++it)
      obsTimes.insert(it->obstime());

  for (typename ParameterSortedList::iterator parameter = dataList.begin();
      parameter != dataList.end(); ++parameter) {
    DataList & dataList = parameter->second;
    if (dataList.empty()) {
      // should never happen
      std::ostringstream msg;
      msg << "Internal error at " << __FILE__ << ':' << __LINE__;
      throw std::runtime_error(msg.str());
    }

    MissingFactory factory(dataList.front());

    for (std::set<boost::posix_time::ptime>::const_reverse_iterator times =
        obsTimes.rbegin(); times != obsTimes.rend(); ++times) {
      typename DataList::iterator find = dataList.begin();
      while (find != dataList.end() and find->obstime() > *times)
        ++find;

      if (find == dataList.end() or find->obstime() != *times)
        dataList.insert(find, factory(*times));
    }
  }
}

struct data_factory {
  const kvalobs::kvDataFactory f_;
  data_factory(const kvalobs::kvData & d)
      : f_(d) {
  }

  kvalobs::kvData operator ()(const boost::posix_time::ptime & t) const {
    return f_.getMissing(0, t);
  }
};
struct text_data_factory {
  const kvalobs::kvTextData d_;
  text_data_factory(const kvalobs::kvTextData & d)
      : d_(d) {
  }

  kvalobs::kvTextData operator ()(const boost::posix_time::ptime & t) const {
    return kvalobs::kvTextData(d_.stationID(), t, "-32767", 0, d_.tbtime(),
                               d_.typeID());
  }
};
}

void fillMissing(DataStore::ParameterSortedDataList & dataList) {
  fillMissing<data_factory>(dataList);
}

void fillMissing(DataStore::ParameterSortedRefDataList & dataList) {
  fillMissing<text_data_factory>(dataList);
}

void DataStore::populateObs_(const db::DatabaseAccess & db,
                             const kvalobs::kvStationInfo & observation,
                             const DataRequirement & abstractObsRequirement,
                             const DataRequirement & concreteObsRequirement) {
//	boost::posix_time::ptime from = observation.obstime();
//	boost::posix_time::ptime to = observation.obstime();
//	from.addMin(concreteObsRequirement.firstTime());
//	to.addMin(concreteObsRequirement.lastTime());

  ParameterTranslation translation = getTranslation(concreteObsRequirement,
                                                    abstractObsRequirement);

  for (DataRequirement::ParameterList::const_iterator parameter =
      concreteObsRequirement.parameter().begin();
      parameter != concreteObsRequirement.parameter().end(); ++parameter) {
    ParameterTranslation::const_iterator find = translation.find(*parameter);
    if (find == translation.end())
      throw UnableToGetData(
          "Unable to find translation for parameter: " + parameter->baseName());

    db::DatabaseAccess::DataList dbdata;
    db.getData(&dbdata, observation, *parameter,
               concreteObsRequirement.firstTime());

    if (dbdata.empty()) {
      kvalobs::kvDataFactory factory(
          observation.stationID(), observation.obstime(),
          parameter->haveType() ? parameter->type() : observation.typeID(),
          parameter->haveSensor() ? parameter->sensor() : 0,
          parameter->haveLevel() ? parameter->level() : 0);
      dbdata.push_back(factory.getMissing(0));
    }
    data_[find->second].assign(dbdata.begin(), dbdata.end());
  }

  fillMissing(data_);
}

void DataStore::populateRefObs_(
    const db::DatabaseAccess & db, const kvalobs::kvStationInfo & observation,
    const qabase::DataRequirement & abstractRefObsRequirement,
    const qabase::DataRequirement & concreteRefObsRequirement) {
  ParameterTranslation translation = getTranslation(concreteRefObsRequirement,
                                                    abstractRefObsRequirement);

  for (DataRequirement::ParameterList::const_iterator parameter =
      concreteRefObsRequirement.parameter().begin();
      parameter != concreteRefObsRequirement.parameter().end(); ++parameter) {
    ParameterTranslation::const_iterator find = translation.find(*parameter);
    if (find == translation.end())
      throw UnableToGetData(
          "Unable to find translation for parameter: " + parameter->baseName());

    db::DatabaseAccess::TextDataList textData;
    db.getTextData(&textData, observation, *parameter,
                   concreteRefObsRequirement.firstTime());
    if (textData.empty()) {
      kvalobs::kvTextData td(observation.stationID(), observation.obstime(), "",
                             0, boost::posix_time::ptime(),
                             observation.typeID());
      textData.push_back(td);
    }

    for (db::DatabaseAccess::TextDataList::const_iterator it = textData.begin();
        it != textData.end(); ++it)
      refData_[find->second].push_back(*it);
  }

  fillMissing(refData_);
}

void DataStore::populateModel_(
    const db::DatabaseAccess & db, const kvalobs::kvStationInfo & observation,
    const qabase::DataRequirement & abstractModelRequirement,
    const qabase::DataRequirement & concreteModelRequirement) {
//	boost::posix_time::ptime from = observation.obstime();
//	boost::posix_time::ptime to = observation.obstime();
//	from.addMin(concreteModelRequirement.firstTime());
//	to.addMin(concreteModelRequirement.lastTime());

  ParameterTranslation translation = getTranslation(concreteModelRequirement,
                                                    abstractModelRequirement);

  for (DataRequirement::ParameterList::const_iterator parameter =
      concreteModelRequirement.parameter().begin();
      parameter != concreteModelRequirement.parameter().end(); ++parameter) {
    ParameterTranslation::const_iterator find = translation.find(*parameter);
    if (find == translation.end())
      throw UnableToGetData(
          "Unable to find translation for parameter: " + parameter->baseName());

    db::DatabaseAccess::ModelDataList modelData;
    db.getModelData(&modelData, observation, *parameter,
                    concreteModelRequirement.firstTime());
    if (not modelData.empty())
      modelData_[find->second].assign(modelData.begin(), modelData.end());
    else {
      throw MissingModelData("missing model data");
      kvalobs::kvModelData missingModel(observation.stationID(),
                                        observation.obstime(), 0, 0, 0, -32767);
      modelData_[find->second].push_back(missingModel);  // missing observation
    }
  }
}

void DataStore::populateMeta_(
    const db::DatabaseAccess & db, const kvalobs::kvStationInfo & observation,
    const std::string & qcx,
    const qabase::DataRequirement & abstractMetaRequirement,
    const qabase::DataRequirement & concreteMetaRequirement) {
  ParameterTranslation translation = getTranslation(concreteMetaRequirement,
                                                    abstractMetaRequirement);

  for (qabase::DataRequirement::ParameterList::const_iterator parameter =
      concreteMetaRequirement.parameter().begin();
      parameter != concreteMetaRequirement.parameter().end(); ++parameter) {
    ParameterTranslation::const_iterator find = translation.find(*parameter);
    if (find == translation.end())
      throw UnableToGetData(
          "Unable to find translation for metadata parameter: "
              + parameter->str());

    std::string::size_type splitPoint = find->first.str().find_last_of('_');
    if (std::string::npos == splitPoint)
      throw UnableToGetData("Cannot understand metadata: " + find->first.str());
    std::string param = find->first.str().substr(0, splitPoint);
    std::string metadataType = find->first.str().substr(splitPoint + 1);

    std::string stationParam = db.getStationParam(observation, param, qcx);

    float val = db::resultfilter::parseStationParam(stationParam, metadataType);
    metaData_[find->second].push_back(val);
  }
}

void DataStore::populateStation_(const db::DatabaseAccess & db) {
  try {
    station_ = db.getStation(observation_.stationID());
  } catch (std::exception & e) {
    station_ = kvalobs::kvStation(
        observation_.stationID(),
        -1,
        -1,
        -1,
        0,
        "unknown station",
        -1,
        -1,
        "",
        "",
        "",
        0,
        true,
        boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1),
                                 boost::posix_time::time_duration(0, 0, 0)));
  }
}

DataStore::DataStore(const db::DatabaseAccess & db,
                     const kvalobs::kvStationInfo & observation,
                     const std::string & qcx,
                     const qabase::CheckSignature & abstractSignature,
                     const qabase::CheckSignature & concreteSignature)
    : observation_(observation),
      qcx_(qcx) {
  const DataRequirement * abstractObsRequirement = abstractSignature.obs();
  const DataRequirement * concreteObsRequirement = concreteSignature.obs();
  if (abstractObsRequirement or concreteObsRequirement) {
    if (!abstractObsRequirement or !concreteObsRequirement)
      throw UnableToGetData("check- and algorithm signature does not match");

    populateObs_(db, observation, *abstractObsRequirement,
                 *concreteObsRequirement);
  }

  const DataRequirement * abstractRefObsRequirement =
      abstractSignature.refobs();
  const DataRequirement * concreteRefObsRequirement =
      concreteSignature.refobs();
  if (abstractRefObsRequirement or concreteRefObsRequirement) {
    if (!abstractRefObsRequirement or !concreteRefObsRequirement)
      throw UnableToGetData("check- and algorithm signature does not match");
    populateRefObs_(db, observation, *abstractRefObsRequirement,
                    *concreteRefObsRequirement);
  }

  const DataRequirement * abstractModelRequirement = abstractSignature.model();
  const DataRequirement * concreteModelRequirement = concreteSignature.model();
  if (abstractModelRequirement or concreteModelRequirement) {
    if (!abstractModelRequirement or !concreteModelRequirement)
      throw UnableToGetData("check- and algorithm signature does not match");

    populateModel_(db, observation, *abstractModelRequirement,
                   *concreteModelRequirement);
  }

  const DataRequirement * abstractMetaRequirement = abstractSignature.meta();
  const DataRequirement * concreteMetaRequirement = concreteSignature.meta();
  if (abstractMetaRequirement or concreteMetaRequirement) {
    if (!abstractMetaRequirement or !concreteMetaRequirement)
      throw UnableToGetData("check- and algorithm signature does not match");

    populateMeta_(db, observation, qcx, *abstractMetaRequirement,
                  *concreteMetaRequirement);
  }

  populateStation_(db);

  flagPosition_ = db.getQcxFlagPosition(qcx);
}

DataStore::DataStore(const DataStore::ParameterSortedDataList & data,
                     int flagPosition)
    : data_(data),
      observation_(
          0,
          boost::posix_time::ptime(boost::gregorian::day_clock::universal_day(),
                                   boost::posix_time::hours(6)),
          1),
      qcx_("undefined"),
      flagPosition_(flagPosition) {
}

DataStore::~DataStore() {
}

namespace {
bool setFlag(kvalobs::kvData & data, int flagIndex, int newValue,
             bool regardlessOfOldValue = false) {
  kvalobs::kvControlInfo ci = data.controlinfo();
  int oldValue = ci.flag(flagIndex);
  if (oldValue < newValue or regardlessOfOldValue) {
    ci.set(flagIndex, newValue);
    data.controlinfo(ci);
    return true;
  }
  return false;
}
bool updateCfailed(kvalobs::kvData & data, const std::string & qcx,
                   int newValue) {
  if (newValue > 1) {
    std::string oldCfailed = data.cfailed();
    std::ostringstream cfailed;
    if (not oldCfailed.empty())
      cfailed << oldCfailed << ',';
    cfailed << qcx;
    //cfailed << ':' << newValue;
    data.cfailed(cfailed.str());
    return true;
  }
  return false;
}
}

void DataStore::apply(const ScriptResultIdentifier & resultType, double value) {
  ParameterSortedDataList::iterator pit = data_.find(resultType.parameter());
  if (pit == data_.end())
    throw InvalidParameter("Unknown parameter: " + resultType.parameter());

  size_t idx = resultType.timeIndex();

  if (pit->second.size() <= idx) {
    std::ostringstream msg;
    msg << resultType << ": Referenced return data does not exist";
    throw NoSuchData(msg.str());
  }
  Data & data = pit->second[idx];

  switch (resultType.correctionType()) {
    case ScriptResultIdentifier::Flag: {
      bool mod = false;
      mod |= setFlag(data, flagPosition_, value);
      mod |= updateCfailed(data, qcx_, value);
      if (mod)
        modified_.insert(&data);
    }
      break;
    case ScriptResultIdentifier::Corrected:
      data.corrected(value);
      modified_.insert(&data);
      break;
    case ScriptResultIdentifier::Missing:
      setFlag(data, 6, value, true);
      if (value == 2)
        data.corrected(-32766);
      else if (value == 3)
        data.corrected(-32767);
      modified_.insert(&data);
      break;
    case ScriptResultIdentifier::Undefined:
    default:
      throw std::runtime_error("Unknown modification type");  // never reached, hopefully
  }
}

}
