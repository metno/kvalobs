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
#include "lib/kvsubscribe/HttpSendData.h"
#include "lib/kvsubscribe/SendDataJsonResult.h"

using std::string;
using std::ostringstream;
using std::cerr;
using std::logic_error;
using miutil::HttpException;

namespace kvalobs {
namespace datasource {

HttpSendData::HttpSendData(const std::string &hostAndPort, bool secure) {
  ostringstream ost;

  if (secure)
    ost << "https://";
  else
    ost << "http://";

  ost << hostAndPort << "/v1/observation";

  host = ost.str();
}

HttpSendData::~HttpSendData() {
}

Result HttpSendData::newData(const std::string &data, const std::string &obsType) {
  ostringstream ost;
  ost << obsType << "\n" << data;

  try {
    http.post(host, ost.str(), "text/plain");

    int retCode = http.returnCode();

    if (retCode != 200) {
      ostringstream err;
      err << "Http return code: " << retCode << ".";
      throw Fatal(err.str());
    }

    string sRes = http.content();
    cerr << "HttpSendData::newData:Response: " << sRes << "\n\n";
    return decodeResultFromJsonString(sRes);
  } catch (const HttpException &ex) {
    throw Fatal(ex.what());
  } catch (const logic_error &err) {
    throw Fatal(err.what());
  } catch (...) {
    throw Fatal("HttpSendData::newData: Unexpected unknown error. This is a bug.");
  }
}

}  // namespace datasource
}  // namespace kvalobs
