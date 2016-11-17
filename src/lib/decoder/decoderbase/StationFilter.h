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

#ifndef SRC_LIB_DECODER_DECODERBASE_STATIONFILTER_H_
#define SRC_LIB_DECODER_DECODERBASE_STATIONFILTER_H_

#include <string>
#include <set>
#include <memory>
#include <stdexcept>
#include <tuple>
#include "decodeutility/kvalobsdataserializer.h"
#include "miconfparser/miconfparser.h"
#include "kvalobs/kvData.h"
#include "kvalobs/kvTextData.h"

namespace kvalobs {
namespace decoder {

class StationFilterElement {
  StationFilterElement();
  explicit StationFilterElement(const std::string &filterName);

  friend class StationFilters;
 public:
  ~StationFilterElement();

  void publish(bool pub);
  void saveToDb(bool save);
  bool publish() const;
  bool saveToDb() const;

  bool stationDefined(long stationId) const;
  bool typeDefined(long typeId) const;
  void setStationRange(long stationIdFrom, long stationIdTo);
  void addStation(long stationId);
  void addTypeId(long typeId);

  std::string name() const;

  /**
   * @throw std::logig_error on error;
   */
  static StationFilterElement readConfig(const miutil::conf::ConfSection &conf, const std::string &name);
  bool filter(long stationId, long typeId) const;

  friend std::ostream& operator<<(std::ostream &strm, const StationFilterElement &filter);

 private:
  long stationIdRangeFrom_;
  long stationIdRangeTo_;
  std::set<long> stationIdList_;
  std::set<long> typeids_;
  std::string name_;
  bool publish_;
  bool saveToDb_;
};

class StationFilters;
typedef std::shared_ptr<StationFilters> StationFiltersPtr;

class StationFilters {
  StationFilterElement defaultFilter_;
  std::list<StationFilterElement> filters_;

  void configDefaultFilter(const miutil::conf::ConfSection &conf);

  std::list<kvalobs::kvData> publishOrSaveData(const std::list<kvalobs::kvData> &sd, bool saveData) const;
  std::list<kvalobs::kvTextData> publishOrSaveTextData(const std::list<kvalobs::kvTextData> &textData, bool saveData) const;

 public:

  StationFilters();
  ~StationFilters();

  StationFilterElement filter(long stationId, long typeId) const;

  std::list<kvalobs::kvData> saveDataToDb(const std::list<kvalobs::kvData> &sd) const;
  std::list<kvalobs::kvTextData> saveTextDataToDb(const std::list<kvalobs::kvTextData> &textData) const;

  std::tuple<std::list<kvalobs::kvData>, std::list<kvalobs::kvTextData> >
  saveDataToDb(const std::list<kvalobs::kvData> &sd, const std::list<kvalobs::kvTextData> &textData) const;

  std::list<kvalobs::kvData> publishData(const std::list<kvalobs::kvData> &sd) const;
  std::list<kvalobs::kvTextData> publishData(const std::list<kvalobs::kvTextData> &textData) const;

  kvalobs::serialize::KvalobsData publishData(const std::list<kvalobs::kvData> &sd, const std::list<kvalobs::kvTextData> &textData) const;

  std::list<std::string> filterNames() const;
  /**
   * @throw std::logical_error if the filter do not exist.
   */
  StationFilterElement getFilterByName(const std::string &name) const;
  static StationFiltersPtr readConfig(const miutil::conf::ConfSection &conf);

};

}
}

#endif /* SRC_LIB_DECODER_DECODERBASE_STATIONFILTER_H_ */
