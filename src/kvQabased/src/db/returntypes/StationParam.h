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

#ifndef STATIONPARAM_H_
#define STATIONPARAM_H_

#include <Exception.h>
#include <string>
#include <vector>
#include <set>
#include <tuple>

namespace qabase {


struct StationParam {
  int stationid;
  std::string paramid;
  int sensor;
  int level;
  std::string qcx;
  std::string metadata;

  StationParam(){}
  StationParam(int sid, const std::string &pid, int sensor_, int level_, const std::string &qcx_, const std::string &metadata_):
    stationid(sid), paramid(pid), sensor(sensor_), level(level_), qcx(qcx_), metadata(metadata_){}

  bool operator < (const qabase::StationParam &rhs) const {
    if (qcx < rhs.qcx || stationid < rhs.stationid || 
      paramid < rhs.paramid ||  
      sensor < rhs.sensor || level < rhs.level) {
      return true;
    }
    return false;
  }
};

class StationParamList: public  std::vector<StationParam> {
  public:
  
  QABASE_EXCEPTION(NoMatch);
  QABASE_EXCEPTION(MultipleMatch);
  
  /**
   * Find metadata for level and sensor. If no level or senor match exact, tries to match level. 
   * Throws NoMath if no matches is found and MultipleMatch if multiple matches is found.
   * 
   * Sensor is -1 if only level match.
   * 
   * @return tuple <level, sensor, metadata>
   * @throw NoMatch, if there is no metadata 
   * @throw MultipleMatch, if multiple metadata matches for levels.
   */
  std::tuple<int, int, std::string> find(int level, int sensor) const;  
};

class StationParamSet :  public std::set<StationParam> {
  public:
  void filterStationParam(StationParamList &result, const std::string &qcx,int stationid, const std::string &paramid) const;
  void insertOrReplace(const StationParam &st );
  void insertFrom(const StationParamList &pl);
  void toVector(StationParamList &pl)const;
};


}

#endif