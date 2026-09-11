/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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

#include "DataSubscriber.h"
#include "queue.h"
#include <decodeutility/kvalobsdata.h>
#include <decodeutility/kvalobsdataparser.h>
#include <fstream>
#include <iostream>
#include <milog/milog.h>
#include <mutex>

namespace kvalobs {
namespace subscribe {

namespace {
void writeNoDebug(const std::string &message, const serialize::KvalobsData &d) {
  return;
}

class DataHandler : public ConsumerDataHandler {
public:
  DataHandler(DataSubscriber::Handler handler) : handler_(handler) {}
  virtual void data(const char *msg, unsigned length) override {
    std::string message(msg, length);
    serialize::KvalobsData d;
    serialize::KvalobsDataParser::parse(message, d);
    DataSubscriber::getDebugWriter()(message, d);
    handler_(d);
  };

  /**
   * Handle errors on message arrival.
   */
  virtual void error(int code, const std::string &msg) override {
    // Default implementation: log the error
    milog::LogContext context("DataHandler");
    LOGERROR(msg);
  }
private:
  DataSubscriber::Handler handler_;
};
} // namespace

std::function<void(const std::string &message, const serialize::KvalobsData &d)>
    DataSubscriber::debugWriter = writeNoDebug;

DataSubscriber::DataSubscriber(Handler handler, Consumer *consumer)
    : Consumer("", nullptr), handler_(handler), consumer_(consumer), ownsConsumer_(false) {
      DataHandler *dataHandler = new DataHandler(handler_);
      consumer_->setHandler(dataHandler);
    }


DataSubscriber::~DataSubscriber() {
  if (ownsConsumer_ && consumer_) {
    delete consumer_;
    consumer_ = nullptr;
  }
}

std::string DataSubscriber::topic(const std::string &domain) {
  return queue::checked(domain);
}

// Consumer interface implementation
void DataSubscriber::run() {
  if (consumer_) {
    consumer_->run();
  }
}

bool DataSubscriber::stopping() const {
  if (consumer_) {
    return consumer_->stopping();
  }
  return false;
}

void DataSubscriber::stop() {
  if (consumer_) {
    consumer_->stop();
  }
}

void DataSubscriber::runOnce(unsigned timeoutInMilliSeconds) {
  // Delegate to the wrapped consumer
  // This method is called by the base Consumer::run() implementation
  if (consumer_) {
    consumer_->runOnce(timeoutInMilliSeconds);
  }
}

void DataSubscriber::setDebugWriter(
    std::function<void(const std::string &message,
                       const serialize::KvalobsData &d)>
        func) {
  debugWriter = func;
}

void DataSubscriber::resetDebugWrite() { debugWriter = writeNoDebug; }

} /* namespace subscribe */
} /* namespace kvalobs */
