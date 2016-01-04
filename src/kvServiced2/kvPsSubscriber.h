/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvPsSubscriber.h,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#ifndef __kvPsSubscriber_h__
#define __kvPsSubscriber_h__

#include <kvalobs/kvDbBase.h>
#include <string>

/* Created by DNMI/IT: borge.moe@met.no Oct 13 2006 
 */

/**
 * \addtogroup  kvService
 *
 * @{
 */

/**
 * \brief Interface to the table ps_subscriber in the kvalobs database.
 * 
 * Each defined persisten subscriber will be defined in the table ps_subscribers 
 * with the following information.
 * 
 <pre>
 name          - the name of the subscriber. The name is unique.
 subscriberid  - subscriberid on the form ps_subscriber_name, where
 name is the same as the name above.
 comment       - An comment.
 delete_after_hours - How long ar an element allowed to wait before we
 delete it from the subscribers table. This is to 
 prevent tables that grow without limits.
 sior          - A stringified CORBA referance for the last callback that
 was registred for this subscriber.
 created       - When was this record created.
 </pre>
 * 
 * 
 * There will be one data table for every persistent subscriber defined in the table
 * ps_subscribers. The name of data table is ps_subscriber_name, where name is the 
 * same as for the subscriber in the ps_subscribers table. 
 */
class kvPsSubscriber : public kvalobs::kvDbBase {
 private:

  int stationID_;
  int typeID_;
  miutil::miTime obstime_;
  miutil::miTime tbtime_;
  miutil::miTime touched_;

  ///name_ is used to identify this subscribers subscriber table.
  std::string name_;

 public:
  kvPsSubscriber(const std::string &name = "")
      : name_(name) {
  }
  kvPsSubscriber(const dnmi::db::DRow &r) {
    set(r);
  }
  kvPsSubscriber(int stationID, int typeID, const miutil::miTime &obstime) {
    set(stationID, typeID, obstime);
  }

  bool set(int stationID, int typeID, const miutil::miTime &obstime);

  bool set(const dnmi::db::DRow&);

  char* tableName() const {
    return const_cast<char*>(std::string(std::string("ps_subscriber_") + name_)
        .c_str());
  }

  std::string name() const {
    return name_;
  }
  void name(const std::string &n) {
    name_ = n;
  }

  miutil::std::string toSend() const;
  miutil::std::string toUpdate() const;
  miutil::std::string uniqueKey() const;
  miutil::std::string createStatement() const;
  miutil::std::string deleteStatement() const;

  int stationID() const {
    return stationID_;
  }
  int typeID() const {
    return typeID_;
  }
  miutil::miTime obstime() const {
    return obstime_;
  }
  miutil::miTime tbtime() const {
    return tbtime_;
  }
  miutil::miTime touched() const {
    return touched_;
  }
};

/** @} */
#endif
