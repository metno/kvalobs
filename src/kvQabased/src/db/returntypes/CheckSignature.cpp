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

#include "CheckSignature.h"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>

namespace qabase {

CheckSignature::CheckSignature(const std::string & signature, int stationid,bool isConcreteSpecification) {
  parse_(signature, stationid, isConcreteSpecification);
}

CheckSignature::CheckSignature(const char * signature, int stationid, bool isConcreteSpecification) {
  parse_(signature, stationid, isConcreteSpecification);
}

CheckSignature::~CheckSignature() {
}

const DataRequirement * CheckSignature::obs() const {
  return get_("obs");
}

const DataRequirement * CheckSignature::refobs() const {
  return get_("refobs");
}

const DataRequirement * CheckSignature::model() const {
  return get_("model");
}

const DataRequirement * CheckSignature::meta() const {
  return get_("meta");
}

const DataRequirement * CheckSignature::get_(const std::string & name) const {
  std::map<std::string, DataRequirement>::const_iterator find = requirements_
      .find(name);
  if (find == requirements_.end())
    return 0;
  return &find->second;
}

void CheckSignature::parse_(const std::string & signature, int stationid, bool isConcreteSpecification) {
  std::vector<std::string> requirements;
  boost::algorithm::split(requirements, signature,
                          boost::algorithm::is_any_of("|"));

  for (std::vector<std::string>::const_iterator it = requirements.begin();
      it != requirements.end(); ++it) {
    DataRequirement justParsed(it->c_str(), stationid, isConcreteSpecification);
    DataRequirement & inMemory = requirements_[justParsed.requirementType()];
    if (not inMemory.empty())
      throw Error("Requirement defined twice: " + signature);
    inMemory = justParsed;
  }
}

}
