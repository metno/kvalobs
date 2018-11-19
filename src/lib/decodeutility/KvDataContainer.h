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
#ifndef __decodeutility_KVDATACONTAINER_H__
#define __decodeutility_KVDATACONTAINER_H__

#include <string>
#include <map>
#include <list>
#include <boost/date_time/posix_time/ptime.hpp>
#include <decodeutility/kvalobsdata.h>

namespace decodeutility {

class KvDataContainer {

 public:
  typedef std::list<kvalobs::kvTextData> TextDataList;
  typedef std::list<kvalobs::kvData> DataList;
  ///TextData and Data maps [stationid][typeid][obstime]
  typedef std::map<int,
      std::map<int, std::map<boost::posix_time::ptime, TextDataList> > > TextData;
  typedef std::map<int,
      std::map<int, std::map<boost::posix_time::ptime, DataList> > > Data;
  typedef std::map<boost::posix_time::ptime, TextDataList> TextDataByObstime;
  typedef std::map<boost::posix_time::ptime, DataList> DataByObstime;

  struct StationInfo{
      int stationId;
      int typeId;
      StationInfo(int sid, int tid):
      stationId(sid), typeId(tid){}
  };

  typedef std::list<StationInfo> StationInfoList;

  KvDataContainer()
      : data_(0) {
  }

  KvDataContainer(const std::list<kvalobs::kvData> &d, const std::list<kvalobs::kvTextData> &td); 
  ///Deletes kvData after it is consumed.
  KvDataContainer(kvalobs::serialize::KvalobsData *kvData);
  ~KvDataContainer();

  int getTextData(TextData &textData, const boost::posix_time::ptime &tbtime =
                      boost::posix_time::ptime()) const;
  int getData(Data &data, const boost::posix_time::ptime &tbtime =
                  boost::posix_time::ptime()) const;
  int get(TextData &textData, Data &data,
          const boost::posix_time::ptime &tbtime =
              boost::posix_time::ptime()) const;

  int getTextData(TextDataByObstime &textData, int stationid, int typeId,
                  const boost::posix_time::ptime &tbtime =
                      boost::posix_time::ptime()) const;
  int getData(DataByObstime &data, int stationid, int typeId,
              const boost::posix_time::ptime &tbtime =
                  boost::posix_time::ptime()) const;

  int get(DataByObstime &data, TextDataByObstime &textData, int stationid,
          int typeId, const boost::posix_time::ptime &tbtime =
              boost::posix_time::ptime()) const;

  bool getData(kvalobs::kvData &data, int stationid, int typeId, int paramid,
               const boost::posix_time::ptime &obstime, char sensor = '0',
               int level = 0) const;
  bool getTextData(kvalobs::kvTextData &data, int stationid, int typeId,
                   int paramid, const boost::posix_time::ptime &obstime) const;

  ///The total count in this collection
  int count() const;

  ///The total count of data for a specific stationid, typeid and obstime.
  int count(int stationid, int typeId, const boost::posix_time::ptime &obstime =
                boost::posix_time::ptime()) const;

  ///Deletes kvData after it is consumed.
  void set(kvalobs::serialize::KvalobsData *kvData);

  StationInfoList stationInfos()const;

 private:
  kvalobs::serialize::KvalobsData *data_;
};
}

#endif
