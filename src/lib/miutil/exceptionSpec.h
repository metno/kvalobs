/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: DataSrcImpl.h,v 1.4.2.2 2007/09/27 09:02:16 paule Exp $

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

#ifndef SRC_LIB_MIUTIL_EXCEPTIONSPEC_H_
#define SRC_LIB_MIUTIL_EXCEPTIONSPEC_H_

#include <exception>
#include <string>

#define EXCEPTION_SPEC_BASE(BaseName)\
class BaseName : public std::exception \
{\
  std::string msg_;\
\
 public:\
  explicit BaseName(const std::string & msg) : msg_(msg) {}\
  virtual ~BaseName() throw() {}\
  virtual const char* what() const throw() { return msg_.c_str(); }\
};

#define EXCEPTION_SPEC(BaseException, ExceptionName, message) \
class ExceptionName : public BaseException { \
 public: \
  explicit ExceptionName(const std::string &msg = message) : BaseException(msg) {} \
}

#define EXCEPTION_SPEC_CUSTOM_MESSAGE(BaseException, ExceptionName) \
class ExceptionName : public BaseException { \
 public: \
  explicit ExceptionName(const std::string & msg) : BaseException(msg) {} \
}


#endif  // SRC_LIB_MIUTIL_EXCEPTIONSPEC_H_
