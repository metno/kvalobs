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
#include <sstream>
#include "miconfparser/confsection.h"
#include "lib/milog/milog.h"
#include "lib/kvsubscribe/HttpSendData.h"
#include "lib/kvsubscribe/SendDataJsonResult.h"
#include "lib/miutil/httpclient.h"
using std::string;
using std::ostringstream;
using std::cerr;
using std::logic_error;
using miutil::HttpException;

namespace kvalobs {
namespace datasource {

namespace {
bool secure(const miutil::conf::ConfSection &conf) {
  return conf.getValue("kvDataInputd.http.useHTTPS").valAsBool(false);
}

std::string getHost(const miutil::conf::ConfSection &conf) {
  std::ostringstream ost;
  ost << conf.getValue("kvDataInputd.http.host").valAsString("localhost");
  ost << ':';
  ost << conf.getValue("kvDataInputd.http.port").valAsInt(8090);
  return ost.str();
}

class MyHttp : public miutil::HTTPClient {
  kvalobs::datasource::HttpSendData *parent_;
 public:
  explicit MyHttp(kvalobs::datasource::HttpSendData *parent)
      : parent_(parent) {
  }
  void log(const std::string &msg) override {
    parent_->log(msg);
  }
};

std::string createUrl(const std::string &hostAndPort, bool secure = false) {
  ostringstream ost;
  if (secure)
    ost << "https://";
  else
    ost << "http://";
  ost << hostAndPort << "/v1/observation";
  return ost.str();
}

}  // namespace

HttpSendData::HttpSendData(const std::string &hostAndPort, bool secure)
  : host_(createUrl(hostAndPort, secure)) {
}

HttpSendData::HttpSendData(const miutil::conf::ConfSection &conf)
    : HttpSendData(getHost(conf), secure(conf)) {
}

HttpSendData::~HttpSendData() {
}


Result HttpSendData::newData(const std::string &data, const std::string &obsType) {
  MyHttp http(this);
  ostringstream ost;
  ost << obsType << "\n" << data;

  try {
    http.post(host_, ost.str(), "text/plain");

    int retCode = http.returnCode();

    if (retCode != 200) {
      try {
        return decodeResultFromJsonString(http.content());
      } catch (const std::exception &ex) {
        ostringstream err;
        err << "Http return code: " << retCode << ".";
        throw Fatal(err.str());
      }
    } else {
      string sRes = http.content();
      LOGDEBUG("HttpSendData::newData:Response (" << host_ << "): " << sRes);
      return decodeResultFromJsonString(sRes);
    }
  } catch (const HttpException &ex) {
    throw Fatal(ex.what() + string(" Url: ") + host_);
  } catch (const logic_error &err) {
    throw Fatal(err.what() + string(" Url: ") + host_);
  } catch (const std::exception &err) {
    throw Fatal(err.what() + string(" Url: ") + host_);
  } catch (...) {
    throw Fatal("HttpSendData::newData (" + host_ + "): Unexpected unknown error. This is a bug.");
  }
}

Result HttpSendData::newData(const std::string &hostAndPort, const std::string &data, const std::string &obsType) {
  HttpSendData http(hostAndPort);
  return http.newData(data, obsType);
}

void HttpSendData::log(const std::string &msg) {
  LOGERROR("HttpSendData: " << msg);
}

}  // namespace datasource
}  // namespace kvalobs
