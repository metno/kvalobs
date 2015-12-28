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

#include "NewDataListener.h"
#include "CheckRunner.h"
#include "QaBaseApp.h"
#include <kvalobs/kvStationInfo.h>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>
#include <thread>
#include <chrono>

namespace qabase {

namespace
{
bool globalStop = false;
}


NewDataListener::NewDataListener(std::shared_ptr<db::DatabaseAccess> db)
    : stopping_(true),
      db_(db),
      processor_(CheckRunner::create(QaBaseApp::createConnectString())) {
}

NewDataListener::~NewDataListener() {
}

void NewDataListener::run() {

  stopping_ = false;

  while (not stopping()) {
    db_->beginTransaction();
    try {
      std::unique_ptr<kvalobs::kvStationInfo> toProcess(db_->selectDataForControl());
      db_->commit();

      if (toProcess) {
        db_->beginTransaction();
        processor_.process(*toProcess);
        db_->markProcessDone(*toProcess);
        db_->commit();
      }
      else
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (std::exception & e) {
      LOGERROR(e.what());
      db_->rollback();
    }
  }
}

void NewDataListener::stop() {
  stopping_ = true;
}

bool NewDataListener::stopping() const {
  return stopping_ or globalStop;
}

void NewDataListener::stopAll()
{
  globalStop = true;
}

} /* namespace qabase */
