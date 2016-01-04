/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: comobsentry.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $

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

#ifndef __BUFRDECODEBASE_H__
#define __BUFRDECODEBASE_H__

#include <float.h>
#include <limits.h>
#include <list>
#include <exception>
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include <kvalobs/kvData.h>
#include "BufrMessage.h"

namespace kvalobs {
namespace decoder {
namespace bufr {

class BufrDecodeResultBase {
 public:
  BufrDecodeResultBase() {
  }
  virtual ~BufrDecodeResultBase() {
  }

  virtual void setObstime(const boost::posix_time::ptime &obstime) = 0;
  virtual boost::posix_time::ptime getObstime() const = 0;
  virtual void setStationid(int wmono) = 0;
  virtual void setLatLong(double latitude, double longitude) = 0;
  virtual void add(float value, int kvparamid,
                   const miutil::miTime &obstime) = 0;
};

class BufrException : public std::exception {
 protected:
  std::string what_;

 public:
  BufrException() {
  }
  BufrException(const std::string &what)
      : what_(what) {
  }
  ~BufrException() throw () {
  }

  const char *what() const throw () {
    return what_.c_str();
  }

};

class BufrEndException : public BufrException {
 protected:
  std::string what_;

 public:
  BufrEndException(const std::string &what = "")
      : what_(what) {
  }
  ~BufrEndException() throw () {
  }

  const char *what() const throw () {
    if (what_.empty())
      const_cast<BufrEndException*>(this)->what_ = "Unexpected end of BUFR.";
    return what_.c_str();
  }
};

class BufrSequenceException : public BufrException {
 public:
  int expectedDescriptor;
  int actualDescriptor;

  BufrSequenceException(int expected, int actual)
      : expectedDescriptor(expected),
        actualDescriptor(actual) {
  }

  const char *what() const throw () {
    std::ostringstream out;
    out << "BUFR: SequenceException expected descriptor: " << expectedDescriptor
        << " actual descriptor: " << actualDescriptor;
    const_cast<BufrSequenceException*>(this)->what_ = out.str();
    return what_.c_str();
  }
};

class BufrDecodeBase;

class BufrValue {
 public:
  typedef enum {
    String,
    Float,
    Undefined
  } ValueType;

  BufrValue()
      : dValue( DBL_MAX),
        type(Undefined) {
  }

  ValueType getType() const {
    return type;
  }
  std::string getStrValue() const {
    return sValue;
  }
  double getValue() const {
    return dValue;
  }
  std::string getUnit() const {
    return unit;
  }
 private:
  friend class BufrDecodeBase;
  double dValue;
  std::string sValue;
  std::string unit;
  ValueType type;
};

class BufrDecodeBase {
  bool doDecode(int descriptor, BufrValue &value, bool mustexist);

 protected:

  BufrMessage *bufrMessage;

  /**
   * @exception BufrSequenceException, BufrException, BufrEndException
   * @param descriptor Expected descriptor
   * @param value Set the value from the bufr.
   * @return If mustexist=true an excpetion is thrown if the descriptor
   *         do not exist. If mustexist=false false is returned.
   */
  bool getDescriptor(int descriptor, double &value, std::string &unit,
                     bool mustexist = true);
  bool getDescriptor(int descriptor, std::string &value, bool mustexist = true);

  /**
   * Get the next descriptor and move the get pointer to the next descriptor.
   *
   * @param descriptor The descriptor.
   * @param value The value,
   * @param unit The unit.
   * @return
   */
  bool nextDescriptor(int &descriptor, BufrValue &value);

  /**
   * Get the next descriptor and without moving the get pointer to the next decriptor.
   *
   * @param descriptor The descriptor.
   * @param value The value,
   * @param unit The unit.
   * @param index The descriptor to look at counted from the current position.
   * @return
   */
  bool peekAt(int &descriptor, BufrValue &value, int index = 1);

  /**
   * @exception BufrSequenceException, BufrException, BufrEndException
   * @param descriptor Expected descriptor.
   * @param kvparam kvalobs parameter id.
   * @param sensor kvalobs  sensor
   * @param level kvalobs level.
   * * @return If mustexist=true an excpetion is thrown if the descriptor
   *         do not exist. If mustexist=false false is returned.
   */
  //bool getDescriptor( int descriptor, int kvparam, BufrDecodeResultBase *res, bool mustexist=true );
  bool ignoreDescriptor(int descriptor, bool mustexist = true);

  /**
   * @exception BufrSequenceException, BufrException
   */
  virtual void decode(BufrDecodeResultBase *result)=0;

 public:
  miutil::miTime obstime;

  BufrDecodeBase() {
  }
  virtual ~BufrDecodeBase() {
  }
  /**
   * @exception BufrSequenceException, BufrException,BufrEndException
   * @param bufr
   * @return
   */
  virtual void decodeBufrMessage(BufrMessage *bufr,
                                 BufrDecodeResultBase *result);

};

}
}
}

#endif /* BUFRDECODEBASE_H_ */
