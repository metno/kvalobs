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

#include "kvsubscribe/SendDataJsonResult.h"

namespace kvalobs {
namespace datasource {

Json::Value decodeResultToJson(const Result &r) {
  Json::Value obj(Json::objectValue);
  Json::Value nullVal;
  Json::Value messageIdVal(r.messageId);
  obj["messageid"] = (r.messageId.empty() ? nullVal : messageIdVal);
  obj["res"] = r.res;
  obj["message"] = r.message;
  obj["retry"] = r.res == NOTSAVED;
  return obj;
}

Result decodeResultFromJson(const Json::Value &jv) {
  Result r;
  int res = jv.get("res", -1).asInt();

  if (res < EResult::OK || res > EResult::ERROR) {
    throw std::logic_error("Invalid return from the server.");
  }

  r.res = EResult(res);
  Json::Value messageId = jv.get("messageid", Json::Value());
  r.message = jv.get("message", "").asString();
  r.messageId = messageId.isNull() ? "" : messageId.asString();
  r.retry = jv.get("retry", false).asBool();
  return r;
}

Result decodeResultFromJsonString(const std::string &result) {
  Json::Value v;
  Json::Reader reader;

  if (!reader.parse(result, v, false)) {
    throw std::logic_error("Invalid return from the server. (" + reader.getFormattedErrorMessages() + ")");
  }

  return decodeResultFromJson(v);
}

}  // namespace datasource
}  // namespace kvalobs
