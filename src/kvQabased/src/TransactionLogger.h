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

#ifndef SRC_KVQABASED_SRC_TRANSACTIONLOGGER_H_
#define SRC_KVQABASED_SRC_TRANSACTIONLOGGER_H_

#include <kvalobs/kvStationInfo.h>
#include <string>
#include <chrono>


namespace qabase {

/**
 * Simple transaction logger. Log rotation is expected to be handled
 * elsewhere (in debian package).
 *
 * The actual writing to file is performed when this object is destroyed.
 */
class TransactionLogger {
 public:
  explicit TransactionLogger(const kvalobs::kvStationInfo & si);
  ~TransactionLogger();

  /**
   * Get the message that will be written to log
   */
  std::string message() const;

  /**
   * Make log statement mark this transaction as a success. If this method has
   * not been called, the transaction will be marked as failed.
   */
  void markSuccess() {
    success_ = true;
  }

  /**
   * Get the name of the file to log to.
   */
  static std::string logFile();

 private:
  bool success_;
  kvalobs::kvStationInfo si_;
  std::chrono::system_clock::time_point startTime_;
};


}  // namespace qabase

#endif  // SRC_KVQABASED_SRC_TRANSACTIONLOGGER_H_
