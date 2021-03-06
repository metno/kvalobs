/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvAlgorithms.h,v 1.1.2.2 2007/09/27 09:02:29 paule Exp $

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
#ifndef __kvAlgorithms_h__
#define __kvAlgorithms_h__

#include <kvalobs/kvDbBase.h>

/* Created by DNMI/IT: borge.moe@met.no
 at Mon Aug 227 15:29:59 2002 */
namespace kvalobs {

/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table algorithms in the kvalobs database.
 */

class kvAlgorithms : public kvDbBase {
 private:
  int language_;
  std::string checkname_;
  std::string signature_;
  std::string script_;

 public:
  kvAlgorithms() {
  }
  kvAlgorithms(const dnmi::db::DRow &r) {
    set(r);
  }
  kvAlgorithms(int language, const std::string &checkname,
               const std::string &signature, const std::string &script) {
    set(language, checkname, signature, script);
  }

  bool set(int language, const std::string &checkname,
           const std::string &signature, const std::string &script);

  bool set(const dnmi::db::DRow&);
  const char* tableName() const {
    return "algorithms";
  }
  std::string toSend() const;

  std::string uniqueKey() const;

  int language() const {
    return language_;
  }
  const std::string & checkname() const {
    return checkname_;
  }
  const std::string & signature() const {
    return signature_;
  }
  const std::string & script() const {
    return script_;
  }

};

/** @} */
}
#endif
