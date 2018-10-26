/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kldecoder.cc,v 1.7.2.7 2007/09/27 09:02:29 paule Exp $                                                       

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
#include <cctype>
#include <sstream>
#include <limits.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <puTools/miTime.h>
#include <miutil/commastring.h>
#include <milog/milog.h>
#include <stdlib.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvTypes.h>
#include <miutil/trimstr.h>
#include "KvDataContainer.h"
#include <miutil/timeconvert.h>
#include "kldecoder.h"
#include <decodeutility/decodeutility.h>
#include <kvalobs/kvPath.h>

namespace pt = boost::posix_time;
using namespace kvalobs::decoder::kldecoder;
using namespace std;
using namespace dnmi::db;
using namespace miutil;
using namespace boost;
using namespace kvalobs;

namespace {
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

class GetObsHelper {
  istringstream in_;
  string obstype_;
  int nMessages;

  bool getLine(string &line, bool &isObsType) {
    isObsType = false;
    string::size_type i;
    while (getline(in_, line)) {
      boost::trim(line);
      if (line.empty())
        continue;
      i = line.find("kldata");

      if (i != string::npos) {
        isObsType = true;
        obstype_ = line;
      }
      return true;
    }
    return false;
  }

  bool getObsType(string &obstype) {
    obstype.erase();
    bool isObsType;
    string dummy;

    if (obstype_.empty()) {
      while (getLine(dummy, isObsType))
        if (isObsType)
          break;
    }
    obstype = obstype_;
    obstype_.erase();
    return !obstype.empty();
  }

 public:
  GetObsHelper(const std::string &data, const std::string &obsType_)
      : in_(boost::trim_copy(data)),
        obstype_(boost::trim_copy(obsType_)),
        nMessages(0) {
  }

  bool eof() const {
    return in_.eof();
  }

  bool getObs(string &obstype, string &data) {
    ostringstream buf;
    bool isObsType;
    data.erase();

    if (!getObsType(obstype))
      return false;

    while (getLine(data, isObsType)) {
      if (isObsType)
        break;
      buf << data << "\n";
    }
    data = buf.str();

    if (!data.empty())
      ++nMessages;

    return !data.empty();
  }

  bool multiMessage() const {
    return nMessages > 1;
  }
};

string getFirstLine(const std::string &buf) {
  string::size_type i = buf.find("\n");
  if (i != string::npos)
    return buf.substr(0, i);
  else
    return buf;
}

std::string removeRedirectFromObsType(const std::string &obstype) {
  string::size_type i = obstype.find("redirected");
  string buf;
  string ret;

  if (i == string::npos)
    return obstype;
  ret = obstype.substr(0, i);
  buf = obstype.substr(i);

  i = buf.find('/');

  if (i != string::npos)
    buf = buf.substr(i);
  else
    buf.erase();

  boost::trim_if(ret, boost::is_any_of(" \r\n/"));
  return ret + buf;
}

}

kvalobs::decoder::kldecoder::KlDecoder::KlDecoder(
    dnmi::db::Connection &con, const ParamList &params,
    const std::list<kvalobs::kvTypes> &typeList, const std::string &obsType,
    const std::string &obs, int decoderId)
    : DecoderBase(con, params, typeList, obsType, obs, decoderId),
      datadecoder(paramList, typeList),
      typeID( INT_MAX),
      stationID(INT_MAX),
      onlyInsertOrUpdate(false)

{
  decodeObsType(obsType);
}

kvalobs::decoder::kldecoder::KlDecoder::~KlDecoder() {
}

void kvalobs::decoder::kldecoder::KlDecoder::decodeObsType(
    const std::string &obstype) {
  string keyval;
  string key;
  string val;
  string::size_type i;
  string::size_type iKey;
  CommaString cstr(obstype, '/');
  long id;
  const char *keys[] = { "nationalnr", "stationid", "wmonr", "icaoid",
      "call_sign", "type", "add", "received_time", "redirected", 0 };

  LOGDEBUG("decodeObsType: '" << obstype << "'");
  typeID = INT_MAX;
  stationID = INT_MAX;
  onlyInsertOrUpdate = false;
  receivedTime = boost::posix_time::ptime();
  

  if (cstr.size() < 2) {
    LOGERROR("decodeObsType: To few keys!");
//        msg="obsType: Invalid Format!";
    return;
  }

  for (int index = 1; index < cstr.size(); ++index) {
    if (!cstr.get(index, keyval)) {
      LOGERROR("decodeObsType: INTERNALERROR: InvalidFormat!");
      return;
    }

    if (!decodeKeyVal(keyval, key, val))  //keyval empty
      continue;

    for (iKey = 0; keys[iKey]; ++iKey) {
      if (key == keys[iKey])
        break;
    }

    if (!keys[iKey]) {
      LOGWARN("decodeObsType: unknown key '" << key << "'");
      continue;
    }

    if (val.empty() && key != "add")  //Must have a value
      continue;

    if (key == "add") {  //Value is optional
      if (val.empty() || val[0] == 't' || val[0] == 'T')
        onlyInsertOrUpdate = true;
    } else if (key == "received_time") {
      receivedTime = pt::time_from_string_nothrow(val);
    } else if (key == "redirected") {
      redirectedFrom = val;
    } else if (key == "type") {
      typeID = atoi(val.c_str());
    } else if (key == "nationalnr" || key == "stationid" || key == "wmonr"
        || key == "icaoid" || key == "call_sign") {

      stationidIn = key + "=" + val;
      stationID = DecoderBase::getStationId(key, val);

      if (stationID < 0) {
        stationID = INT_MAX;
        LOGERROR("No station with stationid '" << key << "=" << val << "'");
        continue;
      }
    } else {
      LOGERROR(
          "decodeObsType: INTERNAL: unhandled key '" << key << "', this is a error and must be fixed.");
    }
  }

  LOGDEBUG(
      "decodeObsType: stationID: " << stationID << " typeid: " << typeID << " update: " << (onlyInsertOrUpdate?"true":"false"));
}

std::string kvalobs::decoder::kldecoder::KlDecoder::name() const {
  return string("KlDataDecoder");
}

kvalobs::decoder::DecoderBase::DecodeResult kvalobs::decoder::kldecoder::KlDecoder::rejected(
    const std::string &msg, const std::string &logid, std::string &msgToSender,
    bool includeObs) {
  ostringstream ost;
  string decoder;
  boost::posix_time::ptime tbtime;
  bool saved = true;

  decoder = ost.str();
  ost.str("");

  tbtime = boost::posix_time::microsec_clock::universal_time();

  ost << "message: " << msg;

  if (includeObs)
    ost << endl << "obsType: " << obsType << endl << "obs: [" << obs << "]";

  ostringstream myObs;

  myObs << obsType;

  if (includeObs)
    myObs << endl << obs;

  msgToSender += ost.str();

  kvalobs::kvRejectdecode rejected(myObs.str(), tbtime, name(), msg);

  if (!putRejectdecodeInDb(rejected)) {
    saved = false;
    ost << endl << "Can't save rejected observation!";
  } else {
    ost << endl << "Saved to rejectdecode!";
  }

  if (!logid.empty()) {
    IDLOGERROR(
        logid,
        "Rejected: " << msg << endl << obs.c_str() << (saved?"":"\nFailed to save to 'rejected'."));
  }

  LOGERROR(ost.str());
  return Rejected;
}

kvalobs::decoder::DecoderBase::DecodeResult kvalobs::decoder::kldecoder::KlDecoder::insertDataInDb(
    kvalobs::serialize::KvalobsData *theData, int stationid, int typeId,
    const std::string &logid, std::string &msgToSender) {
  using namespace boost::posix_time;
  KvDataContainer::DataByObstime data;
  KvDataContainer::TextDataByObstime textData;
  KvDataContainer::TextDataByObstime::iterator tid;
  KvDataContainer::TextDataList td;
  map<ptime, int> observations;

  KvDataContainer container(theData);
  int priority = 4;

  if (receivedTime.is_special())
    priority = 10;


  if (container.get(data, textData, stationid, typeId,
                    pt::second_clock::universal_time()) < 0) {
    IDLOGINFO(logid, "No Data.");
    return Ok;
  }

  for (KvDataContainer::DataByObstime::iterator it = data.begin();
      it != data.end(); ++it) {

    td.clear();
    tid = textData.find(it->first);

    if (tid != textData.end()) {
      td = tid->second;
      textData.erase(tid);
    }

    if (!addDataToDb(to_miTime(it->first), stationid, typeId, it->second, td,
                     priority, logid, getOnlyInsertOrUpdate())) {
      ostringstream ost;

      ost << "DBERROR: stationid: " << stationid << " typeid: " << typeId
          << " obstime: " << it->first;
      LOGERROR(ost.str());
      IDLOGERROR(logid, ost.str());
      msgToSender += "\n" + ost.str();
      return NotSaved;
    }

    observations[it->first] += it->second.size();
  }

  //Is there any left over text data.
  if (!textData.empty()) {
    KvDataContainer::DataList dl;
    for (KvDataContainer::TextDataByObstime::iterator it = textData.begin();
        it != textData.end(); ++it) {
      if (!addDataToDb(to_miTime(it->first), stationid, typeId, dl, it->second,
                       priority, logid, getOnlyInsertOrUpdate())) {
        ostringstream ost;
        ost << "DBERROR: TextData: stationid: " << stationid << " typeid: "
            << typeId << " obstime: " << it->first;
        LOGERROR(ost.str());
        IDLOGERROR(logid, ost.str());
        msgToSender += "\n" + ost.str();
        return NotSaved;
      }
      observations[it->first] += it->second.size();
    }
  }

  ostringstream ost;
  int totalObservations = 0;
  if (observations.size() > 0) {
    for (map<ptime, int>::const_iterator it = observations.begin();
        it != observations.end(); ++it) {
      ost << "\n# observations " << to_kvalobs_string(it->first) << ": "
          << it->second;
      totalObservations += it->second;
    }
  }

  ostringstream msgOst;
  IDLOGINFO(
      logid,
      "Total number of observations saved to DB: " << totalObservations << " stationid '" << stationid << "' typeid '" << typeId << "'" << endl << ost.str());
  LOGINFO(
      "Total number of observations saved to DB: " << totalObservations << " stationid '" << stationid << "' typeid '" << typeId << "'");

  msgOst << "Total number of observations saved to DB: " << totalObservations
         << " for stationid '" << stationid << "', typeid '" << typeId << "'"
         << endl << ost.str();
  msgToSender += "\n" + msgOst.str();
  return Ok;
}

kvalobs::decoder::DecoderBase::DecodeResult kvalobs::decoder::kldecoder::KlDecoder::execute(
    std::string &msg) {
  ostringstream ostMsg;
  GetObsHelper helper(obs, obsType);
  bool error = false;
  DecodeResult ret;
  string retMsg;
  string line;

  while (helper.getObs(obsType, obs) && !error) {
    if (helper.multiMessage())
      ostMsg << "<<<<<<<<<<<<<<<<<<<< DECODER RESULT >>>>>>>>>>>>>>>>>>>>\n";

    decodeObsType(obsType);
    obsType = removeRedirectFromObsType(obsType);
    retMsg.erase();
    ret = doExecute(retMsg);

    switch (ret) {
      case Ok:
        ostMsg << "OK";
        break;
      case NotSaved:
        ostMsg << "NOTSAVED";
        error = true;
        break;
      case Rejected:
        ostMsg << "REJECTED";
        break;
      case Error:
        ostMsg << "ERROR";
        error = true;
        break;
      case Redirect:
        ostMsg << "REDIRECT";
        break;
    }
    ostMsg << ": " << obsType << endl << retMsg << "\n";
  }

  if (helper.multiMessage())
    msg = "<<<<<<<<<<<<<<<<<<<< DECODER RESULT >>>>>>>>>>>>>>>>>>>>\n";

  msg += ostMsg.str();

  if (helper.multiMessage())
    msg += "\n<<<<<<<<<<<<<<<<<<<< END >>>>>>>>>>>>>>>>>>>>\n";

  if (error || !helper.multiMessage())
    return ret;
  else
    return Ok;
}

kvalobs::decoder::DecoderBase::DecodeResult kvalobs::decoder::kldecoder::KlDecoder::doExecute(
    std::string &msg) {
  bool setUsinfo7 = getSetUsinfo7();
  int typeId = getTypeId(msg);
  int stationid = getStationId(msg);
  string decoder = name();
  ostringstream o;

  if (!redirectedFrom.empty())
    decoder += "." + redirectedFrom;

  milog::LogContext lcontext(decoder);
  LOGDEBUG(obsType << "\n" << obs);
  logid.clear();

  if (stationid == INT_MAX) {
    o.str("");

    o << "Missing or unknown stationid! " << stationidIn << " typeid: ";

    if (typeId > 0)
      o << typeId;
    else
      o << "<NA>";

    return rejected(o.str(), "", msg, false);
  }

  if (typeId < 0 || typeId == INT_MAX) {
    o.str("");
    o << "Format error in type. " << stationidIn << ".";

    return rejected(o.str(), "", msg, false);
  }

  if (receivedTime.is_special() && setUsinfo7)
    receivedTime = pt::second_clock::universal_time();

  o.str("");
  o << "New observation. stationid: " << stationid << " typeid: " << typeId;

  if (!receivedTime.is_special())
    o << " Obs. received: " << receivedTime;

  LOGINFO(o.str());

  IdlogHelper idLog(stationid, typeId, this);
  logid = idLog.logid();

  trimstr(obs);
  obs += "\n";

  o.str("");
  o << name() << endl << "------------------------------" << endl
    << "ReceivedTime: ";
  if (receivedTime.is_special())
    o << " NOT given or set_useinfo7 is not set.";
  else
    o << receivedTime;

  o << endl << "Insert type : "
    << (getOnlyInsertOrUpdate() ? "update (replenish)" : "insert");

  o << endl << "ObstType    : " << obsType << endl << "Obs         : " << obs
    << endl;

  IDLOGINFO(logid, o.str());

  serialize::KvalobsData *kvData;
  kvData = datadecoder.decodeData(obs, stationid, typeId, receivedTime, logid,
                                  name());

  if (!kvData)
    return rejected(datadecoder.messages, logid, msg);

  if (datadecoder.warnings) {
    IDLOGWARN(logid, datadecoder.messages);
  } else {
    IDLOGINFO(logid, datadecoder.messages);
  }

  msg = datadecoder.messages;
  return insertDataInDb(kvData, stationid, typeId, logid, msg);
}

long kvalobs::decoder::kldecoder::KlDecoder::getStationId(
    std::string &msg) const {
  return stationID;
}

bool kvalobs::decoder::kldecoder::KlDecoder::getOnlyInsertOrUpdate() const {
  return onlyInsertOrUpdate;
}

bool kvalobs::decoder::kldecoder::KlDecoder::getSetUsinfo7() {
  bool setUsinfo7 = false;
  miutil::conf::ConfSection *conf = myConfSection();
  miutil::conf::ValElementList val = conf->getValue("set_useinfo7");

  if (val.size() > 0) {
    string v = val[0].valAsString();
    if (v.size() > 0 && (v[0] == 't' || v[0] == 'T'))
      setUsinfo7 = true;
  }

  return setUsinfo7;
}

long kvalobs::decoder::kldecoder::KlDecoder::getTypeId(std::string &msg) const {
  return typeID;
}

