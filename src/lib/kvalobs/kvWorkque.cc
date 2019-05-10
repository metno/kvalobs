/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvWorkque.cc,v 1.2.2.3 2007/09/27 09:02:31 paule Exp $

 Copyright (C) 2007 met.no

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
#include <kvalobs/kvWorkque.h>
#include <miutil/timeconvert.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace dnmi;

void kvalobs::kvWorkque::createSortIndex() {
  std::ostringstream s;
  s << std::setfill('0') << std::setw(3) << priority_ << std::setw(20) << observationid_;
  
  sortBy_ = s.str();
}

kvalobs::kvWorkque::kvWorkque(const kvWorkque &we)
    : observationid_(we.observationid_),
      priority_(we.priority_),
      process_start_(we.process_start_),
      qa_start_(we.qa_start_),
      qa_stop_(we.qa_stop_),
      service_start_(we.service_start_),
      service_stop_(we.service_stop_)

{
  createSortIndex();
}

kvalobs::kvWorkque&
kvalobs::kvWorkque::operator=(const kvWorkque &rhs) {
  if (&rhs != this) {
    observationid_ = rhs.observationid_;
    priority_ = rhs.priority_;
    process_start_ = rhs.process_start_;
    qa_start_ = rhs.qa_start_;
    qa_stop_ = rhs.qa_stop_;
    service_start_ = rhs.service_start_;
    service_stop_ = rhs.service_stop_;

    createSortIndex();
  }

  return *this;
}

bool kvalobs::kvWorkque::set(long observationid, int pri,
                                 const boost::posix_time::ptime &process_start,
                                 const boost::posix_time::ptime &qa_start,
                                 const boost::posix_time::ptime &qa_stop,
                                 const boost::posix_time::ptime &service_start,
                                 const boost::posix_time::ptime &service_stop) {
  observationid_ = observationid;
  priority_ = pri;
  process_start_ = process_start;
  qa_start_ = qa_start;
  qa_stop_ = qa_stop;
  service_start_ = service_start;
  service_stop_ = service_stop;

  createSortIndex();
}

bool kvalobs::kvWorkque::set(const dnmi::db::DRow &r_) {
  db::DRow &r = const_cast<db::DRow&>(r_);
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];

      if (*it == "observationid") {
        observationid_ = atol(buf.c_str());
      } else if (*it == "priority") {
        priority_ = atoi(buf.c_str());
      } else if (*it == "process_start") {
        if (!buf.empty())
          process_start_ = boost::posix_time::time_from_string_nothrow(buf);
      } else if (*it == "qa_start") {
        if (!buf.empty())
          qa_start_ = boost::posix_time::time_from_string_nothrow(buf);
      } else if (*it == "qa_stop") {
        if (!buf.empty())
          qa_stop_ = boost::posix_time::time_from_string_nothrow(buf);
      } else if (*it == "service_start") {
        if (!buf.empty())
          service_start_ = boost::posix_time::time_from_string_nothrow(buf);
      } else if (*it == "service_stop") {
        if (!buf.empty())
          service_stop_ = boost::posix_time::time_from_string_nothrow(buf);
      } else {
        CERR("kvWorkque::set .. unknown entry:" << *it << std::endl);
      }
    } catch (...) {
      CERR("kvWorkque: unexpected exception ..... \n");
    }
  }

  createSortIndex();
  return true;

}

std::string kvalobs::kvWorkque::toSend() const {
  ostringstream ost;

  ost << "(" <<  priority_ << ","
      << (process_start_.is_not_a_date_time() ? "NULL" : quoted(process_start_))
      << "," << (qa_start_.is_not_a_date_time() ? "NULL" : quoted(qa_start_))
      << "," << (qa_stop_.is_not_a_date_time() ? "NULL" : quoted(qa_stop_))
      << "," << (service_start_.is_not_a_date_time() ? "NULL" : quoted(service_start_))
      << "," << (service_stop_.is_not_a_date_time() ? "NULL" : quoted(service_stop_)) 
      << "," << observationid_
      << ")";

  return ost.str();
}

std::string kvalobs::kvWorkque::toUpdate() const {
  ostringstream ost;
  bool comma = false;

  ost << "SET ";

  if (!process_start_.is_not_a_date_time()) {
    comma = true;
    ost << "process_start=" << quoted(process_start_);
  }

  if (!qa_start_.is_not_a_date_time()) {
    if (comma)
      ost << ", ";

    comma = true;
    ost << "qa_start=" << quoted(qa_start_);
  }

  if (!qa_stop_.is_not_a_date_time()) {
    if (comma)
      ost << ", ";

    comma = true;
    ost << "qa_stop=" << quoted(qa_stop_);
  }

  if (!service_start_.is_not_a_date_time()) {
    if (comma)
      ost << ", ";

    comma = true;
    ost << "service_start=" << quoted(service_start_);
  }

  if (!service_stop_.is_not_a_date_time()) {
    if (comma)
      ost << ", ";

    comma = true;
    ost << "service_stop=" << quoted(service_stop_);
  }

  ost << " WHERE observationid=" << observationid_;

  return ost.str();

}

std::string kvalobs::kvWorkque::uniqueKey() const {
  ostringstream ost;

  ost << " WHERE observationid=" << observationid_;

  return ost.str();
}

void kvalobs::kvWorkque::process_start(
    const boost::posix_time::ptime &start) {
  process_start_ = start;
}

void kvalobs::kvWorkque::qa_start(const boost::posix_time::ptime &start) {
  qa_start_ = start;
}

void kvalobs::kvWorkque::qa_stop(const boost::posix_time::ptime &stop) {
  qa_stop_ = stop;
}

void kvalobs::kvWorkque::service_start(
    const boost::posix_time::ptime &start) {
  service_start_ = start;
}

void kvalobs::kvWorkque::service_stop(
    const boost::posix_time::ptime &stop) {
  service_stop_ = stop;
}

/*
INSERT INTO workstatistik SELECT
o.stationid,
  o.obstime,
o.typeid,
  o.tbtime,
  q.priority,
  q.process_start,
  q.qa_start,
  q.qa_stop,
  q.service_start,
  q.service_stop,
  q.observationid
from workque q, observations o
WHERE q.observationid=o.observationid AND q.qa_stop IS NOT NULL AND (SELECT count(*) FROM workstatistik s WHERE q.observationid=s.observationid)=0;




 stationid     | integer                     | not null
 obstime       | timestamp without time zone | not null
 typeid        | integer                     | not null
 tbtime        | timestamp without time zone | not null
 priority      | integer                     | not null
 process_start | timestamp without time zone | 
 qa_start      | timestamp without time zone | 
 qa_stop       | timestamp without time zone | 
 service_start | timestamp without time zone | 
 service_stop  | timestamp without time zone | 
 observationid | bigint                      | 

SELECT
	o.stationid,
  o.obstime,
	o.typeid,
  o.tbtime,
  q.priority,
  q.process_start,
  q.qa_start,
  q.qa_stop,
  q.service_start,
  q.service_stop,
  q.observationid
from workque q, observations o
WHERE q.observationid=o.observationid AND (SELECT count(*) FROM workstatistik s WHERE q.observationid=s.observationid)==0;
order by priority, tbtime ;    



*/