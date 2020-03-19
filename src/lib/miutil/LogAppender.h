/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvPath.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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

#ifndef SRC_MIUTIL_LOGAPPENDER_H_
#define SRC_MIUTIL_LOGAPPENDER_H_

#include <string>

namespace miutil {

/**
 * Simple logger that apends to a file atomical. Log rotation is expected to be handled
 * elsewhere (in debian package).
 *
 */
class LogAppender {
 public:
  explicit LogAppender(const std::string &logFile, const std::string &dir=".");
  ~LogAppender();

  bool isOk()const {return ok_;}
  std::string lastError()const { return lastError_;}
  /**
   * Log the message to the logfile.
   * @return true if logged and false otherwise. Check lastError.
   */
  bool log(const std::string &message);

  /**
   * Get the name of the file to log to.
   */
  std::string logFile() { return logfile_;}

 private:
  bool ok_;
  std::string lastError_;
  std::string logfile_;
};

}  

#endif  
