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

#include "StationParam.h"

namespace qabase {


std::tuple<int, int, std::string> StationParamList::find(int level, int sensor) const {
  int levels=0;
  std::tuple<int, int, std::string> levelMatch;

  for ( auto const & param: *this){
    if (param.level == level && param.sensor == sensor ) {
      return std::make_tuple(level, sensor, param.metadata);
    }

    if ( param.level == level ) {
      levels++;
      levelMatch=std::make_tuple(level, param.sensor, param.metadata);
    }
  }

  if ( levels == 1 ) {
    return levelMatch;  
  }
  
  if( levels == 0) {
    throw NoMatch("no matche for level "+ std::to_string(level) + " and sensor "+std::to_string(sensor));
  }
   
  throw MultipleMatch("multiple match for level "+ std::to_string(level));
}



void StationParamSet::filterStationParam(StationParamList &result, const std::string &qcx,int stationid, const std::string &paramid) const {
  result.clear();

  for( const auto &e : *this) {
    if(e.stationid == stationid && e.paramid == paramid && e.qcx == qcx ) {
      result.push_back(e);
    }
  }
  
}  



void StationParamSet::insertOrReplace(const StationParam &st ) {
  auto r = this->insert(st);

  if( r.second ) {
    return;
  }
  this->erase(r.first);
  this->insert(st);
}

void StationParamSet::insertFrom(const StationParamList &pl) {
  for( const auto &e: pl) {
    insertOrReplace(e);
  }
}

void StationParamSet::toVector(StationParamList &pl)const {
  pl.clear();
  for ( auto &e : *this) {
    pl.push_back(e);
  }
}

}

