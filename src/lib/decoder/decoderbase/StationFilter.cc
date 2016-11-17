/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2007-2016 met.no

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

#include <limits>
#include <string>
#include <list>
#include <iostream>
#include <stdexcept>
#include "milog/milog.h"
#include "StationFilter.h"

namespace c = miutil::conf;

namespace kvalobs {
namespace decoder {

StationFilterElement::StationFilterElement()
    : stationIdRangeFrom_(-1),
      stationIdRangeTo_(-1),
      name_("__default__"),
      publish_(false),
      saveToDb_(true) {
}

StationFilterElement::StationFilterElement(const std::string &filterName)
    : stationIdRangeFrom_(-1),
      stationIdRangeTo_(-1),
      name_(filterName),
      publish_(false),
      saveToDb_(false) {
}
StationFilterElement::~StationFilterElement() {
}

void StationFilterElement::publish(bool pub) {
  publish_ = pub;
}

void StationFilterElement::saveToDb(bool save) {
  saveToDb_ = save;
}

bool StationFilterElement::publish() const {
  return publish_;
}

bool StationFilterElement::saveToDb() const {
  return saveToDb_;
}

void StationFilterElement::setStationRange(long stationIdFrom, long stationIdTo) {
  if (stationIdFrom == stationIdTo) {
    stationIdRangeFrom_ = -1;
    stationIdRangeTo_ = -1;
    addStation(stationIdFrom);
  } else if (stationIdFrom < stationIdTo) {
    stationIdRangeFrom_ = stationIdFrom;
    stationIdRangeTo_ = stationIdTo;
  } else {  //  stationIdTo < stationIdFrom
    stationIdRangeFrom_ = stationIdTo;
    stationIdRangeTo_ = stationIdFrom;
  }
}
void StationFilterElement::addStation(long stationId) {
  stationIdList_.insert(stationId);
}

void StationFilterElement::addTypeId(long typeId) {
  typeids_.insert(typeId);
}

std::string StationFilterElement::name() const {
  return name_;
}

bool StationFilterElement::stationDefined(long stationId) const {
  if (stationIdRangeFrom_ > -1 && stationIdRangeTo_ > -1 && stationId >= stationIdRangeFrom_ && stationId <= stationIdRangeTo_)
    return true;

  return stationIdList_.find(stationId) != stationIdList_.end();
}

bool StationFilterElement::typeDefined(long typeId) const {
  typeids_.find(typeId) != typeids_.end();
}

bool StationFilterElement::filter(long stationId, long typeId) const {
  if (!stationDefined(stationId))
    return false;

  return typeids_.empty() || typeDefined(typeId);
}

namespace {
void setVals(const std::string &key, const std::string &name, const c::ConfSection &conf, std::set<long> &dst) {
  c::ValElementList vals = conf.getValue(key);
  long v;

  for (c::ValElement &e : vals) {
    v = e.valAsInt(-1);
    if (v < 0)
      throw std::logic_error("Filter element '" + name + "', key '" + key + "' invalid value(s), must be integers greater or equal to 0.");

    dst.insert(v);
  }
}
}

StationFilterElement StationFilterElement::readConfig(const miutil::conf::ConfSection &conf, const std::string &name) {
  StationFilterElement e(name);

  e.saveToDb(conf.getValue("save_to_db").valAsBool(false));
  e.publish(conf.getValue("publish").valAsBool(false));

  // Optional element
  c::ValElementList val = conf.getValue("station_range");

  if (val.size() > 0) {
    // Must be at least 1 elements.
    // If only one element. This is interpreted as
    // all stationids greater or equal to the value. ie 'stationIdFrom'.
    if (val.size() < 1)
      throw std::logic_error(
          "Filter element '" + name
              + "', key 'station_range' to few elements. Must be at least one, (stationIdFom, stationIdTo) if stationIdTo is missing, then the range is all stationids greater or equal to 'stationIdFrom'.");

    long v1 = val.valAsInt(-1, 0);
    long v2 = val.valAsInt(-1, 1);

    if (v2 < 0 && val.size() == 1)
      v2 = std::numeric_limits<long>::max();

    if (v1 < 0 || v2 < 0)
      throw std::logic_error(
          "Filter element '" + name
              + "', key 'station_range' invalid element(s). Must be at least one, (stationIdFom, stationIdTo) if stationIdTo is missing, then the range is all stationids greater or equal to 'stationIdFrom'.");

    e.setStationRange(v1, v2);
  }

  setVals("station_list", name, conf, e.stationIdList_);
  setVals("typeid_list", name, conf, e.typeids_);

  return e;
}

StationFilters::StationFilters() {
}

StationFilters::~StationFilters() {
}

std::list<std::string> StationFilters::filterNames() const {
  std::list<std::string> names;
  for (auto &f : filters_)
    names.push_back(f.name());
  return names;
}

StationFilterElement StationFilters::getFilterByName(const std::string &name) const {
  for (auto &f : filters_)
    if (f.name() == name)
      return f;
  throw std::logic_error("Filter: '" + name + "' do not exist!");
}

StationFilterElement StationFilters::filter(long stationId, long typeId) const {
  if (filters_.empty())
    return StationFilterElement();  // Default filter.
  for (auto &f : filters_) {
    if (f.filter(stationId, typeId))
      return f;
  }
  return defaultFilter_;  // Default filter.
}

std::list<kvalobs::kvData> StationFilters::publishOrSaveData(const std::list<kvalobs::kvData> &sd, bool saveData)const
{
  std::list<kvalobs::kvData> data;
  long sid=std::numeric_limits<long>::max();
  long tid=std::numeric_limits<long>::max();
  StationFilterElement f;

  for( auto &d : sd) {
    if(sid!=d.stationID() || tid!=d.typeID()) {
      sid=d.stationID();
      tid=d.typeID();
      f=filter(sid, tid);
    }

    // Use the saveToDb or publish filter.
    if( saveData ){
      if( f.saveToDb() )
        data.push_back(d);
    } else if( f.publish() ) {
      data.push_back(d);
    }
  }

  return std::move(data);

}
std::list<kvalobs::kvTextData> StationFilters::publishOrSaveTextData(const std::list<kvalobs::kvTextData> &textData, bool saveData)const {
  std::list<kvalobs::kvTextData> txtData;
  long sid=std::numeric_limits<long>::max();
  long tid=std::numeric_limits<long>::max();
  StationFilterElement f;

  for( auto &d : textData) {
    if(sid!=d.stationID() || tid!=d.typeID()) {
      sid=d.stationID();
      tid=d.typeID();
      f=filter(sid, tid);
    }

    // Use the saveToDb or publish filter.
    if( saveData ) {
      if( f.saveToDb() )
        txtData.push_back(d);
    } else if( f.publish() ) {
      txtData.push_back(d);
    }
  }
  return std::move(txtData);

}


std::list<kvalobs::kvData>
StationFilters::saveDataToDb(const std::list<kvalobs::kvData> &sd)const{
  return publishOrSaveData(sd, true);
}

std::list<kvalobs::kvTextData> StationFilters::saveTextDataToDb(const std::list<kvalobs::kvTextData> &textData)const
{
  return publishOrSaveTextData( textData, true);
}

std::tuple<std::list<kvalobs::kvData>, std::list<kvalobs::kvTextData> >
StationFilters::saveDataToDb(const std::list<kvalobs::kvData> &sd,
                             const std::list<kvalobs::kvTextData> &textData)const
{
  return std::make_tuple(saveDataToDb(sd), saveTextDataToDb(textData));
}


std::list<kvalobs::kvData> StationFilters::publishData(const std::list<kvalobs::kvData> &sd)const{
  return publishOrSaveData(sd, false);
}

std::list<kvalobs::kvTextData> StationFilters::publishData(const std::list<kvalobs::kvTextData> &textData)const{
  return publishOrSaveTextData( textData, false);
}

kvalobs::serialize::KvalobsData StationFilters::publishData(const std::list<kvalobs::kvData> &sd, const std::list<kvalobs::kvTextData> &textData)const{
  return kvalobs::serialize::KvalobsData( publishData(sd), publishData(textData));
}


void StationFilters::configDefaultFilter(const miutil::conf::ConfSection &conf) {
  std::list<std::string> keys = conf.getKeys();
  bool publish = false;
  bool saveToDb = true;

  for (auto &key : keys) {
    if (key == "save_to_db") {
      saveToDb=conf.getValue(key).valAsBool(true);
    } else if (key == "publish") {
      publish=conf.getValue(key).valAsBool(false);
    } else {
      throw std::logic_error("Invalid key '" + key + "' in the filters 'default' section, valid keys 'save_to_db' and 'publish'.");
    }
  }
  defaultFilter_.saveToDb(saveToDb);
  defaultFilter_.publish(publish);
}

StationFiltersPtr StationFilters::readConfig(const miutil::conf::ConfSection &conf_) {
  StationFiltersPtr filters(new StationFilters());
  c::ConfSection &conf = const_cast<c::ConfSection&>(conf_);
  c::ConfSection *fconf = conf.getSection("kvDataInputd.filters");
  std::ostringstream errs;
  bool hasError = false;

  // No filters are defined. This is ok.
  if (!fconf)
    return filters;

  std::list<std::string> fDefs = fconf->getSubSections();
  std::cerr << "Filter sections: \n";

  for (auto &filterName : fDefs) {
    try {
      c::ConfSection *filter = fconf->getSection(filterName);
      if (!filter)  // Should never happen
        continue;

      if (filterName == "default") {
        filters->configDefaultFilter(*filter);
        continue;
      }

      StationFilterElement elem = StationFilterElement::readConfig(*filter, filterName);
      filters->filters_.push_back(elem);
    } catch (const std::exception &ex) {
      errs << "Error: Filter def '" << filterName << "': " << ex.what() << "\n";
      hasError = true;
    }
  }

  if (hasError) {
    LOGERROR(errs.str());
  }

  return filters;
}

}
}
