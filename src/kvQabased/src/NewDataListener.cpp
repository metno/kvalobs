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
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include "db/KvalobsDatabaseAccess.h"
#include "kvalobs/kvStationInfo.h"
#include "CheckRunner.h"
#include "QaBaseApp.h"

namespace qabase {

namespace {
bool globalStop = false;
bool processingCompleted = false;
}

NewDataListener::NewDataListener(std::shared_ptr<db::DatabaseAccess> db)
    : stopping_(true),
      db_(db),
      processor_(CheckRunner::create(QaBaseApp::createConnectString())) {
  // NOOP
}

NewDataListener::~NewDataListener() {
  // NOOP
}

void NewDataListener::run() {
  stopping_ = false;
  while (!stopping()) {
    try {
      qabase::NewDataListener::StationInfoPtr toProcess = fetchDataToProcess();
      // Fetch next data to process
      if (toProcess) {
        startProcessing();
        qabase::CheckRunner::DataListPtr dataList = runChecks(*toProcess);
        if (!dataList->empty()) {
          bool dataSent = false;
          int sendAttempts = 0;
          while (!dataSent) {
            processor_.sendToKafka(dataList);
            if (isProcessingCompleted()) {
              dataSent = true;
              markProcessDone(*toProcess);
              if (sendAttempts > 0)
                LOGWARN("Successfully sent data after " << sendAttempts << " retries");
            } else {
              if (sendAttempts == 0)
                LOGWARN("Could not send data to Kafka. Retrying...");
              sendAttempts++;
              std::this_thread::sleep_for(std::chrono::seconds(1));
            }
          }
        } else {
          LOGINFO("DataList from runChecks() is empty; nothing to send to Kafka. Marking done and continuing...");
          markProcessDone(*toProcess);
        }
      } else {
        // Did not find any data to process. Pausing for 1 second. This is normal.
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    } catch (std::exception & e) {
      LOGERROR("Unhandled exception in NewDataListener::run: " << e.what());
    }
  }
}

qabase::NewDataListener::StationInfoPtr NewDataListener::fetchDataToProcess() const {
  while (true) {
    try {
      db_->beginTransaction();
      qabase::NewDataListener::StationInfoPtr ret(db_->selectDataForControl());
      db_->commit();
      return ret;
    } catch (dnmi::db::SQLSerializeError & e) {
      db_->rollback();
      LOGDEBUG("Serialization error on fetchDataToProcess: " << e.what());
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
}

qabase::CheckRunner::DataListPtr NewDataListener::runChecks(const qabase::NewDataListener::StationInfo & toProcess) const {
  db_->beginTransaction();
  qabase::CheckRunner::DataListPtr dataList = processor_.runChecks(toProcess);
  db_->commit();
  return dataList;
}

void NewDataListener::markProcessDone(const qabase::NewDataListener::StationInfo &toProcess) {
  while (true) {
    try {
      db_->beginTransaction();
      db_->markProcessDone(toProcess);
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
  return stopping_ or globalStop;
}

void NewDataListener::stopAll() {
  globalStop = true;
}

void NewDataListener::startProcessing() {
  processingCompleted = false;
}

bool NewDataListener::isProcessingCompleted() const {
  return processingCompleted;
}

void NewDataListener::onKafkaSendSuccess(kvalobs::subscribe::KafkaProducer::MessageId id, const std::string & data) {
  LOGDEBUG("Successfully sent data: <" + data + ">");
  processingCompleted = true;
}

void NewDataListener::onKafkaSendError(kvalobs::subscribe::KafkaProducer::MessageId id, const std::string & data, const std::string & errorMessage) {
  LOGDEBUG("Unable to send data: " + errorMessage + "  Data: <" + data + ">");
}

} /* namespace qabase */
