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

/**
 * Parse the section kvDataInputd.filters in kvalobs.conf.
 * This sections is a list of filters that controls what 
 * to do with an observation. The default action is to save it to 
 * the database and add an entrie in workque. kvQabased only 
 * work on data in the workque.
 * 
 * We can define filters in the folowing way:
 * 
 *   - Add an observation to the database and workque. This is
 *     the dafault.
 *   - Publish the observation to the 'checked' queue without saving to the database.
 *     This implies that no entrie is put in the worque.
 *   - Publish the observation and add it to the database, but no entries in the workque.
 *   - Only save the data to the database with no entries in the workque and no publish.
 * 
 * Each filter has a name.
 * The filter variables that can be set is (* indicates default value):
 * 
 *  save_to_db (*true/false) - Shall the observation be put into the database.
 *                             save_to_db=false, implies add_to_work_queue=false.
 *  publish (true/*false)    - Shall the obbservation be published on the checked que.
 *  add_to_work_queue (*true/false) - Shall we add the observation to workque. Only observation 
 *                                    in the workque is checked by kvQabased.
 *  typeid_list              - A list of typeids this filter element is valid for. An empty list (default) 
 *                             means all typides.
 *  station_list             - A list of stationids this filter element is valid for. If the list 
 *                             has an element with value 0 it means all stations.
 *  station_range  [from, to] - Speciefies a range of stationids that is valid. 
 *                              If the 'to'=-1, all stationids greater or equal to 'from'.
 *                              If the 'from'=-1, all stationids less or equal to 'to'.
 *   
 * kvDataInputd{
    filters {
      filter1 {
        save_to_db=true
        publish = false
        typeid_list = (300, 302)
        station_range =(1, 10)
        station_list=(20)
      }

      #For all stations with typeid=508. Do not save to the database, 
      #but publish to the 'checked' queue.
      filter2 {
        save_to_db=false
        publish = true
        typeid_list=(508)
        station_list=(0)
      }
      default {
        save_to_db = true
        publish = false
      }   
    }
  }
*/



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
  void setAddToWorkQueue(bool f);
  bool addToWorkQueue()const;

  std::string name() const;

  /**
   * @throw std::logig_error on error;
   */
  static StationFilterElement readConfig(const miutil::conf::ConfSection &conf, const std::string &name, bool isDefault);
  bool filter(long stationId, long typeId) const;

  friend std::ostream& operator<<(std::ostream &strm, const StationFilterElement &filter);

 private:
  long stationIdRangeFrom_;
  long stationIdRangeTo_;
  std::set<long> stationIdList_;
  std::set<long> typeids_;
  std::string name_;
  bool addToWokque_;
  bool publish_;
  bool saveToDb_;
};

class StationFilters;
typedef std::shared_ptr<StationFilters> StationFiltersPtr;

class StationFilters {
  StationFilterElement defaultFilter_;
  std::list<StationFilterElement> filters_;

  //void configDefaultFilter(const miutil::conf::ConfSection &conf);

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
