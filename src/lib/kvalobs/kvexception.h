/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvexception.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __KvException_h__
#define __KvException_h__

#include <exception>

namespace kvalobs {

/**
 * \addtogroup kvinternalhelpers
 * @{
 */

/**
 * \brief An excetion class used to throw an exception that
 *        in some way is a bad format.
 */
class FormatException : public std::exception {
  std::string reason;
 public:
  FormatException(const std::string &reason_)
      : reason(reason_) {
  }
  ~FormatException() throw () {
  }

  const char *what() const throw () {
    return reason.c_str();
  }
};

/**
 * \brief An excetion class used to throw an exception that
 *       in some way is a unknown parameter.
 */
class UnknownParam : public std::exception {
  std::string reason;
 public:
  UnknownParam(const std::string &reason_)
      : reason(reason_) {
  }
  ~UnknownParam() throw () {
  }

  const char *what() const throw () {
    return reason.c_str();
  }
};

/**
 * @brief A method received some kind of invalid input.
 */
class InvalidInput : public std::exception {
  virtual const char* what() const throw () {
    return "Invalid input";
  }
};

/** @} */
}

#endif
