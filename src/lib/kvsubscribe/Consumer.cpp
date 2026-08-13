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
#include "Consumer.h"
#include <iostream>
#include <stdexcept>
#include <thread>

namespace kvalobs {
namespace subscribe {

std::list<Consumer *> Consumer::allConsumers_;

namespace {
class NullDataHandler : public ConsumerDataHandler {
public:
  void data(const char *msg, unsigned length) override {
    // Do nothing
  }

  void error(int code, const std::string &msg) override {
    // Do nothing
  }
};
NullDataHandler nullHandler;
} // namespace

Consumer::Consumer(const std::string &topic, ConsumerDataHandler *handler)
    : stopping_(false), handler_(&nullHandler), topic_(topic) {
  if (handler != nullptr) {
    handler_ = handler;
  } else {
    handler_ = &nullHandler;
  }
  allConsumers_.push_back(this);
}

Consumer::Consumer(const std::string &topic) : Consumer(topic, &nullHandler) {}

Consumer::~Consumer() {
  stop();
  allConsumers_.remove(this);
}

ConsumerDataHandler *Consumer::setHandler(ConsumerDataHandler *handler) {
  ConsumerDataHandler *old = handler_;
  handler_ = handler;
  return old;
}

ConsumerDataHandler *Consumer::getHandler() const { return handler_; }

std::string Consumer::getTopic() const { return topic_; }

void Consumer::run() {
  while (!stopping()) {
    runOnce(1000);
  }
}



void Consumer::stopAll() {
  for (auto consumer : allConsumers_) {
    consumer->stop();
  }

  // Wait for all consumers to stop
  for (auto consumer : allConsumers_) {
    while (!consumer->stopping()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  allConsumers_.clear();
}

void Consumer::handleData(const char *msg, unsigned length) {
  handler_->data(msg, length);
}

void Consumer::handleError(int code, const std::string &msg) {
  handler_->error(code, msg);
}

} // namespace subscribe
} // namespace kvalobs
