/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: synopdecoder.cc,v 1.18.2.5 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <float.h>
#include <limits.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <puTools/miTime.h>
#include <miutil/commastring.h>
#include <miutil/splitstr.h>
#include <miutil/base64.h>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvTypes.h>
#include <fileutil/mkdir.h>
#include "bufrdecoder.h"
#include "BufrDecodeKvResult.h"
#include "BufrDecodeSynoptic.h"

using namespace kvalobs::decoder::bufr;
using namespace std;
using namespace dnmi::db;
using namespace miutil;
using namespace boost;
using namespace kvalobs;

miutil::miTime BufrDecoder::lastStationCheck;
boost::mutex BufrDecoder::mutex;

BufrDecoder::BufrDecoder(dnmi::db::Connection &con, const ParamList &params,
                         const std::list<kvalobs::kvTypes> &typeList,
                         const std::string &obsType, const std::string &obs,
                         int decoderId)
    : DecoderBase(con, params, typeList, obsType, obs, decoderId),
      earlyobs(INT_MAX),
      lateobs( INT_MAX)

{
}

BufrDecoder::~BufrDecoder() {
}

std::string BufrDecoder::name() const {
  return "BufrDecoder";
}

long BufrDecoder::getStationId(std::string & msg) {
}

long BufrDecoder::getStationid(int wmono) const {
  if (!stationList)
    return -1;

  for (std::list<kvalobs::kvStation>::const_iterator it = stationList->begin();
      it != stationList->end(); ++it) {
    if (wmono == it->wmonr())
      return it->stationID();
    else if (wmono > it->wmonr())
      return -1;
  }

  return -1;
}

bool BufrDecoder::getEarlyLateObs(int &early, int &late) const {
  if (lateobs == INT_MAX || earlyobs == INT_MAX)
    return false;

  early = earlyobs;
  late = lateobs;
  return true;
}

bool BufrDecoder::saveData(list<kvalobs::kvData> &data, bool &rejected,
                           std::string &rejectedMessage) {
  ostringstream logid;
  list<kvalobs::kvTextData> textData;
  list<kvalobs::kvData>::iterator it = data.begin();
  int i = 0;
  int priority = 10;

  rejected = false;

  if (it == data.end())
    return true;

  if (it->stationID() < 100000) {
    //National stations that is registred in table 'station'.
    priority = 6;
  } else if (it->stationID() <= 10000000) {
    //Foregn stations is in the range [100000, 10000000]
    priority = 7;
  } else if (it->stationID() > 10000000 && it->stationID() < 130000000) {
    //COMMENT:
    //As a quick fix to set priority. We reduce the priority
    //for ships. We now that ships get a automatic generated stationid
    //in the range [10100009, 123123599]. For

    priority = 8;
  }

  for (; it != data.end(); it++) {
    if (it->obstime().is_special() || it->tbtime().is_special()) {
      rejectedMessage = "Missing obsTime or tbtime for observation!";
      rejected = true;
      return false;
    }
  }

  logid << "n" << it->stationID() << "-t" << it->typeID() << ".log";
  createLogger(logid.str());
  int ret = true;
  it = data.begin();

  if (!addDataToDb(it->obstime(), it->stationID(), it->typeID(), data, textData,
                   priority, logid.str())) {
    ret = false;
  }

  removeLogger(logid.str());

  return ret;
}

bool BufrDecoder::initialize() {
  kvDbGate gate(getConnection());
  list<kvStation> *stat = new list<kvStation>();
  list<kvTypes> types;

  //Get stations from the database

  if (!stat) {
    LOGERROR("NOMEM: Can't allocate space for <station> data!");
    return false;
  }

  if (!gate.select(*stat, " WHERE wmonr IS NOT NULL ORDER BY wmonr")) {
    LOGERROR(
        "Can't get station data from table <station>!\n" << gate.getErrorStr());
    delete stat;
    return false;
  } else {
    LOGINFO(
        "Data for " << stat->size() << " station is read from table <station>");
  }

  stationList.reset(stat);
  lastStationCheck = miTime::nowTime();

  if (gate.select(types, "where typeid=1 OR typeid=7 order by typeid DESC")) {
    if (!types.empty()) {
      earlyobs = types.begin()->earlyobs();
      lateobs = types.begin()->lateobs();
    }
  }

  return true;
}

void BufrDecoder::splitBufr(const std::string &bufr, list<string> &bufrs) {

  string::size_type prev = string::npos;
  string::size_type i = bufr.find("BUFR");

  bufrs.clear();

  while (i != string::npos) {
    if (prev == string::npos) {
      prev = i;
      i = bufr.find("BUFR", i + 1);
      continue;
    }

    bufrs.push_back(bufr.substr(prev, i - prev));

    prev = i;
    i = bufr.find("BUFR", i + 1);
  }

  if (prev != string::npos)
    bufrs.push_back(bufr.substr(prev));
}

std::string BufrDecoder::getFormat() const {
  string::size_type n;
  vector<string> strings = splitstr(obsType, '/');

  for (vector<string>::size_type i = 0; i < strings.size(); ++i) {
    if (strings[i].find("format") != string::npos) {
      vector<string> keyval = splitstr(strings[i], '=');

      if (keyval.size() == 2 && keyval[0] == "format")
        return keyval[1];
    }
  }

  return "";
}

kvalobs::decoder::DecoderBase::DecodeResult BufrDecoder::execute(
    std::string &msg) {
  kvalobs::kvRejectdecode reject;
  bool saveReject;
  std::string saveRejectMessage;
  list<kvalobs::kvData> data;
  miTime nowTime(miTime::nowTime());
  string format;
  string bufr;
  list<string> bufrList;
  int errorCount = 0;

  Lock lock(mutex);

  milog::LogContext lcontext("BufrDecoder");
  LOGINFO("New observation(s)");

  if (obs.length() == 0) {
    LOGERROR("Incomming message has zero size!");
    return Ok;
  }

  format = getFormat();

  if (format != "base64") {
    LOGERROR(
        "It is expected that the observation is base64 encoded. Format is <"
            + format + ">.");
    msg = "It is expected that the observation is base64 encoded. Format is <"
        + format + ">.";
    return Rejected;
  }

  if (lastStationCheck.undef()
      || abs(miTime::hourDiff(lastStationCheck, nowTime)) > 1) {
    LOGINFO("Initialize the station information from the database.");

    if (!initialize()) {
      LOGERROR("Can't initialize the SynopDecoder!!!");
      msg = "Can't initialize the SynopDecoder!!!";
      return NotSaved;
    }
  }

  decode64(obs, bufr);
  splitBufr(bufr, bufrList);

  BufrDecodeSynoptic bufrDecoder;
  DecodeKvResult decodeResult;
  BufrMessage bufrMsg;

  for (list<string>::const_iterator it = bufrList.begin(); it != bufrList.end();
      ++it) {
    if (!BufrMessage::bufrExpand(*it, bufrMsg)) {
      string buf;
      encode64(it->c_str(), it->size(), buf);
      LOGERROR(
          "Failed to decode bufr (base64): " << endl << buf << endl << "Reason: " << bufrMsg.error);
      ++errorCount;
      continue;
    }

    decodeResult.clear();
    bufrDecoder.decodeBufrMessage(&bufrMsg, &decodeResult);

    if (!decodeResult.saveData(this)) {
      errorCount++;
    }
  }

  msg = "OK!";

  if (errorCount > 0) {
    LOGWARN(
        "BUFR: #" << bufrList.size()-errorCount << " Observation(s) decoded and saved. #" << errorCount << " failed to decode.");
  } else {
    LOGINFO(
        "BUFR: SUCCESS: #" << bufrList.size() << " Observation(s) decoded and saved!");
  }

  return Ok;
}

bool DecodeKvResult::saveData(BufrDecoder *decoder) {
  miutil::miTime tbTime(miutil::miTime::nowTime());

  if (wmono_ != INT_MAX)
    stationid_ = decoder->getStationid(wmono_);

  if (stationid_ < 0) {
    LOGWARN("BUFR: No stationid found for wmono <" << wmono_ << ">.");
    return false;
  }

  if (obstime_.undef()) {
    LOGWARN("BUFR: No obstime in BUFR message for wmono <" << wmono_ << ">.");
    return false;
  }
  std::map<int, miutil::miTime>::iterator itTbTime = stationidTbTimeList.find(
      stationid_);

  if (itTbTime != stationidTbTimeList.end() && itTbTime->second == tbTime)
    tbTime.addSec(1);

  stationidTbTimeList[stationid_] = tbTime;
  std::list<kvalobs::kvData> kvDataList;

  for (std::list<Data>::const_iterator itData = data_.begin();
      itData != data_.end(); ++itData) {
    kvDataList.push_back(
        kvalobs::kvData(stationid_, itData->obstime, itData->value,
                        itData->paramid, tbTime, typeid_, 0, 0, itData->value,
                        kvalobs::kvControlInfo(), kvalobs::kvUseInfo(), ""));
  }

  bool rejected;
  string rejectMessage;
  decoder->saveData(kvDataList, rejected, rejectMessage);
  return true;
}

