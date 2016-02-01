/*
 wdb - weather and water data storage

 Copyright (C) 2007 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 E-mail: wdb@met.no

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA  02110-1301, USA
 */

#ifndef SRC_LIB_MIUTIL_HTTPCLIENT_H_
#define SRC_LIB_MIUTIL_HTTPCLIENT_H_

#include <ostream>
#include <sstream>
#include <string>
#include "curl/curl.h"
#include "miutil/exceptionSpec.h"

namespace miutil {
EXCEPTION_SPEC_BASE(HttpException);
EXCEPTION_SPEC(HttpException, HttpInitError, "The HTTP client is not initialized!");
EXCEPTION_SPEC_CUSTOM_MESSAGE(HttpException, HttpConnectError);
EXCEPTION_SPEC_CUSTOM_MESSAGE(HttpException, HttpSSLError);
EXCEPTION_SPEC_CUSTOM_MESSAGE(HttpException, HttpAccessDenied);
EXCEPTION_SPEC_CUSTOM_MESSAGE(HttpException, HttpTimeout);
EXCEPTION_SPEC_CUSTOM_MESSAGE(HttpException, HttpIOError);
EXCEPTION_SPEC_CUSTOM_MESSAGE(HttpException, HttpUrlError);

/**
 * HTTPClient is not thread safe.
 * Create a HTTPClient object in each
 * thread you need to call an http service.
 */

class HTTPClient {
  HTTPClient(const HTTPClient &) = delete;
  HTTPClient& operator=(const HTTPClient &) = delete;

  CURL *curl;
  struct curl_slist *header;
  std::ostream *out;
  std::ostringstream content_;
  char *errbuf;

 protected:
  void open();

 public:
  HTTPClient();
  virtual ~HTTPClient();

  /**
   * Implementation detail. Dont use.
   */
  std::ostream *outStream() {
    return out;
  }

  void close();

  void get(const std::string &url, std::ostream &content);
  void post(const std::string &url, const std::string &content, const std::string &contentType);

  void postFile(const std::string &url, const std::string &file, const std::string &contentType);

  std::string content() const {
    return content_.str();
  }
  long contenlLength() const;
  std::string contentType() const;
  long returnCode() const;
  double totalTime() const;
  virtual void log(const std::string &msg);
};

}  // namespace miutil

#endif  // SRC_LIB_MIUTIL_HTTPCLIENT_H_
