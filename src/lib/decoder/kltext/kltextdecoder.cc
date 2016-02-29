/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: dummydecoder.cc,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $

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
#include <limits.h>
#include "puTools/miTime.h"
#include "lib/milog/milog.h"
#include "lib/miutil/commastring.h"
#include "lib/miutil/trimstr.h"
#include "lib/miutil/timeconvert.h"
#include "lib/kvalobs/kvTextData.h"
#include "lib/decoder/kltext/kltextdecoder.h"
#include "lib/decoder/kltext/kltext.h"

using kvalobs::kvTextData;
using std::string;
using miutil::CommaString;
using miutil::trimstr;
namespace pt = boost::posix_time;

namespace kvalobs {
namespace decoder {
namespace kltextdecoder {

KlTextDecoder::KlTextDecoder(dnmi::db::Connection &con, const ParamList &params, const std::list<kvalobs::kvTypes> &typeList, const std::string &obsType,
                             const std::string &obs, int decoderId)
    : DecoderBase(con, params, typeList, obsType, obs, decoderId),
      typeID(INT_MAX),
      stationID(INT_MAX) {
}

KlTextDecoder::~KlTextDecoder() {
}

std::string KlTextDecoder::name() const {
  return "KlTextDecoder";
}

kvalobs::decoder::DecoderBase::DecodeResult KlTextDecoder::execute(std::string &msg) {
  milog::LogContext lcontext(name());
  kvTextData textData;
  DecodeResult res;

  LOGINFO("New observation!  " << miutil::miTime::nowTime());

  KlText decode([this](const string &key, const string &val) {return getStationId(key, val);});

  res = decode.execute(obsType, obs, msg, &textData);

  if (res != Ok)
    return res;

  if (!putkvTextDataInDb(textData, 10)) {
    msg += "Failed to add the textdata to the database.";
    LOGERROR("Failed to put the data to the database: \nPartial INSERT SQL:\n INSERT ... " << textData.toSend());
    return NotSaved;
  }

  return Ok;
}

}  // namespace kltextdecoder
}  // namespace decoder
}  // namespace kvalobs
