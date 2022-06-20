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
using std::shared_ptr;
using std::make_shared;
using std::endl;
using std::ostringstream;
using httpserver::http_response;
using httpserver::string_response;
//using httpserver::http_response_builder;
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

namespace {
  shared_ptr<http_response> response(const std::string &content, 
    int response_code = http_utils::http_ok,
    const std::string& content_type = http_utils::text_plain) {
      return shared_ptr<http_response>(new string_response( content, response_code, content_type));
  }
}

const shared_ptr<http_response> ObservationHandler::render_POST(const httpserver::http_request& req) {
  Json::Value jval;
  bool ok=false;
  std::ostringstream oerr;
  std::string producer;

  if (app.inShutdown()) {
    return response("The service is unavailable.", HttpServiceUnavailable);
  }
  unsigned long long serial = getSerialNumber();
  ostringstream ost;

  ost << "http(" << serial << ")";
  milog::LogContext logContext(ost.str());

  LOGINFO("Path: '" << req.get_path() << " serialNumber: " << serial << ".");
  IDLOGINFO("http","Path: '" << req.get_path() << " serialNumber: " << serial << ".");

  Observation obs;
  shared_ptr<http_response> res;

  try {
    obs = getObservation(req);

    LOGDEBUG("obsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n");
    IDLOGDEBUG("http","obsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n");

    kd::Result r = app.newObservation(obs.obsType.c_str(), obs.obs.c_str(), serial, &producer, "http");
    jval = kd::decodeResultToJson(r);
    if (r.res == kd::EResult::OK) {
      ok=true;
      res=response(jval.toStyledString(), HttpOk, "application/json");
    } else if (r.res == kd::EResult::ERROR && b::starts_with(r.message, "SHUTDOWN")) {
      res=response("The service is unavailable.", HttpServiceUnavailable);
      oerr << "The service is unavailable.\n";
    } else {
      res=response(jval.toStyledString(), HttpBadRequest, "application/json");
      oerr << jval.toStyledString() << "\n";
    }
  } catch (const DecodeResultException &ex) {
    oerr << "DecodeResultException: " << ex.what() << "\n"; 
    LOGERROR("DecodeResultException: " << ex.what() << "\nobsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n")
    IDLOGERROR("http","DecodeResultException: " << ex.what() << "\nobsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n")
    jval = decodeResultToJson(ex);
    res=response(jval.toStyledString(), HttpBadRequest, "application/json");
  } catch ( const std::exception &ex) {
    oerr << "Unexpected exception: " << ex.what() << "\n";
    LOGERROR("Unexpected exception: " << ex.what() << "\nobsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n")
    IDLOGERROR("http","Unexpected exception: " << ex.what() << "\nobsType: '" << obs.obsType << "'\n" << "obsData:[\n" << obs.obs << "\n]\n")
    string err("Problems: " );
    err += ex.what();
    res=response(err, HttpInternalServerError, "application/text");
  }

  if( !ok && obs.obsType.find("kv2kv") != string::npos) {
    IDLOGDEBUG("kv2kvdecoder", "Serialnumber: " << serial << "\n" << oerr.str() << obs.obsType << "\n--- raw begin ----\n" << obs.obs <<"\n--- raw end ---");
  } 

  return res;
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
