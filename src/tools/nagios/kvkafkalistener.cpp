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

#include <boost/program_options.hpp>
#include <kvsubscribe/DataSubscriber.h>
#include <csignal>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <thread>

namespace {
bool stop = false;
std::condition_variable condition;
std::mutex mutex;
typedef std::chrono::time_point<std::chrono::system_clock> Time;
std::set<Time> arrivals;
const Time programStartTime = std::chrono::system_clock::now();


void terminate(int signal) {
kvalobs::subscribe::KafkaConsumer::stopAll();
stop = true;
condition.notify_all();
}

void report(std::ostream & s, const std::string & identifier, const Time & since) {
  s << identifier << '\t';
  if ( since > programStartTime)
    s << std::distance(arrivals.lower_bound(since), arrivals.end());
  else
    s << '-';
  s << '\n';
}

void runReporter(const std::string & reportFile) {
  std::unique_lock<std::mutex> lock(mutex);
  while (!stop) {
    std::ofstream s(reportFile);
    auto now = std::chrono::system_clock::now();

    s << "seconds_since_last_data\t";
    if (arrivals.empty()) {
      s << '-';
    } else {
      auto lastArrival = * arrivals.crbegin();
      auto timeSinceLast = now - lastArrival;
      s << timeSinceLast.count() / 1000000000;
    }
    s << '\n';
    report(s, "last_1_minute", now - std::chrono::minutes(1));
    report(s, "last_15_minutes", now - std::chrono::minutes(15));
    report(s, "last_60_minutes", now - std::chrono::minutes(60));
    s << std::flush;

    condition.wait_for(lock, std::chrono::seconds(60));
  }
}

void newData(const ::kvalobs::serialize::KvalobsData & data) {
  auto now = std::chrono::system_clock::now();

  std::unique_lock<std::mutex> lock(mutex);
  arrivals.insert(now);

  // Clear old data
  arrivals.erase(arrivals.begin(), arrivals.upper_bound(now - std::chrono::minutes(60)));
}
}  // namespace

int main(int argc, char ** argv) {
  std::string reportFile;
  std::string domain;
  std::string server;

  using namespace boost::program_options;


  options_description opt;
  opt.add_options()
      ("output-file,o", value<std::string>(& reportFile)->default_value("kvalobs.kafka.status"), "Write reports to the given file")
      ("domain,d", value<std::string>(& domain)->default_value("test"), "Use the given kvalobs domain in kafka")
      ("server,s", value<std::string>(& server)->default_value("localhost"), "Test against the given server");

  options_description general;
  general.add_options()
      ("configuration", value<std::string>()->default_value("/etc/default/kvkafkalistener"), "Read additional config from the given file, if it exists")
      ("help", "Get this help message");

  options_description allOptions("Program options");
  allOptions.add(opt).add(general);

  parsed_options parsed = command_line_parser(argc, argv).options(allOptions).run();
  variables_map vm;
  store(parsed, vm);
  notify(vm);

  if (vm.count("help")) {
    std::cout << "Continuously send a report about kafka's kvalobs activity to the given file\n\n";
    std::cout << allOptions << std::endl;
    return 0;
  }

  if (vm.count("configuration")) {
    std::string configFile = vm["configuration"].as<std::string>();
    std::ifstream s(configFile);
    parsed_options parsedFromFile = parse_config_file(s, opt);
    store(parsedFromFile, vm);
    notify(vm);
  }

  kvalobs::subscribe::DataSubscriber subscriber(newData, domain, server);

  signal(SIGINT, terminate);
  signal(SIGHUP, terminate);

  std::thread t(runReporter, reportFile);
  subscriber.run();
  t.join();
}
