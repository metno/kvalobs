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

#include "FakeDatabaseAccess.h"
#include <db/returntypes/Observation.h>
#include <kvalobs/kvDataOperations.h>
#include <decodeutility/kvalobsdata.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <cstdio>

FakeDatabaseAccess::FakeDatabaseAccess() {
}

FakeDatabaseAccess::~FakeDatabaseAccess() {
}

void FakeDatabaseAccess::getChecks(CheckList * out,
                                   const qabase::Observation & obs) const {
  switch (obs.stationID()) {
    case 10:
      out->push_back(
          kvalobs::kvChecks(
              10, "QC1-2-101", "QC1-2", 1, "foo", "obs;RR_24;;", "* * * * *",
              boost::posix_time::time_from_string("2010-01-01 00:00:00")));
      break;
    case 20:
      out->push_back(
          kvalobs::kvChecks(
              20, "QC1-2-102", "QC1-2", 1, "bar", "obs;RR_24,TAM_24;;",
              "* * * * *",
              boost::posix_time::time_from_string("2010-01-01 00:00:00")));
      break;
  }
}

int FakeDatabaseAccess::getQcxFlagPosition(const std::string & qcx) const {
  int a, b, c;
  if (std::sscanf(qcx.c_str(), "QC%d-%d-%d", &a, &b, &c) < 2)
    return 0;
  return b;
}

void FakeDatabaseAccess::getParametersToCheck(
    ParameterList * out, const qabase::Observation & obs) const {
  out->insert("RR_24");
  out->insert("TAM_24");
}

kvalobs::kvAlgorithms FakeDatabaseAccess::getAlgorithm(
    const std::string & algorithmName) const {
  if (algorithmName == "foo") {
    std::string simpleScript = "sub check() {\n"
        " my @retvector;\n"
        " push(@retvector, \"X_0_0_flag\");\n"
        " push(@retvector, 4);\n"
        " my $numout = @retvector;\n"
        " return(@retvector,$numout);\n"
        "}\n";

    return kvalobs::kvAlgorithms(1, "foo", "obs;X;;", simpleScript);
  } else if (algorithmName == "bar") {
    std::string simpleScript = "sub check() {\n"
        " my @retvector;\n"
        " push(@retvector, \"X_0_0_flag\");\n"
        " push(@retvector, 1);\n"
        " push(@retvector, \"Y_0_0_flag\");\n"
        " push(@retvector, 2);\n"
        " my $numout = @retvector;\n"
        " return(@retvector,$numout);\n"
        "}\n";

    return kvalobs::kvAlgorithms(1, "bar", "obs;X,Y;;", simpleScript);
  } else
    throw std::exception();
}

std::string FakeDatabaseAccess::getStationParam(
    const kvalobs::kvStationInfo & si, const std::string & parameter,
    const std::string & qcx) const {
  return "max;min\n2.2;1.1";
}

kvalobs::kvStation FakeDatabaseAccess::getStation(int stationid) const {
  return kvalobs::kvStation(
      stationid,
      1.15,
      12.5,
      333,
      0,
      "Test Station",
      0,
      0,
      "",
      "",
      "",
      9,
      true,
      boost::posix_time::ptime(boost::gregorian::date(1900, 1, 1),
                               boost::posix_time::time_duration(0, 0, 0)));
}

void FakeDatabaseAccess::getModelData(
    ModelDataList * out, const kvalobs::kvStationInfo & si,
    const qabase::DataRequirement::Parameter & parameter,
    int minutesBackInTime) const {
  kvalobs::kvModelData m(si.stationID(), si.obstime(), 110, 0, 0, 42.1);
  out->push_back(m);
}

void FakeDatabaseAccess::getData(
    DataList * out, const qabase::Observation & obs,
    const qabase::DataRequirement::Parameter & parameter,
    int minuteOffset) const {
  kvalobs::kvDataFactory factory(obs.stationID(), obs.obstime(), obs.typeID());
  if (parameter.baseName() == "RR_24") {
    boost::posix_time::ptime t = obs.obstime();
    boost::posix_time::ptime last = t;
    t += boost::posix_time::minutes(minuteOffset);
    t += boost::posix_time::hours(
        (24 - ((t.time_of_day().hours() - 6) % 24)) % 24);
    for (; t <= last; t += boost::posix_time::hours(24))
      out->push_front(factory.getData(t.time_of_day().hours(), 110, t));  // value = observation hour
  } else if (parameter.baseName() == "RR_12") {
    boost::posix_time::ptime t = obs.obstime();
    boost::posix_time::ptime last = t;
    t += boost::posix_time::minutes(minuteOffset);
    t += boost::posix_time::hours(
        (12 - ((t.time_of_day().hours() - 6) % 12)) % 12);
    for (; t <= last; t += boost::posix_time::hours(12))
      out->push_front(factory.getData(t.time_of_day().hours(), 109, t));  // value = observation hour
  } else if (parameter.baseName() == "TAM_24") {
    kvalobs::kvDataFactory factory(obs.stationID(), obs.obstime(),
                                   obs.typeID() + 1);

    boost::posix_time::ptime t = obs.obstime();
    boost::posix_time::ptime last = t;
    t += boost::posix_time::minutes(minuteOffset);
    for (; t <= last; t += boost::posix_time::hours(1))
      ;
    out->push_front(factory.getData(t.time_of_day().hours(), 109, t));  // value = observation hour
  }

}

void FakeDatabaseAccess::getTextData(
    TextDataList * out, const qabase::Observation & obs,
    const qabase::DataRequirement::Parameter & parameter,
    int minuteOffset) const {
}

FakeDatabaseAccess::KvalobsDataPtr FakeDatabaseAccess::complete(
    const qabase::Observation & obs, const DataList & d,
    const TextDataList & td) const {
  return std::make_shared < kvalobs::serialize::KvalobsData > (d, td);
}


void FakeDatabaseAccess::write(const DataList & data) {
  for (DataList::const_iterator it = data.begin(); it != data.end(); ++it) {
    savedData.erase(*it);
    savedData.insert(*it);
  }
}

qabase::Observation * FakeDatabaseAccess::selectDataForControl() {
  return nullptr;
}
