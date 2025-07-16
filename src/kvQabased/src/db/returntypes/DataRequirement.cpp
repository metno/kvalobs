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
#include "db/KvalobsDatabaseAccess.h"
#include <boost/spirit/include/classic.hpp>
#include <sstream>

namespace qabase {

DataRequirement::DataRequirement()
    : firstTime_(0),
      lastTime_(0),
      isConcrete_(false) {
}

DataRequirement::DataRequirement(const char * signature, int stationid,  bool isConcreteSpecifification)
    : firstTime_(0),
      lastTime_(0),
      isConcrete_(isConcreteSpecifification) {
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

  if (std::find(station_.begin(), station_.end(), stationid) == station_.end()) {
    station_.push_back(stationid);
  }

  if ( !ok ) {
    throw Invalid("Invalid syntax for signature: " + std::string(signature));
  }

  if (lastTime_ < firstTime_){
    std::swap(lastTime_, firstTime_);
  }

  if (isConcreteSpecifification && requirementType_ == "meta" ) {
    for( auto &param : parameter_ ){
      if ( ! param.parseAsConcreteMetaParameter() ) {
        throw Invalid("Invalid metadata syntax for concrete meta parameter '" + param.baseName()+"' in signature: " + std::string(signature));
      }
    }
  }


}

DataRequirement::~DataRequirement() {
}

std::list<int> DataRequirement::getDefinedTypeIDs() const{
  std::list<int> ret;
  for (const auto &param : parameter_) {
    if (param.haveType()) {
      ret.push_back(param.type());
    }
  }
  return ret;
}

bool DataRequirement::dataMatchesTheRequirement(const db::DatabaseAccess::DataList &dataList) const {
    int nMatching = 0;
    for (const Parameter &param : parameter_) {
      for(const kvalobs::kvData &data : dataList) {
        // If the parameter does not match the data, we skip it.
        if ( param.useParameter(data) ) {
          nMatching++; // We have a matching data.
          break;;
        }
      }
    }
    //Do we have data for all parameters?
    return nMatching == parameter_.size();
  }
  

bool DataRequirement::haveTypeID(int typeid_) const {
    std::list<int> definedTypeIDs = getDefinedTypeIDs();
    if ( definedTypeIDs.empty() ) {
      return true; // If no typeids are defined, we assume all typeids are wanted.
    }
    for ( auto t : definedTypeIDs) {
      if ( t == typeid_ ) {
        return true; // If the typeid is in the list of defined typeids, we return true.
      }

    } 
    return false;
  }  

bool DataRequirement::haveParameterSensorLevel(const std::string & paramName, int sensor, int level) const {
    for (const auto &param : parameter_) {
      if (param.baseName() == paramName && param.haveSensor(sensor) && param.haveLevel(level)) {
        return true;
      }
    }
    return false;
  }
bool DataRequirement::haveSensorLevel(int sensor, int level) const {
  for (ParameterList::const_iterator it = parameter_.begin();
      it != parameter_.end(); ++it) {
    if (it->haveSensor(sensor) && it->haveLevel(level)){
        return true;
    }
  }
  return false;
}

bool DataRequirement::empty() const {
  return parameter_.empty() and station_.empty();
}

bool DataRequirement::haveParameter(const std::string & baseParameter) const {
  for (ParameterList::const_iterator find = parameter_.begin();
      find != parameter_.end(); ++find) {
    if (find->baseName() == baseParameter){
      return true;
    }
  }
  return false;
}


bool DataRequirement::haveStation(int what) const {
  return std::find(station_.begin(), station_.end(), what) != station_.end();
}

std::string DataRequirement::str() const {
  std::ostringstream oss;
  oss << requirementType_ << ';';

  for (ParameterList::const_iterator it = parameter_.begin();
      it != parameter_.end(); ++it) {
    if (it != parameter_.begin()) {
      oss << ',';
    }
    oss << it->str();
  }
  //
  oss << ";;";

  if (firstTime_!=0 || lastTime_ != 0) {
    oss << firstTime_ << ',' << lastTime_ << ';';
  }
  return oss.str();
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

bool DataRequirement::Parameter::useParameter(const kvalobs::kvData &data) const {
    std::string dataParamName = db::KvalobsDatabaseAccess::getParamName(data.paramID());
    if (dataParamName != name_) {
      return false; // The parameter name does not match.
    }
   

    return haveSensor(data.sensor()) &&
           haveLevel(data.level()) &&
           haveType(data.typeID());
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

bool DataRequirement::Parameter::parseAsConcreteMetaParameter() {
  std::string::size_type splitPoint = name_.find_last_of('_');
  if (std::string::npos == splitPoint) {
    return false;
  }
    
  metaDataParamName_= name_.substr(0, splitPoint);
  metaDataType_ = name_.substr(splitPoint + 1);

  return true;
}

}
