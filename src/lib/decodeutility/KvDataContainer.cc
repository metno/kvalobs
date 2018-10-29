/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kldata.h,v 1.1.2.3 2007/09/27 09:02:29 paule Exp $

 Copyright (C) 2007 met.no

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

#include "KvDataContainer.h"

using namespace kvalobs::serialize::internal;
using kvalobs::serialize::KvalobsData;
using boost::posix_time::ptime;
using std::map;
namespace decodeutility {

///Deletes kvData after it is consumed.
KvDataContainer::KvDataContainer(kvalobs::serialize::KvalobsData *kvData)
    : data_(kvData) {
}

KvDataContainer::KvDataContainer(const std::list<kvalobs::kvData> &d, const std::list<kvalobs::kvTextData> &td) {
  KvalobsData *t = new KvalobsData(d, td);
  data_=t;
}

KvDataContainer::~KvDataContainer() {
  delete data_;
}

int KvDataContainer::getTextData(TextData &textData,
                                 const boost::posix_time::ptime &tbtime) const {
  int n = 0;
  textData.clear();
  std::list<kvalobs::kvTextData> td;
  data_->data(td, true, tbtime);

  for (auto &&elem : td) {
    textData[elem.stationID()][elem.typeID()][elem.obstime()].push_back(elem);
    ++n;
  }

  return n;
}

int KvDataContainer::getData(Data &data,
                             const boost::posix_time::ptime &tbtime) const {
  int n = 0;
  std::list<kvalobs::kvData> d;
  data_->data(d, true, tbtime);
  data.clear();
  for (auto &&elem : d) {
    data[elem.stationID()][elem.typeID()][elem.obstime()].push_back(elem);
    ++n;
  }

  return n;
}

int KvDataContainer::get(TextData &textData, Data &data,
                         const boost::posix_time::ptime &tbtime) const {
  return getTextData(textData, tbtime) + getData(data, tbtime);
}

int KvDataContainer::getTextData(TextDataByObstime &textData, int stationid,
                                 int typeId,
                                 const boost::posix_time::ptime &tbtime) const {
  int n = 0;
  textData.clear();
  std::list<kvalobs::kvTextData> td;
  data_->data(td, true, tbtime);

  for (auto &&elem : td) {
    if (elem.stationID() != stationid || elem.typeID() != typeId)
      continue;

    textData[elem.obstime()].push_back(elem);
    ++n;
  }

  return n;
}

int KvDataContainer::getData(DataByObstime &data, int stationid, int typeId,
                             const boost::posix_time::ptime &tbtime) const {
  int n = 0;
  data.clear();
  std::list<kvalobs::kvData> d;
  data_->data(d, true, tbtime);

  for (auto &&elem : d) {
    if (elem.stationID() != stationid || elem.typeID() != typeId)
      continue;

    data[elem.obstime()].push_back(elem);
    ++n;
  }

  return n;
}

int KvDataContainer::get(DataByObstime &data, TextDataByObstime &textData,
                         int stationid, int typeId,
                         const boost::posix_time::ptime &tbtime) const {
  return getTextData(textData, stationid, typeId, tbtime)
      + getData(data, stationid, typeId, tbtime);
}

bool KvDataContainer::getData(kvalobs::kvData &data, int stationid, int typeId,
                              int paramid,
                              const boost::posix_time::ptime &obstime,
                              char sensor, int level) const {
  DataByObstime tmpData;

  if (getData(tmpData, stationid, typeId) == 0)
    return false;

  const DataList &theData = tmpData[obstime];

  for (DataList::const_iterator it = theData.begin(); it != theData.end();
      ++it) {
    if (it->paramID() == paramid && it->sensor() == sensor
        && it->level() == level) {
      data = *it;
      return true;
    }
  }

  return false;
}

bool KvDataContainer::getTextData(
    kvalobs::kvTextData &data, int stationid, int typeId, int paramid,
    const boost::posix_time::ptime &obstime) const {
  TextDataByObstime tmpData;

  if (getTextData(tmpData, stationid, typeId) == 0)
    return false;

  const TextDataList &theData = tmpData[obstime];

  for (TextDataList::const_iterator it = theData.begin(); it != theData.end();
      ++it) {
    if (it->paramID() == paramid) {
      data = *it;
      return true;
    }
  }

  return false;
}

///The total count in this collection
int KvDataContainer::count() const {
  Data data;
  TextData textData;

  return getData(data) + getTextData(textData);
}

///The total count of data for a specific stationid, typeid and obstime.
int KvDataContainer::count(int stationid, int typeId,
                           const boost::posix_time::ptime &obstime) const {
  DataByObstime data;
  TextDataByObstime textData;

  int n = getData(data, stationid, typeId)
      + getTextData(textData, stationid, typeId);

  if (obstime.is_special())
    return n;

  return data[obstime].size() + textData[obstime].size();
}

///Deletes kvData after it is consumed.
void KvDataContainer::set(kvalobs::serialize::KvalobsData *kvData_) {
  delete data_;
  data_ = kvData_;
}

KvDataContainer::StationInfoList KvDataContainer::stationInfos()const{
  map<int, map<int, int> > stInfo;
  StationInfoList ret;
  std::list<kvalobs::kvData> d;
  data_->data(d, false);

  for (auto &elem : d) {
    stInfo[elem.stationID()][elem.typeID()]++;
  }

  for( auto sid : stInfo) {
    for( auto tid : sid.second) {
      ret.push_front(StationInfo(sid.first, tid.first));
    }
  } 
  return ret;
}

}

