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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kvsubscribe/DataSubscriber.h>
#include <decodeutility/kvalobsdata.h>
#include <decodeutility/kvalobsdataserializer.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <list>

using kvalobs::subscribe::DataSubscriber;
using boost::program_options::value;

void handle_full(const kvalobs::serialize::KvalobsData & d) {
  std::cout << kvalobs::serialize::KvalobsDataSerializer::serialize(d) << std::endl;
}

namespace {

const char QC1 = 1;
const char QC2 = 2;
const char HQC = 4;

/**
 * return bit-mask:
 * 1 = qc1
 * 2 = qc2
 * 4 = hqc
 */
int modifications(const kvalobs::kvData & d) {
  switch (d.useinfo().flag(0)) {
    case 0: return 0;
    case 1: return QC1|QC2|HQC;
    case 2: return QC2|HQC;
    case 3: return QC1|HQC;
    case 4: return HQC;
    case 5: return QC1|QC2;
    case 6: return QC2;
    case 7: return QC1;
    default: return 0;
  }
}

int modifications(const kvalobs::serialize::KvalobsData & kvd, const kvalobs::kvStationInfo & si) {
  std::list<kvalobs::kvData> dl;
  kvd.getData(dl);
  int ret = 0;
  for (const kvalobs::kvData & d : dl)
    if (d.stationID() == si.stationID() && d.typeID() == si.typeID() && d.obstime() == si.obstime())
      ret |= modifications(d);
  return ret;
}

std::string modificationString(int mods) {
  std::string ret;
  bool first = true;
  if ( mods & QC1 ) {
    ret = "QC1";
    first = false;
  }
  if ( mods & QC2 ) {
    if (!first)
      ret += ',';
    ret += "QC2";
    first = false;
  }
  if ( mods & HQC ) {
    if (!first)
      ret += ',';
    ret += "QC1 ";
  }
  return ret;
}

}  // namespace

void handle_summary(const kvalobs::serialize::KvalobsData & d) {
  for (const kvalobs::kvStationInfo & st : d.summary()) {
    std::cout << st.stationID() << '\t' << st.typeID() << '\t' << st.obstime();
    int mods = modifications(d, st);
    if (mods)
      std::cout << '\t' << modificationString(mods);
    std::cout << '\n';
  }
  std::cout << std::flush;
}

void version(std::ostream & s) {
  s << "kvsubscribe (" << PACKAGE_VERSION << ")\n";
}

void help(const boost::program_options::options_description & opt, std::ostream & s) {
  version(s);
  s << "\nList data from kvalobs in real time\n\n";
  s << opt << std::endl;
}

int main(int argc, char ** argv) {
  std::string domain;
  std::string host;
  std::string progressFile;
  auto handlerFunction = handle_summary;

  boost::program_options::options_description opt("Command-line options");
  opt.add_options()
      ("host,h", value<std::string>(& host)->default_value("localhost"), "Host(s) to connect to")
      ("domain,d", value<std::string>(& domain)->default_value("test"), "Domain name to use")
      ("full", "List all data, instead of summaries")
      ("all-available", "Instead of starting with data from now on, get all available data from queue, even back in time")
      ("progress-file", value<std::string>(&progressFile), "Save progress in the given file, so you may restart at the point you left at a later time")
      ("version", "Show version information, and exit")
      ("help", "Show this help message, and exit");

  try {
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
      help(opt, std::cout);
      return 0;
    }

    if (vm.count("version")) {
      version(std::cout);
      return 0;
    }

    if (vm.count("full"))
      handlerFunction = handle_full;

    DataSubscriber subscriber(handlerFunction, domain, host);
    if (vm.count("all-available"))
      subscriber.startAtEarliestData();
    if (!progressFile.empty())
      subscriber.startAtStored(progressFile);
    subscriber.run();
  }
  catch (boost::program_options::unknown_option &) {
    std::clog << "Invalid option.\n\n";
    help(opt, std::clog);
    return 1;
  }
  catch (std::exception & e) {
    std::clog << e.what() << std::endl;
    return 1;
  }
}
