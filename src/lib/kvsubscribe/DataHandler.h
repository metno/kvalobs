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

#ifndef SRC_LIB_KVSUBSCRIBE_DATAHANDLER_H_
#define SRC_LIB_KVSUBSCRIBE_DATAHANDLER_H_

#include <string>

namespace kvalobs {
namespace serialize {
class KvalobsData;
}

namespace subscribe {

/**
 * Abstract base class for handling parsed Kvalobs data and errors.
 *
 * This class defines the interface that must be implemented by concrete
 * data handlers to process KvalobsData objects and handle errors.
 * It separates the data handling logic from the transport/consumer logic.
 */
class DataHandler {
public:
  virtual ~DataHandler() {}

  /**
   * Handle parsed Kvalobs data.
   *
   * @param data The parsed KvalobsData object to process
   */
  virtual void handleData(const ::kvalobs::serialize::KvalobsData &data) = 0;

  /**
   * Handle errors that occur during data processing.
   *
   * @param code Error code
   * @param msg Error message
   */
  virtual void handleError(int code, const std::string &msg) = 0;
};

} /* namespace subscribe */
} /* namespace kvalobs */

#endif /* SRC_LIB_KVSUBSCRIBE_DATAHANDLER_H_ */
