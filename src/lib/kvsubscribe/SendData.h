/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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

#ifndef SRC_LIB_KVSUBSCRIBE_SENDDATA_H_
#define SRC_LIB_KVSUBSCRIBE_SENDDATA_H_

#include <string>
#include "boost/lexical_cast.hpp"
#include "miutil/exceptionSpec.h"

namespace kvalobs {
namespace datasource {

EXCEPTION_SPEC_BASE(Fatal);

/**
 * NODECODER,   there is no decoder for the obsType.
 *              The observation is not saved to the database. Don't
 *              mind to retry to send the observation until a
 *              decoder is written and installed.
 * DECODEERROR, cant decode the message. The
 *              message is saved to rejectdecode.
 * NOTSAVED,    the message is not SAVED to the database,
 *              if possible try to resend it later, after
 *              a delay.
 * ERROR,       A general error. Look at the 'message'. The
 *              observation is not saved to the database.
 * OK           The message is decoded and saved to the
 *              database.
 */

enum EResult {
  OK = 0,
  NODECODER = 1,
  DECODEERROR = 2,
  NOTSAVED = 3,
  ERROR = 4
};

struct Result {
  EResult res;
  std::string message;
  std::string messageId;
  bool retry;
  
  Result()
      : res(EResult::OK),
        retry(false) {
  }
  explicit Result(int messageid)
      : res(EResult::OK),
        retry(false),
        messageId(boost::lexical_cast<std::string>(messageid)) {
  }
};

class SendData {
 public:
  virtual ~SendData() {
  }
  /**
   * @throws Fatal on transport error.
   */
  virtual Result newData(const std::string &data, const std::string &obsType) = 0;
};

}  // namespace datasource
}  // namespace kvalobs

#endif  // SRC_LIB_KVSUBSCRIBE_SENDDATA_H_
