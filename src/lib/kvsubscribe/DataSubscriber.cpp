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
#include <decodeutility/kvalobsdata.h>
#include <decodeutility/kvalobsdataparser.h>
#include <milog/milog.h>
#include <iostream>
#include <fstream>
#include <mutex>
#include "queue.h"

namespace kvalobs {
namespace subscribe {

namespace {
void writeNoDebug(const std::string &message, const serialize::KvalobsData &d ) {
  return;
}
}

std::function<void(const std::string &message, const serialize::KvalobsData &d)> DataSubscriber::debugWriter=writeNoDebug;

DataSubscriber::DataSubscriber(Handler handler,
                               const std::string & domain,
                               const std::string & brokers,
                               const std::string &groupId)
    : KafkaConsumer(topic(domain), brokers, groupId),
      handler_(handler) {
}

std::string DataSubscriber::topic(const std::string & domain) {
  return queue::checked(domain);
}


void DataSubscriber::setDebugWriter(std::function<void(const std::string &message, const serialize::KvalobsData &d)> func){
  debugWriter=func;
}

void DataSubscriber::resetDebugWrite(){
  debugWriter=writeNoDebug;
}

void DataSubscriber::data(const char * msg, unsigned length) {
  std::string message(msg, length);
  serialize::KvalobsData d;
  serialize::KvalobsDataParser::parse(message, d);
  debugWriter( message, d);
  handler_(d);
}

void DataSubscriber::error(int code, const std::string & msg) {
  milog::LogContext context("DataSubscriber");
  LOGERROR(msg);
}

} /* namespace subscribe */
} /* namespace kvalobs */
