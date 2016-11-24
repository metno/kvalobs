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

#ifndef SRC_KVDATAINPUTD_OBSERVATIONHANDLER_H_
#define SRC_KVDATAINPUTD_OBSERVATIONHANDLER_H_

#include <atomic>
#include <string>
#include "httpserver.hpp"
#include "lib/kvsubscribe/SendData.h"
#include "lib/kvsubscribe/ProducerCommand.h"
#include "kvDataInputd/DataSrcApp.h"

class ObservationHandler : public httpserver::http_resource<ObservationHandler> {
 public:
  struct Observation {
    std::string obsType;
    std::string obs;
    Observation(const std::string &obsType_, const std::string &obs_)
        : obsType(obsType_),
          obs(obs_) {
    }
  };

  class DecodeResultException : public std::exception, public kvalobs::datasource::Result {
   public:
    DecodeResultException(kvalobs::datasource::EResult res, const std::string &errormsg) {
      this->res = res;
      message = errormsg;
    }

    const char* what() const noexcept {
      return message.c_str();
    }
  };

  ObservationHandler(DataSrcApp &app, kvalobs::service::ProducerQuePtr raw);
  void render_POST(const httpserver::http_request& req, httpserver::http_response** res);

 protected:
  unsigned long long getSerialNumber();
  Observation getObservation(const httpserver::http_request& req);
  void postOnRawQue(const std::string &rawData);

 private:
  DataSrcApp &app;
  std::atomic_ullong serialNumber;
  kvalobs::service::ProducerQuePtr rawQue;
};

#endif  // SRC_KVDATAINPUTD_OBSERVATIONHANDLER_H_
