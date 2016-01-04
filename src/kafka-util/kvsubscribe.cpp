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

//#include "../HintSubscriber.h"
#include <kvsubscribe/DataSubscriber.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <iostream>
#include <memory>
#include <csignal>

using namespace kvalobs::subscribe;

namespace hint {
void up() {
  std::cout << "kvalobs coming up" << std::endl;
}
void down() {
  std::cout << "kvalobs going down" << std::endl;
}
}
namespace data {
void message(const kvalobs::serialize::KvalobsData & data) {
  std::cout << kvalobs::serialize::KvalobsDataSerializer::serialize(data)
            << std::endl;
}
}

void stopListening(int singal) {
  KafkaConsumer::stopAll();
}

void help(std::ostream & s, const std::string & applicationName) {
  s << "Usage: " << applicationName << " hint|data" << std::endl;
}
KafkaConsumer * getConsumer(const std::string & type,
                            const std::string & domain,
                            const std::string & brokers) {
  if (type == "hint") {  //return new HintSubscriber(hint::up, hint::down, brokers);
    throw std::logic_error("Not implemented consumer type: " + type);
  } else if (type == "data") {
    return new DataSubscriber(data::message, domain, DataSubscriber::Latest,
                              brokers);
  }

  throw std::logic_error("Invalid consumer type: " + type);
}

int main(int argc, char ** argv) {
  try {
    if (argc != 2) {
      help(std::clog, argv[0]);
      return 1;
    }
    std::string arg(argv[1]);

    std::unique_ptr<KafkaConsumer> consumer;

    if (arg == "--help") {
      help(std::cout, argv[0]);
      return 0;
    } else
      consumer.reset(getConsumer(arg, "test", "localhost"));

    signal(SIGINT, stopListening);
    signal(SIGTERM, stopListening);

    consumer->run();
  } catch (std::exception & e) {
    std::clog << e.what() << std::endl;
  }
}
