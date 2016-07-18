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

bool StationFilterElement::filter(long stationId, long typeId) const {
  bool hasStationid = false;
  if (stationIdRangeFrom_ != -1 && stationIdRangeTo_ != -1 && stationId >= stationIdRangeFrom_ && stationId < stationIdRangeTo_)
    hasStationid = true;

  if (!hasStationid)
    hasStationid = stationIdList_.find(stationId) != stationIdList_.end();

  if (!hasStationid)
    return false;

  if (typeids_.empty())
    return true;
  else
    return typeids_.find(typeId) != typeids_.end();
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
      throw std::logic_error("Filter element '" + name + "', key 'station_range' to few elements. Must be at least one, (stationIdFom, stationIdTo) if stationIdTo is missing, then the range is all stationids greater or equal to 'stationIdFrom'.");

    long v1 = val.valAsInt(-1, 0);
    long v2 = val.valAsInt(-1, 1);

    if(v2<0 && val.size()==1)
      v2 = std::numeric_limits<long>::max();

    if (v1 < 0 || v2 < 0)
        throw std::logic_error("Filter element '" + name + "', key 'station_range' invalid element(s). Must be at least one, (stationIdFom, stationIdTo) if stationIdTo is missing, then the range is all stationids greater or equal to 'stationIdFrom'.");

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

StationFilters StationFilters::readConfig(const miutil::conf::ConfSection &conf_) {
  StationFilters filters;
  c::ConfSection &conf = const_cast<c::ConfSection>(conf_);
  c::ConfSection *fconf = conf.getSection("kvDataInputd.filters");

  // No filters are defined. This is ok.
  if (!fconf)
    return filters;

  std::list<std::string> fDefs=fconf->getSubSections();
  std::cerr << "Filter sections: \n    ";

  for( auto &filter : fDefs  )
    std::cerr << " " << filter << "\n";


  return filters;
}

}
}
