/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: decoder.cc,v 1.41.2.5 2007/09/27 09:02:27 paule Exp $

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
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <tuple>
#include <map>
#include "boost/thread.hpp"
#include "puTools/miTime.h"
#include "lib/decoder/decoderbase/ConfParser.h"
#include "lib/decoder/decoderbase/DataUpdateTransaction.h"
#include "lib/decoder/decoderbase/RedirectInfo.h"
#include "lib/decoder/decoderbase/decoder.h"
#include "lib/decoder/decoderbase/metadata.h"
#include "lib/decodeutility/getUseInfo7.h"
#include "lib/decodeutility/isTextParam.h"
#include "lib/kvalobs/getLogInfo.h"
#include "lib/kvalobs/kvDbGate.h"
#include "lib/kvalobs/kvGeneratedTypes.h"
#include "lib/kvalobs/kvPath.h"
#include "lib/kvalobs/kvQueries.h"
#include "lib/kvalobs/kvWorkelement.h"
#include "lib/miutil/SemiUniqueName.h"
#include "lib/miutil/commastring.h"
#include "lib/miutil/timeconvert.h"
#include "lib/miutil/trimstr.h"

using std::string;
using std::endl;
using std::list;
using std::ostringstream;
using std::vector;
using std::ifstream;
using std::list;
using std::tuple;
using kvalobs::kvData;
using kvalobs::kvTextData;

namespace {

/**
 * Sort in lists where each elements in the lists have the same stationid, typeid and obstime.
 */
std::list<std::list<kvalobs::kvData>> collate(const std::list<kvalobs::kvData> &data) {
  namespace pt = boost::posix_time;
  using std::map;
  using std::list;
  using kvalobs::kvData;

  map<long, map<long, map<pt::ptime, list<kvData>>> > collated;
  list<list<kvData>> ret;

  for (auto &d : data) {
    collated[d.stationID()][d.typeID()][d.obstime()].push_back(d);
  }

  for (auto &sid : collated) {
    for (auto &tid : sid.second) {
      for (auto &obst : tid.second) {
        ret.push_back(std::move(obst.second));
      }
    }
  }
  return ret;
}
}  // namespace

namespace kvdatainput {
namespace decodecommand {
extern boost::thread_specific_ptr<kvalobs::decoder::RedirectInfo> ptrRedirect;
}
}

kvalobs::decoder::ObsPgmParamInfo::ObsPgmParamInfo() {
}

kvalobs::decoder::ObsPgmParamInfo::ObsPgmParamInfo(const ObsPgmParamInfo &cs)
    : obsPgm(cs.obsPgm),
      obstime(cs.obstime) {
}

kvalobs::decoder::ObsPgmParamInfo&
kvalobs::decoder::ObsPgmParamInfo::operator=(const ObsPgmParamInfo &rhs) {
  if (&rhs != this) {
    obsPgm = rhs.obsPgm;
    obstime = rhs.obstime;
  }

  return *this;
}

bool kvalobs::decoder::ObsPgmParamInfo::isActive(int stationid, int typeid_, int paramid, int sensor, int level, const miutil::miTime &obstime,
                                                 Active &state) const {
  state = NO;

  for (CIObsPgmList it = obsPgm.begin(); it != obsPgm.end(); ++it) {
    if (it->stationID() == stationid && it->typeID() == typeid_ && it->paramID() == paramid && it->level() == level && it->nr_sensor() > sensor) {
      if (it->isOn(to_ptime(obstime))) {
        if (it->collector())
          state = MAYBE;
        else
          state = YES;

        return true;
      }
    }
  }

  return false;
}

kvalobs::decoder::DecoderBase::DecoderBase(dnmi::db::Connection &con_, const ParamList &params, const std::list<kvalobs::kvTypes> &typeList_,
                                           const std::string &obsType_, const std::string &obs_, int decoderId_)
    : decoderId(decoderId_),
      con(con_),
      paramList(params),
      typeList(typeList_),
      obsType(obsType_),
      obs(obs_),
      theKvConf(0),
      filters( new StationFilters() ){
}

kvalobs::decoder::DecoderBase::~DecoderBase() {
  list<string>::iterator it = createdLoggers.begin();

  for (; it != createdLoggers.end(); ++it) {
    milog::Logger::removeLogger(*it);
  }
}


void kvalobs::decoder::DecoderBase::setFilters( const kvalobs::decoder::StationFiltersPtr filters_){
  filters=filters_;
}

kvalobs::decoder::StationFilterElement
kvalobs::decoder::DecoderBase::filter(long stationId, long typeId)const
{
  return filters->filter(stationId, typeId);
}

std::tuple<std::list<kvalobs::kvData>, std::list<kvalobs::kvTextData>>
kvalobs::decoder::DecoderBase::filterSaveDataToDb( const std::list<kvalobs::kvData> &d, const std::list<kvalobs::kvTextData> &td)const
{
  return filters->saveDataToDb( d, td);
}

bool kvalobs::decoder::DecoderBase::dataToPublish( const std::list<kvalobs::kvData> &d, const std::list<kvalobs::kvTextData> &td)
{
  kvalobs::serialize::KvalobsData pubData = filters->publishData(d,td);

  if( !pubData.empty() ) {
    publishData.push_back(std::move(pubData));
    return true;
  }
  return false;

}


std::string kvalobs::decoder::DecoderBase::semiuniqueName(const std::string &prefix, const char *endsWith) {
  return miutil::SemiUniqueName::uniqueName(prefix, endsWith);
}

/**
 * This is a creative use of thread specific data to make a
 * binary compatible transfer of data between a decoder and
 * a decoder command. The DecoderBase class is binary compatible
 * with old code that use it.
 */
bool kvalobs::decoder::DecoderBase::setRedirectInfo(const std::string &obsType, const std::string &data) {
  RedirectInfo *redirectInfo = kvdatainput::decodecommand::ptrRedirect.get();

  if (!redirectInfo)
    return false;

  redirectInfo->decoder(name());
  redirectInfo->obsType(obsType);
  redirectInfo->data(data);

  return true;
}

void kvalobs::decoder::DecoderBase::setKvConf(miutil::conf::ConfSection *theKvConf_) {
  theKvConf = theKvConf_;
}

char kvalobs::decoder::DecoderBase::getUseinfo7Code(int typeId, const boost::posix_time::ptime &now, const boost::posix_time::ptime &obt,
                                                    const std::string &logid) {
  int flag = decodeutility::getUseinfo7Code(typeId, now, obt, typeList);

  if (flag < 0) {
    if (logid.empty()) {
      LOGWARN("Unknown typeid: " << typeId);
    } else {
      IDLOGWARN(logid, "Unknown typeid: " << typeId);
    }

    return 0;
  }

  return flag;
}

void kvalobs::decoder::DecoderBase::setMetaconf(const std::string &metaconf_) {
  namespace m = miutil::parsers::meta;

  m::MetaParser parser;
  m::Meta *meta = parser.parse(metaconf_);

  LOGDEBUG6("Metaconf: " << endl << metaconf_);

  if (meta) {
    if (!parser.getWarnings().empty()) {
      LOGWARN("Warnings from parsing metaconf!" << endl << metaconf_ << endl << "Reason: " << parser.getWarnings());
    }

    metaconf = *meta;
    delete meta;
  } else {
    LOGERROR("Error parsing metaconf!" << endl << metaconf_ << endl << "Reason: " << parser.getErrMsg());
  }
}

std::string kvalobs::decoder::DecoderBase::getObsTypeKey(const std::string &keyToFind_) const {
  string keyval;
  string key;
  string val;
  string keyToFind(keyToFind_);

  miutil::trimstr(keyToFind);

  if (keyToFind.empty())
    return "";

  miutil::CommaString cstr(obsType, '/');

  for (int i = 0; i < cstr.size(); ++i) {
    if (!cstr.get(i, keyval))
      continue;

    miutil::CommaString tmpKeyVal(keyval, '=');

    if (tmpKeyVal.size() < 2)
      continue;

    tmpKeyVal.get(0, key);
    tmpKeyVal.get(1, val);

    miutil::trimstr(key);

    if (key == keyToFind) {
      if (key.size() >= 2)
        return val;
    }
  }

  return "";
}

std::string kvalobs::decoder::DecoderBase::getMetaSaSd() const {
  string meta = getObsTypeKey("meta_SaSd");

  if (meta.length() < 2)
    return "";

  return meta;
}

const kvalobs::kvTypes*
kvalobs::decoder::DecoderBase::findType(int typeid_) const {
  std::list<kvalobs::kvTypes>::const_iterator it = typeList.begin();

  for (; it != typeList.end(); ++it) {
    if (it->typeID() == typeid_)
      return &(*it);
  }

  return 0;
}

void kvalobs::decoder::DecoderBase::addStationInfo(long stationid, const miutil::miTime &obstime, long typeid_, const miutil::miTime &tbtime, int priority) {
  IkvStationInfoList it = stationInfoList.begin();

  for (; it != stationInfoList.end(); ++it) {
    if (it->stationID() == stationid && it->obstime() == to_ptime(obstime) && it->typeID() == typeid_) {
      return;
    }
  }

  kvDbGate gate(&con);
  boost::posix_time::ptime undefTime;

  if (!gate.insert(kvWorkelement(stationid, to_ptime(obstime), typeid_, to_ptime(tbtime), priority, undefTime, undefTime, undefTime, undefTime, undefTime),
                   true)) {
    LOGERROR("addStationInfo: can't save kvWorkelement into the" << endl << "the table 'workque' in  the database!\n" << "[" << gate.getErrorStr() << "]");
    return;
  }

  LOGDEBUG(
      "addStation: station added to the table 'workque'." << endl << "-- Stationid: " << stationid << endl << "-- obstime:   " << obstime << endl << "-- typeid:    " << typeid_ << endl << "-- priority:  " << priority);

  stationInfoList.push_back(kvStationInfo(stationid, to_ptime(obstime), typeid_));
}

void kvalobs::decoder::DecoderBase::updateStationInfo(const kvalobs::kvStationInfoList& stationInfo) {
  IkvStationInfoList it = stationInfoList.begin();

  for (kvalobs::kvStationInfoList::const_iterator nit = stationInfo.begin(); nit != stationInfo.end(); ++nit) {
    for (; it != stationInfoList.end(); ++it) {
      if (it->stationID() == nit->stationID() && it->obstime() == nit->obstime() && it->typeID() == nit->typeID()) {
        break;
      }
    }
    if (it == stationInfoList.end())
      stationInfoList.push_back(*nit);
  }
}

bool kvalobs::decoder::DecoderBase::isGenerated(long stationid, long typeid_) {
  kvDbGate gate(&con);

  for (list<GenCachElem>::iterator it = genCachElem.begin(); it != genCachElem.end(); ++it) {
    if (it->stationID() == stationid && it->typeID() == typeid_) {
      return it->generated();
    }
  }

  list<kvGeneratedTypes> stList;

  if (!gate.select(stList, kvQueries::selectIsGenerated(stationid, typeid_))) {
    LOGDEBUG("isGenerated: DBERROR: " << gate.getErrorStr());
    throw dnmi::db::SQLException(gate.getErrorStr());
  }

  genCachElem.push_back(GenCachElem(stationid, typeid_, !stList.empty()));

  return !stList.empty();
}

int kvalobs::decoder::DecoderBase::getDecoderId() const {
  return decoderId;
}

long kvalobs::decoder::DecoderBase::getStationId(const std::string &key, const std::string &value) {
  namespace db = dnmi::db;

  string query("SELECT stationid FROM station WHERE ");
  query += key + "=" + value;

  LOGDEBUG("DecoderBase::getStationId: query(" << query << ")\n");

  db::Result *res;

  try {
    long stationId = -1;
    res = con.execQuery(query);

    if (res) {
      if (res->hasNext()) {
        db::DRow row = res->next();
        stationId = atol(row[0].c_str());
      }
      delete res;
    }

    return stationId;
  } catch (const db::SQLException &ex) {
    LOGERROR("Exception: " << ex.what() << endl);
    return -2;
  }
}

bool kvalobs::decoder::DecoderBase::deleteKvDataFromDb(const kvalobs::kvData &sd) {
  kvDbGate gate(&con);
  std::string query(kvQueries::selectData(sd));
  std::ostringstream ost;

  ost << "delete from " << sd.tableName() << " " << query;

  try {
    LOGDEBUG("deleteKvDataFromDb: (delete): " << ost.str() << "\n");
    con.exec(ost.str());
  } catch (...) {
    return false;
  }

  return true;
}

bool kvalobs::decoder::DecoderBase::putKvDataInDb(const kvalobs::kvData &sd_, int priority) {
  kvalobs::kvData sd(sd_);
  kvDbGate gate(&con);

  try {
    if (isGenerated(sd.stationID(), sd.typeID())) {
      LOGDEBUG("GENERATEDDATA: stationid: " << sd.stationID() << " typeid: " << sd.typeID() << " obstime: " << sd.obstime());

      sd.typeID(-1 * sd.typeID());
    }
  } catch (...) {
    // Do nothing. The log message from isGenerated is enough for
    // the momment.
  }

  if (!gate.insert(sd, true)) {
    LOGDEBUG("putKvDataInDb: can't save kvData to the database!\n" << "[" << gate.getErrorStr() << "]");

    return false;
  }

  addStationInfo(sd.stationID(), to_miTime(sd.obstime()), sd.typeID(), to_miTime(sd.tbtime()), priority);

  return true;
}

bool kvalobs::decoder::DecoderBase::putKvDataInDb(const std::list<kvalobs::kvData> &sd, int priority) {
  using std::list;
  using kvalobs::kvData;
  kvDbGate gate(&con);

  if (sd.empty())
    return true;

  list<list<kvData>> allData = collate(sd);
  for (auto &dl : allData) {
    auto &d = dl.front();
    addStationInfo(d.stationID(), to_miTime(d.obstime()), d.typeID(), to_miTime(d.tbtime()), priority);
    for (auto &de : dl) {
      if (!gate.insert(de, true)) {
        LOGERROR("putKvDataInDb: can't save kvData to the database!\n" << "[" << gate.getErrorStr() << "]");
        return false;
      }
    }
  }

  return true;
}

bool kvalobs::decoder::DecoderBase::addDataToDb(const miutil::miTime &obstime, int stationid, int typeid_, std::list<kvalobs::kvData> &sd,
                                                std::list<kvalobs::kvTextData> &textData, int priority, const std::string &logid) {
  return addDataToDb(obstime, stationid, typeid_, sd, textData, priority, logid, false);
}

bool kvalobs::decoder::DecoderBase::addDataToDb(const miutil::miTime &obstime, int stationid, int typeid_, std::list<kvalobs::kvData> &sd,
                                                std::list<kvalobs::kvTextData> &textData, int priority, const std::string &logid, bool onlyAddOrUpdateData) {
  namespace pt = boost::posix_time;

  boost::gregorian::date date(obstime.year(), obstime.month(), obstime.day());
  boost::posix_time::time_duration clock(pt::hours(obstime.hour()) + pt::minutes(obstime.min()) + pt::seconds(obstime.sec()));
  boost::posix_time::ptime pt_obstime(date, clock);
  tuple<list<kvData>, list<kvTextData>> data=filterSaveDataToDb( sd, textData);
  dataToPublish(sd, textData);

  if( std::get<0>(data).empty() && std::get<1>(data).empty()) {
    // No data to save to the database.
    return true;
  }

  kvalobs::decoder::DataUpdateTransaction work(pt_obstime, stationid, typeid_, priority, &std::get<0>(data), &std::get<1>(data), logid, onlyAddOrUpdateData);

  try {
    con.perform(work);
    updateStationInfo(work.stationInfoList());
    decodedData.push_back(work.insertedOrUpdatedData());
  } catch (...) {
  }

  return work.ok();
}

bool kvalobs::decoder::DecoderBase::putkvTextDataInDb(const kvalobs::kvTextData &td_, int priority) {
  kvalobs::kvTextData td(td_);
  kvDbGate gate(&con);

  try {
    if (isGenerated(td.stationID(), td.typeID())) {
      LOGDEBUG("GENERATEDDATA: stationid: " << td.stationID() << " typeid: " << td.typeID() << " obstime: " << td.obstime());

      td.typeID(-1 * td.typeID());
    }
  } catch (...) {
    // COMMENT:
    // do nothing. The log message from isGenerated is enough for
    // the momment.
  }

  if (!gate.insert(td, true)) {
    LOGDEBUG("putkvTextDataInDb: can't save kvTextData to the database!\n" << "[" << gate.getErrorStr() << "]");

    return false;
  }

  addStationInfo(td.stationID(), to_miTime(td.obstime()), td.typeID(), to_miTime(td.tbtime()), priority);

  return true;
}

bool kvalobs::decoder::DecoderBase::putkvTextDataInDb(const std::list<kvalobs::kvTextData> &td_, int priority) {
  std::list<kvalobs::kvTextData> td(td_);
  std::list<kvalobs::kvTextData>::iterator it = td.begin();
  long sid = -1;
  long tid;
  long myTid;
  miutil::miTime obsTime;
  miutil::miTime tbTime;

  if (it == td.end())
    return true;

  kvDbGate gate(&con);

  for (; it != td.end(); ++it) {
    if (sid == -1 || sid != it->stationID() || tid != it->typeID() || obsTime != to_miTime(it->obstime())) {
      if (sid != -1) {
        addStationInfo(sid, obsTime, tid, tbTime, priority);
      }

      sid = it->stationID();
      tid = it->typeID();
      obsTime = to_miTime(it->obstime());
      tbTime = to_miTime(it->tbtime());
      myTid = tid;

      try {
        if (myTid > 0 && isGenerated(sid, tid)) {
          LOGDEBUG("GENERATEDDATA: stationid: " << it->stationID() << " typeid: " << it->typeID() << " obstime: " << it->obstime());

          myTid = -1 * tid;
        }
      } catch (...) {
        // Do nothing. The log message from isGenerated is enough for
        // the momment.
      }
    }

    it->typeID(myTid);

    if (!gate.insert(*it, true)) {
      LOGDEBUG("putkvTextDataInDb: can't save kvTextData to the database!\n" << "[" << gate.getErrorStr() << "]");

      return false;
    }
  }

  addStationInfo(sid, obsTime, tid, tbTime, priority);

  return true;
}

bool kvalobs::decoder::DecoderBase::putRejectdecodeInDb(const kvalobs::kvRejectdecode &sd) {
  kvDbGate gate(&con);
  kvRejectdecode reject(gate.esc(sd.message()), sd.tbtime(), gate.esc(sd.decoder()), gate.esc(sd.comment()), sd.fixed());

  if (!gate.insert(reject, true)) {
    LOGERROR("putRejectdecodeInDb: can't save kvRejectdecode to the database!\n" << "[" << gate.getErrorStr() << "]");

    return false;
  }

  return true;
}

bool kvalobs::decoder::DecoderBase::putkvStationInDb(const kvalobs::kvStation &st) {
  kvDbGate gate(&con);

  if (!gate.insert(st, true)) {
    LOGERROR("kvStationInDb: can't save kvStation to the database!\n" << "[" << gate.getErrorStr() << "]");

    return false;
  }

  return true;
}

bool kvalobs::decoder::DecoderBase::isTextParam(const std::string &paramname) {
  return decodeutility::isTextParam(paramname, paramList);
}

bool kvalobs::decoder::DecoderBase::isTextParam(int paramid) {
  return decodeutility::isTextParam(paramid, paramList);
}

bool kvalobs::decoder::DecoderBase::loadConf(int sid, int tid, kvalobs::decoder::ConfParser &parser) {
  namespace c = miutil::conf;
  c::ConfParser myparser;
  ostringstream fnames;
  ifstream fis;

  fnames << kvPath(sysconfdir);

  fnames << "/decode/" << name() << ".conf";

  std::string fname = fnames.str();

  fis.open(fname.c_str());

  if (!fis) {
    LOGERROR("Cant open the configuration file <" << fname << ">!" << endl);
  } else {
    LOGINFO("Reading configuration from file <" << fname << ">!" << endl);
    c::ConfSection *conf = myparser.parse(fis);

    if (!conf) {
      LOGERROR("Error while reading configuration file: <" << fname << ">!" << endl << myparser.getError() << endl);
    } else {
      LOGINFO("Configuration file loaded!\n");
      parser.parse(*conf);

      return true;
    }
  }

  return false;
}

miutil::conf::ConfSection*
kvalobs::decoder::DecoderBase::myConfSection() {
  miutil::conf::ConfSection *conf;
  ostringstream sectionName;

  if (!theKvConf) {
    LOGDEBUG("myConfSection: driver <" << name() << "> has NOT implemented use of data from <kvalobs.conf>.");
    return 0;
  }

  sectionName << "kvDataInputd." << name();
  conf = theKvConf->getSection(sectionName.str());

  if (!conf) {
    LOGDEBUG("No configuration section defined in <kvalobs.conf> for the section '" << sectionName.str() << "'.");
  }

  return conf;
}

miutil::conf::ValElementList kvalobs::decoder::DecoderBase::getKeyFromConf(const std::string &key, const miutil::conf::ValElementList &defaultValue) {
  namespace c = miutil::conf;

  if (!theKvConf) {
    LOGDEBUG("getKeyFromConf: driver <" << name() << "> has NOT implemented use of data from <kvalobs.conf>.");
    return defaultValue;
  }

  c::ValElementList val = theKvConf->getValue(key);

  if (val.empty())
    return defaultValue;
  else
    return val;
}

miutil::conf::ValElementList kvalobs::decoder::DecoderBase::getKeyInMyConfSection(const std::string &key, const miutil::conf::ValElementList &defaultValue) {
  namespace c = miutil::conf;
  c::ConfSection *conf = myConfSection();

  if (!conf)
    return defaultValue;

  c::ValElementList val = conf->getValue(key);

  if (val.empty())
    return defaultValue;
  else
    return val;
}

std::string kvalobs::decoder::DecoderBase::createOrCheckDir(const std::string &where, const std::string &dir) {
  list<string> pathlist;
  string path = where;
  ostringstream ost;
  bool error = false;

  if (!path.empty() && path[path.length() - 1] == '/')
    path.erase(path.length() - 1);

  pathlist.push_back(path);
  pathlist.push_back("decoders");
  pathlist.push_back(name());

  if (!dir.empty()) {
    path = dir;

    if (!path.empty() && path[path.length() - 1] == '/')
      path.erase(path.length() - 1);

    vector<string> d;
    string part;
    boost::split(d, path, boost::is_any_of("/"));
    for (vector<string>::iterator it = d.begin(); it != d.end(); ++it) {
      part = boost::trim_copy_if(*it, boost::is_any_of(" \r\t\n"));
      if (!part.empty())
        pathlist.push_back(part);
    }
  }

  ost.str("");

  for (list<string>::iterator it = pathlist.begin(); it != pathlist.end() && !error; ++it) {
    ost << *it;

    if (mkdir(ost.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < -1) {
      if (errno == EEXIST) {
        continue;
      } else {
        error = true;
      }
    }
    ost << "/";
  }

  if (error) {
    ost.str("");
    for (list<string>::iterator it = pathlist.begin(); it != pathlist.end() && !error; ++it)
      ost << *it << "/";

    LOGERROR("Can NOT create the dierctory. A directory in the path maybe missing or a dangling link!\n" << "Path: '" << ost.str() << "'.");

    return "";
  }
  return ost.str();
}

std::string kvalobs::decoder::DecoderBase::logdirForLogger(const std::string &dir) {
  return createOrCheckDir(logdir(), dir);
}

std::string kvalobs::decoder::DecoderBase::datdirForLogger(const std::string &dir) {
  return createOrCheckDir(kvPath(localstatedir), dir);
}

milog::FLogStream*
kvalobs::decoder::DecoderBase::openFLogStream(const std::string &filename) {
  const int defSize = 1024 * 100;
  milog::FLogStream *fs;
  ostringstream ost;
  list<string> pathlist;
  bool error = false;
  int nRotate = 2;
  int fileSize = defSize;

  if (theKvConf) {
    getLogfileInfo(theKvConf, "kvDataInputd." + name(), nRotate, fileSize);

    nRotate = nRotate < 1 ? 2 : nRotate;
    fileSize = fileSize < defSize ? defSize : fileSize;
  }

  try {
    fs = new milog::FLogStream(nRotate, fileSize);
  } catch (...) {
    LOGERROR("OUT OF MEMMORY, cant allocate FLogStream!");
    return 0;
  }

  string path = logdir();

  if (!path.empty() && path[path.length() - 1] == '/')
    path.erase(path.length() - 1);

  if (path.empty())
    error = true;
  else
    pathlist.push_back(path);

  if (error) {
    LOGERROR("Cant open logfile! " << "MISSING/EMPTY 'logdir'" << endl << "Logging all activity to: /dev/null");
    fs->open("/dev/null");

    return fs;
  }

  pathlist.push_back("decoders");
  pathlist.push_back(name());

  error = false;

  ost.str("");

  for (list<string>::iterator it = pathlist.begin(); it != pathlist.end() && !error; ++it) {
    ost << "/" << *it;

    if (mkdir(ost.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < -1) {
      if (errno == EEXIST) {
        continue;
      } else {
        error = true;
      }
    }
  }

  if (error) {
    LOGERROR("A directory in the logpath maybe missing or a dangling link!" << "Path: " <<ost.str() << endl << "Logging all activity to: /dev/null");
    fs->open("/dev/null");
    return fs;
  }

  ost << "/" << filename;

  if (!fs->open(ost.str())) {
    LOGERROR("Failed to create logfile: " << ost.str() << endl << "Using /dev/null!");
    fs->open("/dev/null");
  } else {
    LOGINFO("Logfile (open): " << ost.str());
  }

  return fs;
}

void kvalobs::decoder::DecoderBase::createLogger(const std::string &logname) {
  milog::FLogStream *ls = openFLogStream(logname + ".log");

  if (!ls)
    return;

  createdLoggers.push_back(logname);

  milog::Logger::createLogger(logname, ls);
  milog::Logger::logger(logname).logLevel(getConfLoglevel());
}

void kvalobs::decoder::DecoderBase::removeLogger(const std::string &logname) {
  list<string>::iterator it = createdLoggers.begin();

  for (; it != createdLoggers.end(); ++it) {
    if (*it == logname) {
      milog::Logger::removeLogger(logname);
      createdLoggers.erase(it);
      return;
    }
  }
}

void kvalobs::decoder::DecoderBase::loglevel(const std::string &logname, milog::LogLevel loglevel) {
  // We only changes the loglevel if we have created the logger.

  if (logname.empty())
    return;

  list<string>::iterator it = createdLoggers.begin();

  for (; it != createdLoggers.end(); ++it) {
    if (*it == logname) {
      milog::Logger::logger(logname).logLevel(loglevel);
      return;
    }
  }
}

milog::LogLevel kvalobs::decoder::DecoderBase::getConfLoglevel() const {
  string sectionName;
  milog::LogLevel ll = milog::NOTSET;

  if (!theKvConf)
    return milog::DEBUG;

  sectionName = "kvDataInputd." + name();

  return getLoglevelRecursivt(theKvConf, sectionName, milog::DEBUG);
}

bool kvalobs::decoder::DecoderBase::loadObsPgmParamInfo(int stationid, int typeid_, const miutil::miTime &obstime, ObsPgmParamInfo &paramInfo) const {
  paramInfo.obsPgm.clear();
  paramInfo.obstime = obstime;

  kvDbGate gate(&con);

  if (!gate.select(paramInfo.obsPgm, kvQueries::selectObsPgm(stationid, typeid_, to_ptime(obstime)))) {
    LOGWARN("DBError: Cant access obs_pgm: " << endl << "Reason: " << gate.getErrorStr());
    return false;
  }

  return true;
}

std::string kvalobs::decoder::DecoderBase::logdir() const {
  if (!logdir_.empty())
    return logdir_;
  else
    return kvPath(kvalobs::logdir);
}

void kvalobs::decoder::DecoderBase::logdir(const std::string &logdir) {
  logdir_ = logdir;
  miutil::trimstr(logdir_);

  if (!logdir_.empty() && logdir_[logdir.length() - 1] == '/')
    logdir_.erase(logdir_.length() - 1);
}
