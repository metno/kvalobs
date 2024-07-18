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

#include "populateScript.h"
#include <scriptrunner/Script.h>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <algorithm>

namespace qabase {
namespace {
bool have_missing_value(int missingFlag) {
  return missingFlag & 1;
}

void addObsDataToScript(scriptrunner::Script & script,
                        const qabase::Observation &obs,
                        const DataStore::ParameterSortedDataList & obsdata) {
  scriptrunner::ScriptInput input("obs");

  std::size_t missingCount = 0;

  std::vector<float> timeoffset;
  const DataStore::DataList & dl = obsdata.begin()->second;
  for (DataStore::DataList::const_iterator it = dl.begin(); it != dl.end();
      ++it)
    timeoffset.push_back(
        (it->obstime() - dl.front().obstime()).total_seconds() / 60);

  std::size_t numtimes = timeoffset.size();

  for (DataStore::ParameterSortedDataList::const_iterator param =
      obsdata.begin(); param != obsdata.end(); ++param) {
    std::vector<float> value;
    std::vector<float> missing;
    std::vector<float> controlinfo;
    for (DataStore::DataList::const_iterator it = param->second.begin();
        it != param->second.end(); ++it) {
      value.push_back(it->original());
      kvalobs::kvControlInfo ci = it->controlinfo();
      missing.push_back(ci.flag(6));  // fmis
      for (int i = 0; i < 16; ++i) {
        controlinfo.push_back(ci.flag(i));
      }
    }
    //numtimes = std::max(numtimes, value.size());

    missingCount += std::count_if(missing.begin(), missing.end(),
                                  have_missing_value);

    input.add(param->first.baseName(), value.begin(), value.end());
    input.add(param->first.baseName() + "_missing", missing.begin(),
              missing.end());
    input.add(param->first.baseName() + "_controlinfo", controlinfo.begin(),
              controlinfo.end());

    if (!param->second.empty()) {
      input.add(param->first.baseName()+"_typeid", param->second.begin()->typeID());
      input.add(param->first.baseName()+"_sensor", param->second.begin()->sensor());
      input.add(param->first.baseName()+"_level", param->second.begin()->level());
    }
  }

  input.add("obs_numtimes", numtimes);
  input.add("obs_missing", missingCount);

  input.add("obs_timeoffset", timeoffset.begin(), timeoffset.end());

  script.addInput(input);
}

void addRefObsDataToScript(
    scriptrunner::Script & script,
    const DataStore::ParameterSortedRefDataList & refobsdata) {
  if (not refobsdata.empty()) {
    scriptrunner::ScriptInput input("refobs");

    std::size_t missingCount = 0;

    std::vector<float> timeoffset;
    const DataStore::RefDataList & dl = refobsdata.begin()->second;
    for (DataStore::RefDataList::const_iterator it = dl.begin(); it != dl.end();
        ++it)
      timeoffset.push_back(
          (it->obstime() - dl.front().obstime()).total_seconds() / 60);
    std::size_t numtimes = timeoffset.size();

    for (DataStore::ParameterSortedRefDataList::const_iterator param =
        refobsdata.begin(); param != refobsdata.end(); ++param) {
      std::vector<float> missing;
      for (DataStore::RefDataList::const_iterator it = param->second.begin();
          it != param->second.end(); ++it)
        if (it->original().empty() or it->original() == "-32767") {
          missing.push_back(3);
          ++missingCount;
        } else
          missing.push_back(0);

      std::vector<std::string> textDataStrings;
      for (DataStore::RefDataList::const_iterator it = param->second.begin();
          it != param->second.end(); ++it)
        textDataStrings.push_back(it->original());
      input.adds(param->first.baseName(), textDataStrings.begin(),
                 textDataStrings.end());
      input.add(param->first.baseName() + "_missing", missing.begin(),
                missing.end());
    }

    input.add("refobs_numtimes", numtimes);
    input.add("refobs_missing", missingCount);

    input.add("refobs_timeoffset", timeoffset.begin(), timeoffset.end());

    script.addInput(input);
  }
}

void addModelDataToScript(
    scriptrunner::Script & script,
    const DataStore::ParameterSortedModelDataList & modeldata) {
  if (not modeldata.empty()) {
    scriptrunner::ScriptInput input("model");

    std::size_t missingCount = 0;

    std::vector<float> timeoffset;
    const DataStore::ModelDataList & dl = modeldata.begin()->second;
    for (DataStore::ModelDataList::const_iterator it = dl.begin();
        it != dl.end(); ++it)
      timeoffset.push_back(
          (it->obstime() - dl.front().obstime()).total_seconds() / 60);
    std::size_t numtimes = timeoffset.size();

    for (DataStore::ParameterSortedModelDataList::const_iterator param =
        modeldata.begin(); param != modeldata.end(); ++param) {
      std::vector<float> missing;
      for (DataStore::ModelDataList::const_iterator it = param->second.begin();
          it != param->second.end(); ++it)
        if (it->original() == -32767) {
          missing.push_back(3);
          ++missingCount;
        } else
          missing.push_back(0);

      std::vector<float> modelDataValues;
      for (DataStore::ModelDataList::const_iterator it = param->second.begin();
          it != param->second.end(); ++it)
        modelDataValues.push_back(it->original());
      input.add(param->first.baseName(), modelDataValues.begin(),
                modelDataValues.end());
      input.add(param->first.baseName() + "_missing", missing.begin(),
                missing.end());
    }

    input.add("model_numtimes", numtimes);
    input.add("model_missing", missingCount);

    input.add("model_timeoffset", timeoffset.begin(), timeoffset.end());

    script.addInput(input);
  }
}

void addMetaDataToScript(
    scriptrunner::Script & script,
    const DataStore::ParameterSortedMetaDataList & metadata) {
  if (not metadata.empty()) {
    scriptrunner::ScriptInput input("meta");

    for (DataStore::ParameterSortedMetaDataList::const_iterator meta = metadata
        .begin(); meta != metadata.end(); ++meta)
      input.add(meta->first.baseName(), meta->second.begin(),
                meta->second.end());

    input.add("meta_numtimes", 1);
    float timeOffsets = 0;
    input.add("meta_timeoffset", &timeOffsets, &timeOffsets + 1);
    input.add("meta_missing", 0);

    script.addInput(input);
  }
}

}

void addDataToScript(scriptrunner::Script & script,
                     const DataStore & dataStore) {
  scriptrunner::ScriptInput input("general");

  const kvalobs::kvStation & station = dataStore.station();
  const qabase::Observation & observation = dataStore.observation();
//	std::ostringstream stationIdentifier;
//	stationIdentifier << station.stationID() << " - " << station.name();
//	input.add("station", stationIdentifier.str());
  input.add("observation_stationid", observation.stationID());
  input.add("observation_typeid", observation.typeID());
  input.add("station_latitude", station.lat());
  input.add("station_longitude", station.lon());

  float time[6];
  boost::posix_time::ptime t = dataStore.observation().obstime();
  time[0] = t.date().year();
  time[1] = t.date().month();
  time[2] = t.date().day();
  time[3] = t.time_of_day().hours();
  time[4] = t.time_of_day().minutes();
  time[5] = t.time_of_day().seconds();
  input.add("obstime", time, time + 6);

  script.addInput(input);

  addObsDataToScript(script, dataStore.observation(), dataStore.getObsData());
  addRefObsDataToScript(script, dataStore.getRefData());
  addModelDataToScript(script, dataStore.getModelData());
  addMetaDataToScript(script, dataStore.getMetaData());
}

}

