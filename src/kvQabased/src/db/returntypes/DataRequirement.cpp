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

#include "DataRequirement.h"
#include <boost/spirit/include/classic.hpp>
#include <sstream>

namespace qabase {

DataRequirement::DataRequirement()
    : firstTime_(0),
      lastTime_(0) {
}

DataRequirement::DataRequirement(const char * signature, int stationid)
    : firstTime_(0),
      lastTime_(0) {
  if (signature[0] == '\0')
    return;

  using namespace boost::spirit::classic;

  bool ok =
      parse(
          signature,

          // Requirement type
          (alpha_p >> *alpha_p)[assign_a(requirementType_)] >> ';'

          // Parameters
              >> !((+(alnum_p | '_')
                  >> !('&' >> !int_p >> '&' >> !int_p >> '&' >> !int_p))[push_back_a(
                  parameter_)]
                  >> *(','
                      >> ((+(alnum_p | '_')
                          >> !('&' >> !int_p >> '&' >> !int_p >> '&' >> !int_p))[push_back_a(
                          parameter_)]))) >> ';'

              // Stations
              >> !((int_p)[push_back_a(station_)]
                  >> *(',' >> (int_p)[push_back_a(station_)])) >> ';'

              // Times
              //>> ! (int_p[assign_a(lastTime_)][assign_a(firstTime_)] >> ! (',' >> int_p[assign_a(firstTime_)]))
              >> !(int_p[assign_a(lastTime_)]
                  >> !(',' >> int_p[assign_a(firstTime_)]))
//			>> ! ((int_p[assign_a(lastTime_)] >> ',' >> int_p[assign_a(firstTime_)]) |
//					int_p[assign_a(lastTime_)])
              ).full;

  if (std::find(station_.begin(), station_.end(), stationid) == station_.end())
    station_.push_back(stationid);

  if (not ok)
    throw Invalid("Invalid syntax for signature: " + std::string(signature));

  if (lastTime_ < firstTime_)
    std::swap(lastTime_, firstTime_);
}

DataRequirement::~DataRequirement() {
}

bool DataRequirement::empty() const {
  return parameter_.empty() and station_.empty();
}

bool DataRequirement::haveParameter(const std::string & baseParameter) const {
  for (ParameterList::const_iterator find = parameter_.begin();
      find != parameter_.end(); ++find)
    if (find->baseName() == baseParameter)
      return true;
  return false;
}

bool DataRequirement::haveStation(int what) const {
  return std::find(station_.begin(), station_.end(), what) != station_.end();
}

ParameterTranslation getTranslation(const DataRequirement & from,
                                    const DataRequirement & to) {
  const DataRequirement::ParameterList & pfrom = from.parameter();
  const DataRequirement::ParameterList & pto = to.parameter();

  unsigned size = pfrom.size();
  if (size != pto.size())
    throw NonmatchingDataRequirements("nonmatching requirements");

  ParameterTranslation ret;
  for (unsigned i = 0; i < size; ++i){
    ret[pfrom[i]] = pto[i];
  }

  return ret;
}

const int DataRequirement::Parameter::NULL_PARAMETER_ =
    std::numeric_limits<int>::min();

DataRequirement::Parameter::Parameter()
    : level_(NULL_PARAMETER_),
      sensor_(NULL_PARAMETER_),
      typeid_(NULL_PARAMETER_) {
}

DataRequirement::Parameter::Parameter(const char * signature)
    : level_(NULL_PARAMETER_),
      sensor_(NULL_PARAMETER_),
      typeid_(NULL_PARAMETER_) {
  parse_(signature);
}

DataRequirement::Parameter::Parameter(const char * start, const char * stop)
    : level_(NULL_PARAMETER_),
      sensor_(NULL_PARAMETER_),
      typeid_(NULL_PARAMETER_) {
  parse_(std::string(start, stop));
}

DataRequirement::Parameter::Parameter(const std::string & signature)
    : name_(signature),
      level_(NULL_PARAMETER_),
      sensor_(NULL_PARAMETER_),
      typeid_(NULL_PARAMETER_) {
  parse_(signature);
}

std::string DataRequirement::Parameter::str() const {
  if (haveLevel() or haveSensor() or haveType()) {
    std::ostringstream ret;
    ret << name_;
    ret << '&';
    if (haveLevel())
      ret << level();
    ret << '&';
    if (haveSensor())
      ret << sensor();
    ret << '&';
    if (haveType())
      ret << type();
    return ret.str();
  } else
    return name_;
}

void DataRequirement::Parameter::parse_(const std::string & parameterString) {
  using namespace boost::spirit::classic;

  bool ok = parse(
      parameterString.c_str(),
      (+(alnum_p | '_'))[assign_a(name_)]
          >> !('&' >> !int_p[assign_a(level_)] >> '&'
              >> !int_p[assign_a(sensor_)] >> '&' >> !int_p[assign_a(typeid_)]))
      .full;

  if (not ok)
    throw DataRequirement::Invalid("Invalid parameter: " + parameterString);

}

}
