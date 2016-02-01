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

#ifndef SRC_LIB_KVSUBSCRIBE_HTTPSENDDATA_H_
#define SRC_LIB_KVSUBSCRIBE_HTTPSENDDATA_H_

#include <string>
#include "miutil/httpclient.h"
#include "kvsubscribe/SendData.h"

namespace kvalobs {
namespace datasource {

class HttpSendData : public SendData, miutil::HTTPClient {
  std::string host_;

 public:
  /**
   * @param hostAndPort must be on the form host:port.
   * @param useHttps Use https if true.
   */
  explicit HttpSendData(const std::string &hostAndPort, bool useHttps = false);
  virtual ~HttpSendData();

  /**
   * @throw Fatal on transport problems.
   */
  Result newData(const std::string &data, const std::string &obsType) override;

  const std::string host() const {
    return host_;
  }

  void log(const std::string &msg) override;
};

}  // namespace datasource
}  // namespace kvalobs
#endif  // SRC_LIB_KVSUBSCRIBE_HTTPSENDDATA_H_
