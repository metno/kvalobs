/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kv2kvDecoder.cc,v 1.10.2.12 2007/09/27 09:02:29 paule Exp $

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
#include "kv2kvDecoder.h"
#include <decodeutility/kvDataFormatter.h>
#include <decodeutility/kvalobsdataparser.h>
#include <decodeutility/KvDataContainer.h>
#include <kvalobs/kvexception.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvQueries.h>
#include <decoderbase/decoder.h>
#include <puTools/miTime.h>
#include <milog/milog.h>
#include <miutil/timeconvert.h>
#include <kvdb/transactionhelper.h>
#include <memory>
#include <cstdio>
#include <algorithm>
#include <set>
#include <cmath>
#include <sstream>
#include <functional>
#include <stdexcept>

using namespace std;
using namespace kvalobs::decoder;
using namespace kvalobs::serialize;
using namespace decodeutility::kvdataformatter;
using decodeutility::KvDataContainer;

namespace pt=boost::posix_time;

namespace kvalobs {

namespace decoder {

namespace kv2kvDecoder {

typedef list<kvData> DList;
typedef list<kvTextData> TDList;

kv2kvDecoder::kv2kvDecoder(dnmi::db::Connection & con, const ParamList & params,
                           const list<kvTypes> & typeList,
                           const std::string & obsType, const std::string & obs,
                           int decoderId)
    : DecoderBase(con, params, typeList, obsType, obs, decoderId),
      dbGate(&con),
      tbtime(boost::posix_time::microsec_clock::universal_time()),
      checked_(false) {
  ostringstream s;
  s << name() << " (" << serialNumber << ")";
  milog::LogContext lcontext(name());
  LOGDEBUG("kv2kvDecoder object created");

  try {
    setChecked(obsType);
    parse(data, obs);
    parseResult_ = Ok;
    parseMessage_ = "Ok";
  } catch (DecoderError & e) {
    parseResult_ = Error;
    parseMessage_ = "Could not parse data";
  }
}

kv2kvDecoder::~kv2kvDecoder() {
}

void kv2kvDecoder::setChecked( const std::string &obsType ){
  auto val = getObsTypeKey("checked");

  if (val.empty() ) {
    return;
  }
  
  if (val[0]=='T' || val[0]=='t') {
    checked_=true;
  } else {
    checked_ = false;
  }
}


DecoderBase::DecodeResult kv2kvDecoder::execute(std::string & msg) {
  ostringstream s;
  s << name() << " (" << serialNumber << " - '" << producer_ << "')";
  
  milog::LogContext lcontext(s.str());

  if (parseResult_ != Ok) {
    msg = parseMessage_;
    saveInRejectDecode();
    return parseResult_;
  }
  
  

  try {
    list<kvData> dl;
    list<kvTextData> tdl;

    if ( ! checked_ ) {
      verifyAndAdapt(data, dl);
      data.getData(tdl, tbtime);
    } else {
      //The data is checked, just save tha data to the database
      //We also keep the tbtime as it is from the message.
      data.data(dl, tdl, false);
    }

    save2(dl, tdl);

    KvalobsData::RejectList rejectedFixes;
    data.getRejectedCorrections(rejectedFixes);
    markAsFixed(rejectedFixes);
  } catch (DecoderError & e) {
    parseMessage_ = e.what();
    parseResult_ = e.res;
    saveInRejectDecode();
  }

  msg = parseMessage_;
  return parseResult_;
}

void kv2kvDecoder::saveInRejectDecode() {
  std::string originalMessage = parseMessage_;
  try {
    kvRejectdecode reject(obsType + "\n" + obs, tbtime, name(), parseMessage_);
    if (!this->putRejectdecodeInDb(reject))
      throw std::runtime_error("Unable to save data in rejectdecode table!");
  } catch (std::exception & e) {
    originalMessage = originalMessage + '\n' + e.what();
  }
  LOGERROR(originalMessage << '\n' << "Data was:\n" << obs);
}

void kv2kvDecoder::parse(KvalobsData & data, const std::string & obs) const {

  try {
    serialize::internal::KvalobsDataParser::parse(obs, data);
    producer_=data.producer();
    if( producer_.empty() ) {
      producer_ = producer; //From the DecoderBase
    }
  } catch (exception & e) {
    LOGERROR(e.what());
    milog::LogContext lcontext("Fallback");
    LOGDEBUG("Trying old parsing method");
    try {
      // Fallback on old way of decoding data.
      
      data = KvalobsData();  // remove any sideeffects from old parse attempt
      DList dlist = decodeutility::kvdataformatter::getKvData(obs);
      data.insert(dlist.begin(), dlist.end());
      LOGINFO("Data not in 'kvxml' format, but in the old CSV format");
      producer_ = producer;  //From the DecoderBase
    } catch (exception & e) {
      LOGERROR(e.what());
      throw DecoderError(decoder::DecoderBase::Rejected, "Cannot parse input.");
    }
  }
}

void kv2kvDecoder::verifyAndAdapt(KvalobsData & data, list<kvData> & out) {
  std::stringstream errStrm;
  int nErrors=0;
  invalidatePrevious(data);

  data.getData(out, tbtime);
  
  for (DList::iterator it = out.begin(); it != out.end(); ++it) {
    kv2kvDecoder::kvDataPtr dbData = getDbData(*it);
    if ( ! data.overwrite()) {
      try {
        verify(*it, dbData);
        adapt(*it, dbData, data.overwrite());
      } catch( const DecoderError &e) {
        string err(e.what());
        auto i = err.find_first_of(":");
  
        if ( i != string::npos ) {
          err = err.substr(i+2);
        }

        if ( nErrors == 0 ) {
          errStrm << "New data is not compatible with old in database:\n";
          errStrm << " - " <<  err;
        } else {
          errStrm << "\n - " << err;
        }
        
        nErrors++; 
      }
    }
  }

  if( nErrors > 0) {
    throw DecoderError(decoder::DecoderBase::Rejected, errStrm.str());
  }
}


void kv2kvDecoder::save2(const list<kvData> & dl_, const list<kvTextData> & tdl_) 
{
  // If checked is true
  //  - the data in the database is updated if it exist or added if it do not exist. 
  //    ie, do not replace data that already is in the database.
  //  - Do not add it to the workque. This will skip the run in kvQaBased.
  //  - Try to set tbTime from tbtime in the data. The oldest tbTime is used if the tbTimes is not equal.
  // This way we can almost 'replicate' the data from one kvalobs instance to another from the checked queue 
  // in kafka.
  
  // bool onlyUpdateData = checked_;
  //bool onlyUpdateData = true;
  DBAddType insertOrUpdate=DbUpdate;
  bool addDataToWorkQueue = ! checked_;
  bool tryToUseDataTbTime = checked_;

  KvDataContainer container(dl_, tdl_);
  KvDataContainer::DataByObstime data;
  KvDataContainer::TextDataByObstime textData;
  KvDataContainer::TextDataByObstime::iterator tid;
  
  KvDataContainer::StationInfoList infl=container.stationInfos();

  for( auto &sinf : container.stationInfos()) {
    IdlogHelper idLog(sinf.stationId, sinf.typeId, this);
    string logid( idLog.logid() );
    

    IDLOGINFO(logid, obs);
    if (container.get(
          data, textData, sinf.stationId, sinf.typeId,
          pt::second_clock::universal_time(), !tryToUseDataTbTime) < 0) {
       continue;               
    }

    for (KvDataContainer::DataByObstime::iterator it = data.begin();
      it != data.end(); ++it) {
      KvDataContainer::TextDataList td;
      tid = textData.find(it->first);

      if (tid != textData.end()) {
        td = tid->second;
        textData.erase(tid);
      }

      try {
        if (!addDataToDbThrow(to_miTime(it->first), sinf.stationId, sinf.typeId, it->second, td,
             logid, insertOrUpdate, addDataToWorkQueue, tryToUseDataTbTime, false)) {
          ostringstream ost;

          ost << "DBERROR: stationid: " << sinf.stationId << " typeid: " << sinf.typeId
              << " obstime: " << it->first;
          LOGERROR(ost.str());
          IDLOGERROR(logid, ost.str());
          throw DecoderError(decoder::DecoderBase::Error, ost.str());
        }
      }
      catch ( const dnmi::db::SQLException &e) {
        ostringstream ost;
        ost << "DBERROR: stationid: " << sinf.stationId << " typeid: " << sinf.typeId
            << " obstime: " << it->first << "\n" 
            << "DB " << e.what() << ". SQLSTATE: '" << e.errorCode() 
            << "' mayRecover: " << (e.mayRecover()?"true":"false") << ".";
          LOGERROR(ost.str());
          IDLOGERROR(logid, ost.str());
          throw DecoderError(decoder::DecoderBase::Error, ost.str());
      }
      catch( const std::exception &e ) {
        ostringstream ost;
        ost << "DBERROR: stationid: " << sinf.stationId << " typeid: " << sinf.typeId
            << " obstime: " << it->first << "\n" 
            << "DB " << e.what() << ".";
          LOGERROR(ost.str());
          IDLOGERROR(logid, ost.str());
          throw DecoderError(decoder::DecoderBase::Error, ost.str());
      }
      catch( ... ) {
        ostringstream ost;
        ost << "DBERROR: stationid: " << sinf.stationId << " typeid: " << sinf.typeId
            << " obstime: " << it->first << "\n" << "DB Unknown error.";;
          LOGERROR(ost.str());
          IDLOGERROR(logid, ost.str());
          throw DecoderError(decoder::DecoderBase::Error, ost.str());
      }
    }

    //Is there any left over text data.
    if (!textData.empty()) {
      KvDataContainer::DataList dl;
      for (KvDataContainer::TextDataByObstime::iterator it = textData.begin();
          it != textData.end(); ++it) {
        if (!addDataToDb(to_miTime(it->first), sinf.stationId, sinf.typeId, dl, it->second,
                       logid, insertOrUpdate, addDataToWorkQueue, tryToUseDataTbTime, false)) {
          ostringstream ost;
          ost << "DBERROR: TextData: stationid: " << sinf.stationId << " typeid: "
              << sinf.typeId << " obstime: " << it->first;
          LOGERROR(ost.str());
          IDLOGERROR(logid, ost.str());
          throw DecoderError(decoder::DecoderBase::Error, ost.str());
        }
      }
    }
  }
}


void kv2kvDecoder::markAsFixed(
    const serialize::KvalobsData::RejectList & rejectedMesage) {
  for (serialize::KvalobsData::RejectList::const_iterator it = rejectedMesage
      .begin(); it != rejectedMesage.end(); ++it) {
    std::stringstream query;
    query << "update rejectdecode set fixed='true'";
    //query << ", comment='FIXED: '||comment ";
    query << it->uniqueKey();
    try {
      getConnection()->exec(query.str());
    } catch (dnmi::db::SQLException & e) {
      throw DecoderError(decoder::DecoderBase::Error,
                         "Unable to mark rejected message as fixed");
    }
  }
}

namespace {
bool sensor_eq_(int sensor1, int sensor2) {
  int res = sensor1 - sensor2;
  return !res or abs(res) == '0';
}

struct lt_kvTextData  {
  bool operator()(const kvTextData & a, const kvTextData & b) const {
    if (a.stationID() != b.stationID())
      return a.stationID() < b.stationID();
    if (a.typeID() != b.typeID())
      return a.typeID() < b.typeID();
    if (a.obstime() != b.obstime())
      return a.obstime() < b.obstime();
    return a.paramID() < b.paramID();
  }
};

struct lt_kvTextData_without_paramID {
  bool operator()(const kvTextData & a, const kvTextData & b) const {
    if (a.stationID() != b.stationID())
      return a.stationID() < b.stationID();
    if (a.typeID() != b.typeID())
      return a.typeID() < b.typeID();
    return a.obstime() < b.obstime();
  }
};
}

void kv2kvDecoder::invalidatePrevious(KvalobsData & data) {
  list<KvalobsData::InvalidateSpec> inv;
  data.getInvalidate(inv);
  if (inv.empty())
    return;
  invalidatePreviousData(data, inv);
  invalidatePreviousTextData(data, inv);
}

void kv2kvDecoder::invalidatePreviousData(
    KvalobsData & data, const list<KvalobsData::InvalidateSpec> & inv) {
  list<kvData> tmp;
  data.getData(tmp, tbtime);
  typedef set<kvData, compare::lt_kvData> kvDataSet;
  kvDataSet sentInData(tmp.begin(), tmp.end());

  for (list<KvalobsData::InvalidateSpec>::const_iterator invIt = inv.begin();
      invIt != inv.end(); ++invIt) {
    DList alreadyInDb;  //sentInData;
    if (!dbGate.select(
        alreadyInDb,
        kvQueries::selectDataFromType(invIt->station, invIt->typeID,
                                      invIt->obstime)))
      throw DecoderError(decoder::DecoderBase::Error,
                         "Could not get data from database.");
    for (DList::const_iterator db = alreadyInDb.begin();
        db != alreadyInDb.end(); ++db) {
      if (compare::eq_sensor(0, db->sensor()) and db->level() == 0) {
        kvDataSet::const_iterator find = sentInData.find(*db);
        if (find == sentInData.end()) {
          if (data.overwrite()) {
            kvDataFactory f(db->stationID(), db->obstime(), db->typeID(),
                            db->sensor(), db->level());
            data.insert(f.getMissing(db->paramID()));
          } else {
            kvData d = *db;
            reject(d);
            data.insert(d);
          }
        }
      }
    }
  }
}

void kv2kvDecoder::invalidatePreviousTextData(
    KvalobsData & data, const list<KvalobsData::InvalidateSpec> & inv) {
  list<kvTextData> tmp;
  data.getData(tmp, tbtime);
  typedef set<kvTextData, lt_kvTextData> kvDataSet;
  kvDataSet sentInData(tmp.begin(), tmp.end());

  for (list<KvalobsData::InvalidateSpec>::const_iterator invIt = inv.begin();
      invIt != inv.end(); ++invIt) {
    TDList alreadyInDb;
    if (!dbGate.select(
        alreadyInDb,
        kvQueries::selectTextData(invIt->station, invIt->obstime,
                                  invIt->obstime)))
      throw DecoderError(decoder::DecoderBase::Error,
                         "Could not get data from database.");
    for (TDList::const_iterator db = alreadyInDb.begin();
        db != alreadyInDb.end(); ++db) {
      if (db->typeID() == invIt->typeID) {
        kvDataSet::const_iterator find = sentInData.find(*db);
        if (find == sentInData.end()) {
          if (data.overwrite()) {
            kvTextData td(db->stationID(), db->obstime(), "", db->paramID(),
                          tbtime, db->typeID());
            data.insert(td);
          } else {
            throw DecoderError(decoder::DecoderBase::Rejected,
                               "Illegal operation: Cannot overwrite text_data");
          }
        }
      }
    }
  }
}

kv2kvDecoder::kvDataPtr kv2kvDecoder::getDbData(const kvData d) {
  milog::LogContext lcontext("adapt");

  list<kvData> dbData;
  if (dbGate.select(dbData, kvQueries::selectData(d))) {
    // Lookup is done so that only a single instance will match
    if (dbData.size() > 1)
      throw DecoderError(decoder::DecoderBase::Error,
                         "Too many rows returned by query");
    if (dbData.empty()) {
      LOGDEBUG("No match for data " << d << " in database");
      return kv2kvDecoder::kvDataPtr();
    }
    LOGDEBUG("Data from database: " << dbData.front());
    return kv2kvDecoder::kvDataPtr(new kvData(dbData.front()));
  }
  throw DecoderError(
      decoder::DecoderBase::Error,
      "Problem with database lookup:\n" + kvQueries::selectData(d));
}

void kv2kvDecoder::verify(const kvData & d, kvDataPtr dbData) const {
  const float delta = 0.0999;
  if (dbData.get() && abs(dbData->original() - d.original()) > delta) {
    ostringstream ss;
    ss << "New data is not compatible with old in database: Original value: DB = "
       << dbData->original() << ". New = " << d.original() 
       << " (stationid: " << d.stationID() << " typeid: " << d.typeID() 
       << " paramid: " << d.paramID() << " obstime: " << pt::to_kvalobs_string(d.obstime()) << ")";
    throw DecoderError(decoder::DecoderBase::Rejected, ss.str());
  }
}

void kv2kvDecoder::adapt(kvData & d, kvDataPtr dbData, bool overwrite) const {
  milog::LogContext lcontext("adapt");
  
  if( dbData ) {
    LOGDEBUG("dbData.get():\t" << *dbData.get());  
  } else {
    LOGDEBUG("dbData.get():\tNo data");  
  }

  LOGDEBUG("overwrite:\t" << (overwrite?"true":"false"));

  if (dbData.get() && !overwrite)
    d.set(d.stationID(), d.obstime(), dbData->original(), d.paramID(),
          dbData->tbtime(), d.typeID(), d.sensor(), d.level(), d.corrected(),
          d.controlinfo(), d.useinfo(), d.cfailed());
}

}

}

}
