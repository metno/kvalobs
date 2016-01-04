/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: DataTblView.h,v 1.1.2.3 2007/09/27 09:02:48 paule Exp $                                                       

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
#ifndef __DATATBLVIEW_H__
#define __DATATBLVIEW_H__

#include <kvalobs/kvDbBase.h>
#include <puTools/miTime.h>
#include <kvdb/kvdb.h>

/// What is this?
class DataTblView : public kvalobs::kvDbBase {
  long stationid_;
  long typeid_;
  miutil::miTime obstime_;

 public:

  DataTblView()
      : stationid_(0),
        typeid_(0) {
  }
  DataTblView(const dnmi::db::DRow &r) {
    set(r);
  }
  DataTblView(const DataTblView &dtv)
      : stationid_(dtv.stationid_),
        typeid_(dtv.typeid_),
        obstime_(dtv.obstime_) {
  }

  DataTblView& operator=(const DataTblView &rhs) {
    if (this != &rhs) {
      stationid_ = rhs.stationid_;
      typeid_ = rhs.typeid_;
      obstime_ = rhs.obstime_;
    }

    return *this;
  }

  bool set(const dnmi::db::DRow&);

  /** Not implemented, not needed!*/
  std::string toSend() const;

  /** Not implemented, not needed!*/
  std::string uniqueKey() const;

  const char* tableName() const;

  bool exist() const {
    return obstime_.undef();
  }

  long typeID() const {
    return typeid_;
  }
  long stationID() const {
    return stationid_;
  }
  miutil::miTime obstime() const {
    return obstime_;
  }
};

#endif /*DATATBLVIEW_H_*/
