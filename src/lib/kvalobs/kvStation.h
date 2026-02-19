/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvStation.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef _kvStation_h
#define _kvStation_h

#include <kvalobs/kvDbBase.h>

/* Created by DNMI/FoU/PU: j.schulze@met.no Aug 26 2002 */
/* Edited by T.Reite april 2004 */

namespace kvalobs {

/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table station in the kvalobs database.
 */

class kvStation : public kvDbBase {
 private:
  int stationid_;
  double lat_;
  double lon_;
  double height_;
  double maxspeed_;
  std::string name_;
  int wmonr_;
  int nationalnr_;
  std::string ICAOid_;
  std::string call_sign_;
  std::string stationstr_;
  int environmentid_;
  bool static_;
  boost::posix_time::ptime fromtime_;

 public:
  kvStation() {
  }
  kvStation(const dnmi::db::DRow& r) {
    set(r);
  }
  kvStation(int st, double la, double lo, double he, double max,
            const std::string& na, int wm, int nn, const std::string& ic,
            const std::string& ca, const std::string& ss, int environmentid,
            bool static_, const boost::posix_time::ptime& fromtime) {
    set(st, la, lo, he, max, na, wm, nn, ic, ca, ss, environmentid, static_,
        fromtime);
  }

  bool set(const dnmi::db::DRow&);

  bool set(int, double, double, double, double, const std::string&, int, int,
           const std::string&, const std::string&, const std::string&, int,
           bool, const boost::posix_time::ptime&);

  const char* tableName() const {
    return "station";
  }
  std::string toSend() const;
  std::string uniqueKey() const;

  int stationID() const {
    return stationid_;
  }
  double lat() const {
    return lat_;
  }
  double lon() const {
    return lon_;
  }
  double height() const {
    return height_;
  }
  double maxspeed() const {
    return maxspeed_;
  }
  const std::string & name() const {
    return name_;
  }
  int wmonr() const {
    return wmonr_;
  }
  int nationalnr() const {
    return nationalnr_;
  }
  const std::string & ICAOID() const {
    return ICAOid_;
  }
  const std::string & call_sign() const {
    return call_sign_;
  }
  const std::string & stationstr() const {
    return stationstr_;
  }
  int environmentid() const {
    return environmentid_;
  }
  bool _static() const {
    return static_;
  }
  const boost::posix_time::ptime & fromtime() const {
    return fromtime_;
  }
};

/** @} */
}

#endif

