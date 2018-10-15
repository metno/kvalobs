/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2016 met.no

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

#include "ObservationComplete.h"
#include <thread>
#include <iostream>
#include <chrono>
#include <string>
#include "kvdb/kvdb.h"
#include "milog/milog.h"

bool ObservationComplete::stopped_ = false;

ObservationComplete::ObservationComplete(const std::string & connectString)
    : dbAccess_(connectString) {
}

ObservationComplete::~ObservationComplete() {
}

void ObservationComplete::operator()() {
  milog::LogContext context("ObservationComplete");

  while (!stopped()) {
    try {
      auto transaction = dbAccess_.transaction(true);
      DataIdentifier di = dbAccess_.nextDataToProcess();
      if (di.isValid()) {
        LOGDEBUG1(di);
        dbAccess_.addMissingData(di);
        transaction->commit();
      } else if (!stopped()) {
        transaction->rollback();
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    } catch (dnmi::db::SQLSerializeError & e) {
      LOGDEBUG(e.what());
    } catch (std::exception & e) {
      LOGERROR(e.what());
      // ...and continue as if nothing happened
    }
  }
}
