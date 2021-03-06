/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvWorkelement.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvWorkqueue_h__
#define __kvWorkqueue_h__

#include <kvalobs/kvDbBase.h>

/*
 * Created by DNMI/IT: borge.moe@met.no
 * at Mon Sep 20 13:25:16 2004
 */

namespace kvalobs {

/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table workque and workstatistik in the kvalobs database.
 */

class kvWorkque : public kvDbBase {
 private:
  long observationid_;
  int priority_;
  boost::posix_time::ptime process_start_;
  boost::posix_time::ptime qa_start_;
  boost::posix_time::ptime qa_stop_;
  boost::posix_time::ptime service_start_;
  boost::posix_time::ptime service_stop_;

  void createSortIndex();

 public:
  kvWorkque() {
  }
  kvWorkque(const dnmi::db::DRow &r) {
    set(r);
  }
  kvWorkque(long observationid, int pri,
                const boost::posix_time::ptime &process_start,
                const boost::posix_time::ptime &qa_start,
                const boost::posix_time::ptime &qa_stop,
                const boost::posix_time::ptime &service_start,
                const boost::posix_time::ptime &service_stop) {
    set(observationid, pri, process_start, qa_start, qa_stop,
        service_start, service_stop);
  }

  bool valid() const {
    return !sortBy_.empty();
  }

  kvWorkque(const kvWorkque &we);

  kvWorkque& operator=(const kvWorkque &rhs);

  bool set(long  observationid, int pri,
           const boost::posix_time::ptime &process_start,
           const boost::posix_time::ptime &qa_start,
           const boost::posix_time::ptime &qa_stop,
           const boost::posix_time::ptime &service_start,
           const boost::posix_time::ptime &service_stop);

  bool set(const dnmi::db::DRow&);

  const char* tableName() const {
    return "workque";
  }
  std::string toSend() const;
  std::string toUpdate() const;
  std::string uniqueKey() const;

  int observationID() const {
    return observationid_;
  }

  int priority() const {
    return priority_;
  }
  const boost::posix_time::ptime & process_start() const {
    return process_start_;
  }

  const boost::posix_time::ptime & qa_start() const {
    return qa_start_;
  }

  const boost::posix_time::ptime & qa_stop() const {
    return qa_stop_;
  }

  const boost::posix_time::ptime & service_start() const {
    return service_start_;
  }

  const boost::posix_time::ptime & service_stop() const {
    return service_stop_;
  }

  void process_start(const boost::posix_time::ptime &start);
  void qa_start(const boost::posix_time::ptime &start);
  void qa_stop(const boost::posix_time::ptime &stop);
  void service_start(const boost::posix_time::ptime &start);
  void service_stop(const boost::posix_time::ptime &stop);
};

/** @} */
}
;
#endif
