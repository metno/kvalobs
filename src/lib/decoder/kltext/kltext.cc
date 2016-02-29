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
#include "lib/decoder/kltext/kltext.h"
#include "lib/decoder/kltext/test/testheader.h"

using kvalobs::kvTextData;
using std::string;
using miutil::CommaString;
using miutil::trimstr;
namespace pt = boost::posix_time;

namespace kvalobs {
namespace decoder {
namespace kltextdecoder {

namespace {

pt::ptime tbTimeForTest;

bool decodeKeyVal(const string &keyval, string &key, string &val) {
  string::size_type i = keyval.find_first_of("=");

  if (i == string::npos) {
    key = keyval;
    val.erase();
  } else {
    key = keyval.substr(0, i);
    val = keyval.substr(i + 1);
  }

  trimstr(key);
  trimstr(val);

  return !key.empty();
}

pt::ptime getTbTime() {
  if (!tbTimeForTest.is_special())
    return tbTimeForTest;
  else
    return pt::second_clock::universal_time();
}

}  // namespace

namespace test {
void setTbTime(const boost::posix_time::ptime &tb) {
  tbTimeForTest = tb;
}
}

void KlText::decodeObsType(const std::string &obstype) {
  string keyval;
  string key;
  string val;
  string::size_type i;
  string::size_type iKey;
  CommaString cstr(obstype, '/');

  LOGDEBUG("decodeObsType: '" << obstype << "'");
  typeID = INT_MAX;
  stationID = INT_MAX;
  receivedTime = pt::ptime();
  redirectedFrom.erase();
  stationidIn.erase();

  if (cstr.size() < 2) {
    LOGERROR("decodeObsType: Too few keys!");
//        msg="obsType: Invalid Format!";
    return;
  }

  for (int index = 1; index < cstr.size(); ++index) {
    if (!cstr.get(index, keyval)) {
      LOGERROR("decodeObsType: INTERNALERROR: InvalidFormat!");
      return;
    }

    if (!decodeKeyVal(keyval, key, val))  // keyval empty
      continue;

    // Valid keys nationalnr, stationid, wmonr, type, received_time and redirected.
    if (key == "received_time") {
      receivedTime = pt::time_from_string_nothrow(val);
    } else if (key == "redirected") {
      redirectedFrom = val;
    } else if (key == "type") {
      typeID = atoi(val.c_str());
    } else if (key == "nationalnr" || key == "stationid" || key == "wmonr") {
      stationidIn = key + "=" + val;
      stationID = getStationId(key, val);

      if (stationID < 0) {
        stationID = INT_MAX;
        LOGERROR("No station with stationid '" << key << "=" << val << "'");
        continue;
      }
    } else {
      LOGERROR("decodeObsType: INTERNAL: unhandled key '" << key << "', this is a error and must be fixed.");
    }
  }

  LOGDEBUG("decodeObsType: stationID: " << stationID << " typeid: " << typeID);
}

KlText::KlText(std::function<int(const std::string &key, const std::string &val)> getStationId_)
    : typeID(INT_MAX),
      stationID(INT_MAX),
      getStationId(getStationId_) {
}

KlText::~KlText() {
}

kvalobs::decoder::DecoderBase::DecodeResult KlText::execute(const std::string &obsType_, const std::string &obs_, std::string &msg,
                                                            kvalobs::kvTextData *textData) {
  obsType = obsType_;
  obs = obs_;
  decodeObsType(obsType);

  LOGINFO("New observation!  " << miutil::miTime::nowTime());

  if (stationID == INT_MAX || typeID == INT_MAX) {
    msg = "Missing nationalnr, stationid, wmonr or type.";
    return kvalobs::decoder::DecoderBase::Rejected;
  }

  trimstr(obs);
  if (obs.length() > 256) {
    LOGWARN("The message is greater than 256, it is truncated to the length 256.");
    msg += "The message is truncated to 256 bytes.\n";
    obs = obs.substr(0, 256);
  }

  if (receivedTime.is_special())
    receivedTime = pt::second_clock::universal_time();

  *textData = kvTextData(stationID, receivedTime, obs, 1001, getTbTime(), typeID);
  return kvalobs::decoder::DecoderBase::Ok;
}

}  // namespace kltextdecoder
}  // namespace decoder
}  // namespace kvalobs
