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

#ifndef SRC_KVQABASED_SRC_NEWDATALISTENER_H_
#define SRC_KVQABASED_SRC_NEWDATALISTENER_H_

#include "DataProcessor.h"
#include <boost/noncopyable.hpp>
#include <memory>
#include <string>
#include <list>
#include "kvalobs/kvStationInfo.h"

namespace kvalobs {
class kvStationInfo;
}
namespace db {
class DatabaseAccess;
}

namespace qabase {

class NewDataListener : boost::noncopyable {
 public:
  explicit NewDataListener(std::shared_ptr<db::DatabaseAccess> db, int selectForControlCount=5, bool kafkaEnabled=true);
  ~NewDataListener();

  void run();

  void stop();

  bool stopping() const;

  static void stopAll();

 private:
  typedef kvalobs::kvStationInfo StationInfo;
  typedef std::shared_ptr<kvalobs::kvStationInfo> StationInfoPtr;
  typedef std::shared_ptr<qabase::Observation> ObservationPtr;
  bool stopping_;
  std::shared_ptr<db::DatabaseAccess> db_;
  DataProcessor processor_;
  int selectForControlCount_;
  mutable std::list<Observation*> selectedForControl;
    
  Observation* fetchDataToProcess_() const;
  ObservationPtr fetchDataToProcess() const;
  qabase::CheckRunner::KvalobsDataPtr runChecks(const qabase::Observation & obs);
  void markProcessDone(const qabase::Observation & obs);
};

} /* namespace qabase */

#endif /* SRC_KVQABASED_SRC_NEWDATALISTENER_H_ */
