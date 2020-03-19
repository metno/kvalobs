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
#include "db/returntypes/Observation.h"
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <cstdlib>
#include "db/KvalobsDatabaseAccess.h"
#include "kvalobs/kvStationInfo.h"
#include "decodeutility/kvalobsdata.h"
#include "CheckRunner.h"
#include "QaBaseApp.h"
#include "TransactionLogger.h"

namespace qabase {

namespace {
std::set<NewDataListener*> listeners;
}

NewDataListener::NewDataListener(std::shared_ptr<db::DatabaseAccess> db, int selectForControlCount)
    : stopping_(true),
      db_(db),
      processor_(CheckRunner::create(QaBaseApp::createConnectString())),
      selectForControlCount_(selectForControlCount) {
  listeners.insert(this);
}

NewDataListener::~NewDataListener() {
  listeners.erase(this);
}

void NewDataListener::run() {
  stopping_ = false;
  while (!stopping()) {
    try {
      qabase::NewDataListener::ObservationPtr toProcess = fetchDataToProcess();
      if (toProcess) {
        qabase::TransactionLogger logger(*toProcess);
        qabase::CheckRunner::KvalobsDataPtr dataList = runChecks(*toProcess);
        markProcessDone(*toProcess);
        logger.markSuccess();
      } else {
        // Did not find any data to process. Pausing for 1 second. This is normal.
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    } catch (std::exception & e) {
      LOGERROR("Unhandled exception in NewDataListener::run: " << e.what());
    }
  }
}


//Select selectForControllCount_ observations from workque. Return the
//first observation and save the rest in the list selectedForControl.
Observation* NewDataListener::fetchDataToProcess_() const {
  while (!stopping()) {
    try {
      auto obs=db_->selectDataForControl(selectForControlCount_);
      auto it=obs.begin();

      if ( it == obs.end() )
        return nullptr;

      auto ret = *it;

      for( ++it; it != obs.end(); ++it ) {
        selectedForControl.push_back(*it);
      }

      { //DEBUG
        std::ostringstream ost;
        ost << "FetchDataForControl_: # " << selectedForControl.size()+1 << "\n"
            << ret->id() << " (" << ret->stationID() << "/" << ret->typeID() << "/" << ret->obstime() << ")\n"; 

        for( auto p : selectedForControl) {
          ost <<p->id() << " (" << p->stationID() << "/" << p->typeID() << "/" << p->obstime() << ")\n"; 
        }
        LOGINFO(ost.str());
      } //END DEBUG

      return ret;
    } catch (dnmi::db::SQLSerializeError & e) {
      db_->rollback();
      LOGDEBUG("Serialization error on fetchDataToProcess: " << e.what());
      std::this_thread::sleep_for(std::chrono::milliseconds(25 + (std::rand() % 50)));
    }
  }
  return nullptr;
}

qabase::NewDataListener::ObservationPtr NewDataListener::fetchDataToProcess() const 
{
  Observation  *ret;
  if ( selectedForControl.begin() != selectedForControl.end() ) {
    ret=selectedForControl.front();
    selectedForControl.pop_front();
  } else {
    ret=fetchDataToProcess_();
  }
  return ObservationPtr(ret);
}


qabase::CheckRunner::KvalobsDataPtr NewDataListener::runChecks(const qabase::Observation & obs) {
  try {
    db_->beginTransaction();
    qabase::CheckRunner::KvalobsDataPtr dataList = processor_.runChecks(obs);
    db_->commit();
    processor_.sendToKafka(obs, dataList, & stopping_);
    return dataList;
  }
  catch (...) {
    db_->rollback();
    throw;
  }
}

void NewDataListener::markProcessDone(const qabase::Observation & obs) {
  while (true) {
    try {
      db_->beginTransaction();
      db_->markProcessDone(obs);
      db_->commit();
      return;
    } catch (dnmi::db::SQLSerializeError & e) {
      db_->rollback();
      LOGDEBUG("Serialization error on markProcessDone: " << e.what());
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
}

void NewDataListener::stop() {
  stopping_ = true;
}

bool NewDataListener::stopping() const {
  return stopping_;
}

void NewDataListener::stopAll() {
  std::cout << "stopAll!" << std::endl;
  for (NewDataListener * listener : listeners)
    listener->stop();
}


} /* namespace qabase */
