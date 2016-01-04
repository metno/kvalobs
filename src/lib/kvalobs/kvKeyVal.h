/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvKeyVal.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvKeyVal_h__
#define __kvKeyVal_h__

#include <kvalobs/kvDbBase.h>

namespace kvalobs {
/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table keyval in the kvalobs database.
 */

class kvKeyVal : public kvDbBase {
 private:
  std::string package_;
  std::string key_;
  std::string val_;

  void createSortIndex();

 public:
  kvKeyVal() {
    clean();
  }
  kvKeyVal(const kvKeyVal &keyVal) {
    set(keyVal);
  }
  kvKeyVal(const dnmi::db::DRow &r) {
    set(r);
  }
  kvKeyVal(const std::string &package, const std::string &key,
           const std::string &val) {
    set(package, key, val);
  }

  bool set(const std::string &package, const std::string &key,
           const std::string &val);

  bool set(const dnmi::db::DRow&);
  bool set(const kvKeyVal &keyVal);

  kvKeyVal& operator=(const kvKeyVal &keyVal) {
    if (&keyVal != this)
      set(keyVal);
    return *this;
  }

  void clean();

  const char* tableName() const {
    return "key_val";
  }
  std::string toSend() const;
  std::string toUpdate() const;
  std::string uniqueKey() const;

  const std::string & package() const {
    return package_;
  }
  const std::string & key() const {
    return key_;
  }
  const std::string & val() const {
    return val_;
  }
  void package(const std::string &v) {
    package_ = v;
  }
  void key(const std::string &v) {
    key_ = v;
  }
  void val(const std::string &v) {
    val_ = v;
  }
};

/** @} */

}
;

#endif
