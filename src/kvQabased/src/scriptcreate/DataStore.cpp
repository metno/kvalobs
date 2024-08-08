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
#include <miutil/replace.h>
#include <set>

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
                             const qabase::Observation & obs,
                             const DataRequirement & abstractObsRequirement,
                             const DataRequirement & concreteObsRequirement) {
//	boost::posix_time::ptime from = obs.obstime();
//	boost::posix_time::ptime to = obs.obstime();
//	from.addMin(concreteObsRequirement.firstTime());
//	to.addMin(concreteObsRequirement.lastTime());

  ParameterTranslation translation = getTranslation(concreteObsRequirement,
                                                    abstractObsRequirement);

  for (DataRequirement::ParameterList::const_iterator parameter =
      concreteObsRequirement.parameter().begin();
      parameter != concreteObsRequirement.parameter().end(); ++parameter) {
    ParameterTranslation::const_iterator find = translation.find(*parameter);
    if (find == translation.end()){
      throw UnableToGetData(
          "Unable to find translation for parameter: " + parameter->baseName());
    }

    db::DatabaseAccess::DataList dbdata;
    db.getData(&dbdata, obs, *parameter,
               concreteObsRequirement.firstTime());

    if (dbdata.empty()) {
      kvalobs::kvDataFactory factory(
          obs.stationID(), obs.obstime(),
          parameter->haveType() ? parameter->type() : obs.typeID(),
          parameter->haveSensor() ? parameter->sensor() : 0,
          parameter->haveLevel() ? parameter->level() : 0);
      dbdata.push_back(factory.getMissing(0));
    }
    data_[find->second].assign(dbdata.begin(), dbdata.end());
  }

  fillMissing(data_);
}

void DataStore::populateRefObs_(
    const db::DatabaseAccess & db, const qabase::Observation & obs,
    const qabase::DataRequirement & abstractRefObsRequirement,
    const qabase::DataRequirement & concreteRefObsRequirement) {
  ParameterTranslation translation = getTranslation(concreteRefObsRequirement,
                                                    abstractRefObsRequirement);

  for (DataRequirement::ParameterList::const_iterator parameter =
      concreteRefObsRequirement.parameter().begin();
      parameter != concreteRefObsRequirement.parameter().end(); ++parameter) {
    ParameterTranslation::const_iterator find = translation.find(*parameter);
    if (find == translation.end()) {
      throw UnableToGetData(
          "Unable to find translation for parameter: " + parameter->baseName());
    }

    db::DatabaseAccess::TextDataList textData;
    db.getTextData(&textData, obs, *parameter,
                   concreteRefObsRequirement.firstTime());
    if (textData.empty()) {
      kvalobs::kvTextData td(obs.stationID(), obs.obstime(), "",
                             0, boost::posix_time::ptime(),
                             obs.typeID());
      textData.push_back(td);
    }

    for (db::DatabaseAccess::TextDataList::const_iterator it = textData.begin();
        it != textData.end(); ++it) {
      refData_[find->second].push_back(*it);
    }
  }

  fillMissing(refData_);
}

void DataStore::populateModel_(
    const db::DatabaseAccess & db, const qabase::Observation & obs,
    const qabase::DataRequirement & abstractModelRequirement,
    const qabase::DataRequirement & concreteModelRequirement) {
//	boost::posix_time::ptime from = obs.obstime();
//	boost::posix_time::ptime to = obs.obstime();
//	from.addMin(concreteModelRequirement.firstTime());
//	to.addMin(concreteModelRequirement.lastTime());

  ParameterTranslation translation = getTranslation(concreteModelRequirement,
                                                    abstractModelRequirement);

  for (DataRequirement::ParameterList::const_iterator parameter =
      concreteModelRequirement.parameter().begin();
      parameter != concreteModelRequirement.parameter().end(); ++parameter) {
    ParameterTranslation::const_iterator find = translation.find(*parameter);
    if (find == translation.end()) {
      throw UnableToGetData(
          "Unable to find translation for parameter: " + parameter->baseName());
    }

    db::DatabaseAccess::ModelDataList modelData;
    db.getModelData(&modelData, obs.stationInfo(), *parameter,
                    concreteModelRequirement.firstTime());
    if (!modelData.empty()) {
      modelData_[find->second].assign(modelData.begin(), modelData.end());
    } else {
      throw MissingModelData("missing model data");
    }
  }
}



namespace {
  /**
   * findMetaParameter takes as parameter a translation of a mapping from concreteMetadata to abstractMetadata, 
   * a concrete parameter to serach for.
   * 
   * Returns a tuple with the first element is the parameter name of the concrete parameter,
   * the second element is the metadata type and the third parameter is the abstract parameter the 
   * concrete parameter is an instance of.
   * 
   * The metadata parameter is on the form Name_metaDataType. The name can contain _, it is the
   * last _ that separetes name from MetaDataType.  
   * 
   * @throw qabase::DataStore::UnableToGetData ekception if the parameter is not found or the param format is wrong.
   */
  std::tuple<DataRequirement::Parameter, DataRequirement::Parameter> findMetaParameter(const ParameterTranslation &translation, 
    const DataRequirement::Parameter &paramToSearch) {
          ParameterTranslation::const_iterator find = translation.find(paramToSearch);
    if (find == translation.end()) {
      throw qabase::DataStore::UnableToGetData(
          "Unable to find translation for metadata parameter: "
              + paramToSearch.str());
    }

    if ( ! find->first.hasMetaData() ) {
      throw qabase::DataStore::UnableToGetData("Expect concrete metadata: " + find->first.str());
    }
    
    return std::make_tuple(find->first, find->second);  
  }

  std::string findMetadata(const std::string &metaParam, 
    const qabase::DataRequirement & concreteObsRequirement, 
    qabase::StationParamList &paramMetadataList, std::ostream &log ) {
    int sensor;
    int level;
    int nMatch=0;
    std::tuple<int,int,std::string> exactMatch(std::make_tuple(-1, -1, ""));
    std::tuple<int,int,std::string> levelMatch(std::make_tuple(-1, -1, ""));
    std::string err;

    for( auto const &obsParam : concreteObsRequirement.parameter()) {
      if( obsParam.baseName() != metaParam ) {
        continue;
      }
      sensor=obsParam.haveSensor()?obsParam.sensor():0;
      level=obsParam.haveLevel()?obsParam.level():0;
      
      try{
        auto metadata=paramMetadataList.find(level, sensor);
        nMatch++;
        if ( std::get<1>(metadata)!=sensor ) {
          levelMatch=metadata;
        } else {
          exactMatch=metadata;
        }
      }
      catch( const qabase::StationParamList::MultipleMatch &ex) {
        nMatch+=2;  //To be more than one
      }
      catch( const qabase::StationParamList::NoMatch &ex) {
      }
    }

    if( nMatch==0) {
      throw qabase::DataStore::NoMetadata("META: No metadata for param '" + metaParam +"'.");
    }

    if ( nMatch > 1 ) {
      throw qabase::DataStore::NoMetadata("META: Multiple metadata for param '" + metaParam +"'.");
    }
    
    if( ! std::get<2>(exactMatch).empty() ) {
      log << "META: parameter '" << metaParam << "' exact match for level=" << std::get<0>(exactMatch) 
        << " and sensor=" << std::get<1>(exactMatch) << " meta: '" << miutil::replace_copy(std::get<2>(exactMatch), "\n", "|") <<"'.\n";
      return std::get<2>(exactMatch);
    } 
    
    log << "META: parameter '" << metaParam << "' partly match for level=" << std::get<0>(levelMatch) 
        << " (sensor=" << std::get<1>(levelMatch) <<" ) meta: '" << miutil::replace_copy(std::get<2>(levelMatch), "\n", "|") << "'.\n";
    
    return std::get<2>(levelMatch); 
  }
}

/**
 * Algoritme:
 *
 * 1. Finner metadata parameter fra checks metadata på formen PARAM_what, eks meta; TJ_max. PARAM=TJ og what=max.
 * 2. Søker opp og finner sensor og level for TJ i checks fra obs spesifikasjonen. eks obs;TJ&10&0&. Level=10 og sensor 0.
 * 3. Bruker sensor og level fra 2. til å finne metadata for TJ i station_param.
 *     a. Hvis sensor og param finnes brukes metadata for denne (Exact match).
 *     b. Hvis 3a. feiler, søk bare etter level uavhengig av sensor (Partiel match).
 * 4. Hvis ingen match finnes i punkt 3. eller 3b. gir flere muligheter så feiler søket etter metadata.
 */
void DataStore::populateMeta_(
    const db::DatabaseAccess & db, const qabase::Observation & obs,
    const std::string & qcx,
    const qabase::DataRequirement & concreteObsRequirement,
    const qabase::DataRequirement & abstractMetaRequirement,
    const qabase::DataRequirement & concreteMetaRequirement
    ) {
  std::set<std::string> notLogged;
  ParameterTranslation translation = getTranslation(concreteMetaRequirement,
                                                    abstractMetaRequirement);
  for (const qabase::DataRequirement::Parameter &parameter : concreteMetaRequirement.parameter()) {
    int sensor=0;
    int level=0;
    auto concreteToAbstractParam = findMetaParameter(translation, parameter);
    DataRequirement::Parameter concreteMetaParam = std::get<0>(concreteToAbstractParam);
    std::string metadataType = concreteMetaParam.metaDataType();
    DataRequirement::Parameter abstractMetaParam=std::get<1>(concreteToAbstractParam);
    
    qabase::StationParamList stationParamsMetadata;
    db.getStationParamAll(stationParamsMetadata, obs.stationInfo(), concreteMetaParam.metaDataParam(), qcx);
    std::ostringstream tmpLog;
    std::string metadata=findMetadata(concreteMetaParam.metaDataParam(), concreteObsRequirement, stationParamsMetadata, tmpLog);

    if( notLogged.insert(tmpLog.str()).second ) {
      *scriptLog_ << tmpLog.str();
    }
    
    float val = db::resultfilter::parseStationParam(metadata, metadataType);
    metaData_[abstractMetaParam].push_back(val);
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
                     const qabase::Observation & obs,
                     const std::string & qcx,
                     const qabase::CheckSignature & abstractSignature,
                     const qabase::CheckSignature & concreteSignature,
                     std::ostream *scriptLog)
    : observation_(obs),
      qcx_(qcx) {
  if( scriptLog == nullptr) {
    scriptLog_=&std::cerr;
  } else {
    scriptLog_=scriptLog;
  }
  const DataRequirement * abstractObsRequirement = abstractSignature.obs();
  const DataRequirement * concreteObsRequirement = concreteSignature.obs();
  if (abstractObsRequirement || concreteObsRequirement) {
    if (!abstractObsRequirement || !concreteObsRequirement){
      throw UnableToGetData("check- and algorithm signature does not match");
    }

    populateObs_(db, obs, *abstractObsRequirement,
                 *concreteObsRequirement);
  }

  const DataRequirement * abstractRefObsRequirement =
      abstractSignature.refobs();
  const DataRequirement * concreteRefObsRequirement =
      concreteSignature.refobs();
  if (abstractRefObsRequirement || concreteRefObsRequirement) {
    if (!abstractRefObsRequirement || !concreteRefObsRequirement){
      throw UnableToGetData("check- and algorithm signature does not match");
    }
    populateRefObs_(db, obs, *abstractRefObsRequirement,
                    *concreteRefObsRequirement);
  }

  const DataRequirement * abstractModelRequirement = abstractSignature.model();
  const DataRequirement * concreteModelRequirement = concreteSignature.model();
  if (abstractModelRequirement || concreteModelRequirement) {
    if (!abstractModelRequirement || !concreteModelRequirement){
      throw UnableToGetData("check- and algorithm signature does not match");
    }

    populateModel_(db, obs, *abstractModelRequirement,
                   *concreteModelRequirement);
  }

  const DataRequirement * abstractMetaRequirement = abstractSignature.meta();
  const DataRequirement * concreteMetaRequirement = concreteSignature.meta();
  if (abstractMetaRequirement || concreteMetaRequirement) {
    if (!abstractMetaRequirement || !concreteMetaRequirement){
      throw UnableToGetData("check- and algorithm signature does not match");
    }

    populateMeta_(db, obs, qcx, *concreteObsRequirement, *abstractMetaRequirement,
                  *concreteMetaRequirement);
  }

  populateStation_(db);

  flagPosition_ = db.getQcxFlagPosition(qcx);
}

// DataStore::DataStore(const DataStore::ParameterSortedDataList & data,
//                      int flagPosition)
//     : data_(data),
//       observation_(
//           0,
//           boost::posix_time::ptime(boost::gregorian::day_clock::universal_day(),
//                                    boost::posix_time::hours(6)),
//           1),
//       qcx_("undefined"),
//       flagPosition_(flagPosition) {
// }

DataStore::~DataStore() {
}

namespace {
bool setFlag(kvalobs::kvData & data, int flagIndex, int newValue,
             bool regardlessOfOldValue = false) {
  kvalobs::kvControlInfo ci = data.controlinfo();
  int oldValue = ci.flag(flagIndex);
  if (oldValue < newValue || regardlessOfOldValue) {
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
  if (pit == data_.end()){
    throw InvalidParameter("Unknown parameter: " + resultType.parameter());
  }

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
      if (mod){
        modified_.insert(&data);
      }
    }
      break;
    case ScriptResultIdentifier::Corrected:
      data.corrected(value);
      modified_.insert(&data);
      break;
    case ScriptResultIdentifier::Missing:
      setFlag(data, 6, value, true);
      if (value == 2){
        data.corrected(-32766);
      } else if (value == 3) {
        data.corrected(-32767);
      }
      modified_.insert(&data);
      break;
    case ScriptResultIdentifier::Undefined:
    default:
      throw std::runtime_error("Unknown modification type");  // never reached, hopefully
  }
}

}
