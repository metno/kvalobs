/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: decoder.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $

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
#include <stdexcept>
#include <sstream>
#include "lib/kvdb/kvdb.h"
#include "lib/milog/milog.h"
#include "lib/kvalobs/kvWorkelement.h"
#include "lib/miutil/timeconvert.h"
#include "lib/decoder/decoderbase/DataUpdateTransaction.h"
#include "lib/kvalobs/observation.h"

namespace pt = boost::posix_time;

using dnmi::db::SQLException;
using std::string;
using std::endl;
using std::ostringstream;
using std::list;
using std::auto_ptr;
using std::logic_error;
using kvalobs::Observation;

namespace {

bool isKvDataEqual(const kvalobs::kvData &rhs, const kvalobs::kvData &lhs) {
  if (rhs.obstime() == lhs.obstime() && rhs.stationID() == lhs.stationID() && rhs.typeID() == lhs.typeID() && rhs.paramID() == lhs.paramID()
      && rhs.sensor() == lhs.sensor() && rhs.level() == lhs.level()) {
    float nv = lhs.original();
    float ov = rhs.original();

    if (static_cast<int>((nv + 0.005) * 100) == static_cast<int>((ov + 0.005) * 100))
      return true;
  }

  return false;
}

bool isKvTextDataEqual(const kvalobs::kvTextData &rhs, const kvalobs::kvTextData &lhs) {
  if (rhs.obstime() == lhs.obstime() && rhs.stationID() == lhs.stationID() && rhs.typeID() == lhs.typeID() && rhs.paramID() == lhs.paramID()
      && rhs.original() == lhs.original())
    return true;
  else
    return false;
}

std::list<kvalobs::kvData>::const_iterator findElem(const kvalobs::kvData &elem, const std::list<kvalobs::kvData> &list) {
  for (std::list<kvalobs::kvData>::const_iterator it = list.begin(); it != list.end(); ++it) {
    if (elem.obstime() == it->obstime() && elem.stationID() == it->stationID() && elem.typeID() == it->typeID() && elem.paramID() == it->paramID()
        && elem.sensor() == it->sensor() && elem.level() == it->level())
      return it;
  }

  return list.end();
}

std::list<kvalobs::kvTextData>::const_iterator findElem(const kvalobs::kvTextData &elem, const std::list<kvalobs::kvTextData> &list) {
  for (std::list<kvalobs::kvTextData>::const_iterator it = list.begin(); it != list.end(); ++it) {
    if (elem.obstime() == it->obstime() && elem.stationID() == it->stationID() && elem.typeID() == it->typeID() && elem.paramID() == it->paramID())
      return it;
  }

  return list.end();
}

}  // namespace

namespace kvalobs {

namespace decoder {

DataUpdateTransaction::DataUpdateTransaction(const boost::posix_time::ptime &obstime, int stationid, int typeID,
                                             std::list<kvalobs::kvData> *newData, std::list<kvalobs::kvTextData> *newTextData, const std::string &logid,
                                             bool onlyAddOrUpdateData_)
    : newData(newData),
      newTextData(newTextData),
      obstime(obstime),
      stationid(stationid),
      typeid_(typeID),
      data_(new kvalobs::serialize::KvalobsData()),
      ok_(new bool(false)),
      logid(logid),
      nRetry(0),
      onlyAddOrUpdateData(onlyAddOrUpdateData_) {
}

DataUpdateTransaction::DataUpdateTransaction(const DataUpdateTransaction &dut)
    : newData(dut.newData),
      newTextData(dut.newTextData),
      obstime(dut.obstime),
      stationid(dut.stationid),
      typeid_(dut.typeid_),
      data_(dut.data_),
      ok_(dut.ok_),
      logid(dut.logid),
      nRetry(dut.nRetry),
      onlyAddOrUpdateData(dut.onlyAddOrUpdateData) {
}

DataUpdateTransaction::~DataUpdateTransaction() {
}


void DataUpdateTransaction::updateWorkQue(dnmi::db::Connection *con, long observationid, int pri) {
  ostringstream q;

  q << "INSERT INTO workque (observationid,priority,process_start,qa_start,qa_stop,service_start,service_stop) "
    << "VALUES(" << observationid << "," << pri << ",NULL,NULL,NULL,NULL,NULL)";

  con->exec(q.str());
}


int DataUpdateTransaction::getPriority(dnmi::db::Connection *con, int stationid, int typeid_, const boost::posix_time::ptime &obstime)
{
  string buf;
  ostringstream q;

  q << "(SELECT *  FROM priority WHERE stationid=" << stationid << " AND typeid=abs(" << typeid_ << ") "
    << "UNION " 
    << "SELECT *  FROM priority WHERE stationid=0 AND typeid=abs(" << typeid_ << ")) ORDER BY stationid DESC LIMIT 1";
  
  std::unique_ptr<dnmi::db::Result> res;
  res.reset(con->execQuery(q.str()));

  if (res->size() == 0 ) {
    return 4;
  }
 
  while (res->hasNext()) {
    dnmi::db::DRow & row = res->next();
    int priority = INT_MAX;
    int priAfter = 10;
    int hour=6;
    list<string> names = row.getFieldNames();
    list<string>::iterator it = names.begin();
  
    for (; it != names.end(); it++) {
      try {
        buf = row[*it];

        if (*it == "priority" ) {
          priority = atoi(buf.c_str());
        } else  if (*it == "hour") {
          hour = atoi(buf.c_str());
        } else if (*it == "pri_after_hour") {
          priAfter = atoi(buf.c_str());
        } else if (*it == "stationid") {
          continue;
        } else if (*it == "typeid") {
          continue;
        } else {
          CERR("DataUpdateTransaction::getPriority .. unknown entry:" << *it << std::endl);
        }
      } catch (...) {
        CERR("DataUpdateTransaction::getPriority: unexpected exception ..... \n");
      }
    }

    if( priority == INT_MAX ) {
      CERR("DataUpdateTransaction::getPriority: no priority def, returning default 4 \n");
      return 4;
    }

    auto now = pt::second_clock::universal_time();
    auto testTime = now-pt::hours(hour);

    if( obstime<testTime)
      return priAfter;
    else
      return priority;
  }

  return 4;
}



bool DataUpdateTransaction::doIsEqual(const std::list<kvalobs::kvData> &oldData, const std::list<kvalobs::kvTextData> &oldTextData, bool replace) {
  bool found=false;

  if( replace ) {
    if ( newData->size() != oldData.size() || newTextData->size() != oldTextData.size()) {
      log << "isEqual: size differ: " << oldData.size() << " (" << newData->size() << ") " << "- " << oldTextData.size() << " (" << newTextData->size()
          << ")\n";

      return false;
    }
  }

  for (list<kvalobs::kvData>::const_iterator nit = newData->begin(); nit != newData->end(); ++nit) {
    found = false;

    for (list<kvalobs::kvData>::const_iterator oit = oldData.begin(); oit != oldData.end(); ++oit) {
      if (oit->obstime() == nit->obstime() && oit->stationID() == nit->stationID() && oit->typeID() == nit->typeID() && oit->paramID() == nit->paramID()
          && oit->sensor() == nit->sensor() && oit->level() == nit->level()) {
        float nv = nit->original();
        float ov = oit->original();

        if (static_cast<int>((nv + 0.005) * 100) == static_cast<int>((ov + 0.005) * 100)) {
          found = true;
          break;
        }
      }
    }

    if (!found)
      return false;
  }

  for (list<kvalobs::kvTextData>::const_iterator nit = newTextData->begin(); nit != newTextData->end(); ++nit) {
    found = false;
    for (list<kvalobs::kvTextData>::const_iterator oit = oldTextData.begin(); oit != oldTextData.end(); ++oit) {
      if (oit->obstime() == nit->obstime() && oit->stationID() == nit->stationID() && oit->typeID() == nit->typeID() && oit->paramID() == nit->paramID()
          && oit->original() == nit->original()) {
        found = true;
        break;
      }
    }

    if (!found)
      return false;
  }

  return found;
}


bool DataUpdateTransaction::isEqual(const std::list<kvalobs::kvData> &oldData_, const std::list<kvalobs::kvTextData> &oldTextData) {
  //if onlyAddOrUpdateData is false, the oldadata is to be replaced by the new data.
  //If true the new data is to be addded to the data that is alleady in the databse.
  if( doIsEqual(oldData_, oldTextData, !onlyAddOrUpdateData) ) {
    return true;
  }

  //Remove missing values from the old data and test again for equality  
  list<kvalobs::kvData> oldData(oldData_);

  for (list<kvalobs::kvData>::iterator it = oldData.begin(); it != oldData.end(); ++it) {
    if (it->original() == -32767)
      it = oldData.erase(it);
  }

  return doIsEqual(oldData, oldTextData, !onlyAddOrUpdateData);
}


bool DataUpdateTransaction::updateObservation(dnmi::db::Connection *conection, Observation *obs) {
  list<kvalobs::kvData> toUpdateData(*newData);
  list<kvalobs::kvTextData> toUpdateTextData(*newTextData);

  for( auto &e : obs->data()){
    auto it = findElem(e, toUpdateData);
    if (it == toUpdateData.end()) {
      toUpdateData.push_back(e);
    }
  }
  
  for( auto &e : obs->textData()){
    auto it = findElem(e, toUpdateTextData);
    if (it == toUpdateTextData.end()) {
      toUpdateTextData.push_back(e);
    }
  }
  
  if( toUpdateData.empty() && toUpdateTextData.empty())
    return true;

  ostringstream q;

  q << "DELETE FROM observations WHERE observationid=" << obs->observationid();

  conection->exec(q.str());

  Observation newObs(obs->stationID(), obs->typeID(), obs->obstime(), pt::second_clock::universal_time(), toUpdateData, toUpdateTextData);
  newObs.insertIntoDb(conection, false);
  int pri = getPriority(conection, stationid, typeid_, obstime);
  updateWorkQue(conection, newObs.observationid(), pri);
  return true;
} 

bool DataUpdateTransaction::replaceObservation(dnmi::db::Connection *conection, long observationid)
{
  ostringstream q;
  q << "DELETE FROM observations WHERE observationid=" << observationid;

  conection->exec(q.str());

  Observation newObs(stationid, typeid_, obstime, pt::second_clock::universal_time(), *newData, *newTextData);
  newObs.insertIntoDb(conection, false);
  int pri = getPriority(conection, stationid, typeid_, obstime);
  updateWorkQue(conection, newObs.observationid(), pri);
  return true;
}


bool DataUpdateTransaction::operator()(dnmi::db::Connection *conection) {
  ostringstream mylog;
  boost::posix_time::ptime tbtime;

  if (obstime.is_not_a_date_time()) {
    LOGERROR("NewData: stationid: " << stationid << " typeid: " << typeid_ << ". Invalid obstime.");
    return false;
  }

  if (newData->empty() && newTextData->empty() ) {
    insertType="NO DATA";
    return true;
  }


  if (!logid.empty()) {
    bool err = false;
    for (std::list<kvalobs::kvData>::const_iterator it = newData->begin(); it != newData->end(); ++it) {
      if (it->obstime().is_not_a_date_time()) {
        err = true;
        mylog << "Invalid obstime: " << it->stationID() << "," << it->typeID() << "," << it->paramID() << "," << it->sensor() << "," << it->level() << ","
              << it->original() << endl;
      } else {
        mylog << pt::to_kvalobs_string(it->obstime()) << "," << it->stationID() << "," << it->typeID() << "," << it->paramID() << "," << it->sensor() << ","
              << it->level() << "," << it->original() << endl;
      }
    }

    if (err) {
      LOGERROR("NewData: INVALID OBSTIME stationid: " << stationid << " typeid: " << typeid_ << endl << mylog.str());
      return false;
    }

    log << "NewData " << (onlyAddOrUpdateData ? "(replenish):" : ":") << "stationid: " << stationid << " typeid: " << typeid_ << " obstime: "
        << pt::to_kvalobs_string(obstime) << endl << mylog.str() << endl;
  }

  std::unique_ptr<Observation> oldObs(Observation::getFromDb(conection, stationid, typeid_, obstime, false));
  
  if (!oldObs) { //No observation exist
    insertType="INSERT";
    Observation newObs(stationid, typeid_, obstime, pt::second_clock::universal_time(), *newData, *newTextData);
    newObs.insertIntoDb(conection, false);
    int pri = getPriority(conection, stationid, typeid_, obstime);
    updateWorkQue(conection, newObs.observationid(), pri);
    return true;
  }

  if (isEqual(oldObs->data(), oldObs->textData())) {
    log << "Data allready exist. stationid: " << stationid << " typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime) << endl;
    IDLOGINFO("duplicates", "DUPLICATE: stationid: " << stationid << " typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime));
    insertType = "DUPLICATE";
    return true;
  }

  if (onlyAddOrUpdateData) {
    insertType = "REPLENISH";
    return updateObservation(conection, oldObs.get());
  }

  insertType = "REPLACE";
  return replaceObservation(conection, oldObs->observationid());
}

void DataUpdateTransaction::onSuccess() {
  ostringstream mylog;
  string prefix(insertType.length(), ' ');
  mylog << insertType << ": stationid: " << stationid << " typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime);
  IDLOGINFO(logid, log.str());
  IDLOGINFO("transaction", mylog.str());
  *ok_ = true;
}

void DataUpdateTransaction::onFailure() {
  ostringstream mylog;
  string prefix(insertType.length(), ' ');
  mylog << insertType << ": Failed: stationid: " << stationid << " typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime);
  IDLOGERROR(logid, log.str());
  IDLOGERROR("transaction", mylog.str());
}

void DataUpdateTransaction::onRetry() {
  if (!logid.empty()) {
    IDLOGDEBUG(logid, "Retry transaction.\n" << log.str());
  }

  nRetry++;
  IDLOGDEBUG(
      "retry",
      "RETRY: " << nRetry << " stationid: " << stationid << " Typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime) << "\nMessage: " << log.str());
  log.str("");
  data_->clear();
}

void DataUpdateTransaction::onAbort(const std::string &driverid, const std::string &errorMessage, const std::string &errorCode) {
  if (!logid.empty()) {
    IDLOGINFO(logid, "Transaction aborted: Driver: '" << driverid << "' ErrorCode: '" << errorCode << "'.\nReason: " << errorMessage);
  }
}

void DataUpdateTransaction::onMaxRetry(const std::string &lastError, const std::string &errorCode, bool mayRecover) {
  ostringstream mylog;

  if (!log.str().empty()) {
    mylog << endl << "Log: " << log.str();
  }

  IDLOGERROR(logid, "Transaction Failed (mayRecover=" << (mayRecover?"true":"false") << " errorCode=" << errorCode <<").\n" << lastError << "\n" << log.str());
  IDLOGERROR(
      "failed",
      "Transaction Failed (mayRecover=" << (mayRecover?"true":"false") << " errorCode=" << errorCode <<").\n" << " Stationid: " << stationid << " Typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime) << "\nLast error: " << lastError << mylog.str());
  IDLOGERROR("transaction", "   FAILED: Stationid: " << stationid << " Typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime));
  throw SQLException(lastError, errorCode, mayRecover);
}

}  // namespace decoder
}  // namespace kvalobs
