/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

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

#include <memory>
#include <iostream>
#include "boost/algorithm/string.hpp"
#include "lib/kvsubscribe/HttpSendData.h"
#include "lib/kvsubscribe/SendDataJsonResult.h"
#include "kvDataInputd/ObservationHandler.h"
#include "kvDataInputd/RawDataCommand.h"

namespace kd = kvalobs::datasource;
namespace b = boost;

using std::string;
using std::endl;
using std::ostringstream;
using httpserver::http_response;
using httpserver::http_response_builder;
using httpserver::http::http_utils;

namespace {
int HttpServiceUnavailable = http_utils::http_service_unavailable;
int HttpOk = http_utils::http_ok;
int HttpBadRequest = http_utils::http_bad_request;
int HttpInternalServerError=http_utils::http_internal_server_error;
}  // namespace

ObservationHandler::ObservationHandler(DataSrcApp &app, kvalobs::service::ProducerQuePtr raw)
    : app(app),
      serialNumber(0),
      rawQue(raw) {
}

void ObservationHandler::render_POST(const httpserver::http_request& req, httpserver::http_response** res) {
  Json::Value jval;
  bool ok=false;
  std::ostringstream oerr;

  if (app.inShutdown()) {
    *res = new http_response(http_response_builder("The service is unavailable.", HttpServiceUnavailable).string_response());
    return;
  }
  unsigned long long serial = getSerialNumber();
  ostringstream ost;

  ost << "http(" << serial << ")";
  milog::LogContext logContext(ost.str());

  LOGINFO("Path: '" << req.get_path() << " serialNumber: " << serial << ".");

  Observation obs;

  try {
    obs = getObservation(req);

    LOGDEBUG("obsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n");

    kd::Result r;

    r = app.newObservation(obs.obsType.c_str(), obs.obs.c_str(), "http");
    jval = kd::decodeResultToJson(r);
    if (r.res == kd::EResult::OK) {
      ok=true;
      *res = new http_response(http_response_builder(jval.toStyledString(), HttpOk, "application/json").string_response());
    } else if (r.res == kd::EResult::ERROR && b::starts_with(r.message, "SHUTDOWN")) {
      *res = new http_response(http_response_builder("The service is unavailable.", HttpServiceUnavailable).string_response());
      oerr << "The service is unavailable.\n";
    } else {
      *res = new http_response(http_response_builder(jval.toStyledString(), HttpBadRequest, "application/json").string_response());
      oerr << jval.toStyledString() << "\n";
    }
  } catch (const DecodeResultException &ex) {
    oerr << "DecodeResultException: " << ex.what() << "\n"; 
    LOGERROR("DecodeResultException: " << ex.what() << "\nobsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n")
    jval = decodeResultToJson(ex);
    *res = new http_response(http_response_builder(jval.toStyledString(), HttpBadRequest, "application/json").string_response());
  } catch ( const std::exception &ex) {
    oerr << "Unexpected exception: " << ex.what() << "\n";
    LOGERROR("Unexpected exception: " << ex.what() << "\nobsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n")
    string err("Problems: " );
    err += ex.what();
    *res = new http_response(http_response_builder(err, HttpInternalServerError, "application/text" ).string_response());
  }

  if( !ok && obs.obsType.find("kv2kv") != string::npos) {
    IDLOGDEBUG("kv2kvdecoder", "Serialnumber: " << serialNumber << "\n" << oerr.str() << obs.obsType<<"\n"<<req.get_content()<<"\n" << obs.obs );
  } 
}

ObservationHandler::Observation ObservationHandler::getObservation(const httpserver::http_request& req) {
  string::size_type i, iStart;
  string content = req.get_content();

  string ct = req.get_header("Content-type");

  postOnRawQue(content);
  LOGDEBUG("Content-type: " << ct << "\n" << "Content: \n" << content << endl);
  iStart = content.find_first_not_of("\n\r\t ");

  if (iStart == string::npos)
    throw DecodeResultException(kd::EResult::DECODEERROR, "Empty message.");

  i = content.find("\n", iStart);

  if (i == string::npos)
    throw DecodeResultException(kd::EResult::DECODEERROR, "Only obstype is given.");

  return Observation(content.substr(iStart, i - iStart), content.substr(i + 1));
}

void ObservationHandler::postOnRawQue(const std::string &rawData) {
  if (rawData.empty())
    return;

  std::unique_ptr<RawDataCommand> data(new RawDataCommand(rawData));

  try {
    rawQue->timedAdd(data.get(), std::chrono::seconds(4), true);
    data.release();
  } catch (std::exception &ex) {
    LOGWARN("Unable to post data to the raw kafka queue.\nReason: " << ex.what() <<"\n"<< rawData);
  }
}

unsigned long long ObservationHandler::getSerialNumber() {
  return serialNumber.fetch_add(1);
}
