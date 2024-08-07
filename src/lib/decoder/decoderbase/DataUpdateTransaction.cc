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
#include "lib/kvalobs/kvDbBase.h"
#include "lib/milog/milog.h"
#include "lib/kvalobs/kvWorkelement.h"
#include "lib/miutil/timeconvert.h"
#include "lib/miutil/splitstr.h"
#include "lib/decoder/decoderbase/DataUpdateTransaction.h"
#include "lib/kvalobs/observation.h"

namespace pt = boost::posix_time;

using dnmi::db::SQLException;
using std::string;
using std::endl;
using std::cerr;
using std::ostringstream;
using std::list;
using std::auto_ptr;
using std::logic_error;
using kvalobs::Observation;
using miutil::splitstr;
using kvalobs::kvDbBase;

#define Q(t) kvDbBase::quoted(t)

namespace {
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
                                             DBAddType insertOrUpdate_, bool addToWorkQueue_, bool tryToUseDataTbTime_,
                                             DataUpdateTransaction::DuplicateTestType duplicateTestType_, int qaId_)
    : newData(newData),
      newTextData(newTextData),
      obstime(obstime),
      stationid(stationid),
      typeid_(typeID),
      observationid(0),
      startTime(pt::microsec_clock::universal_time()),
      data_(new kvalobs::serialize::KvalobsData()),
      dataToPublish_(new kvalobs::serialize::KvalobsData()),
      ok_(new bool(false)),
      logid(logid),
      nRetry(0),
      insertOrUpdate(insertOrUpdate_),
      addToWorkQueue(addToWorkQueue_),
      tryToUseDataTbTime(tryToUseDataTbTime_),
      duplicateTestType(duplicateTestType_),
      onlyHqcData(false),
      qaId(qaId_) {
}

DataUpdateTransaction::DataUpdateTransaction(const DataUpdateTransaction &dut)
    : newData(dut.newData),
      newTextData(dut.newTextData),
      obstime(dut.obstime),
      stationid(dut.stationid),
      typeid_(dut.typeid_),
      observationid(dut.observationid),
      startTime(dut.startTime),
      data_(dut.data_),
      dataToPublish_(dut.dataToPublish_),
      ok_(dut.ok_),
      logid(dut.logid),
      nRetry(dut.nRetry),
      insertOrUpdate(dut.insertOrUpdate),
      addToWorkQueue(dut.addToWorkQueue),
      tryToUseDataTbTime(dut.tryToUseDataTbTime),
      duplicateTestType(dut.duplicateTestType),
      onlyHqcData(dut.onlyHqcData),
      qaId(dut.qaId) {
}

DataUpdateTransaction::~DataUpdateTransaction() {
}


void DataUpdateTransaction::onlyHqcDataCheck() 
{
  dataToPublish_->clear();
  onlyHqcData=false;

  for (auto it : *newData) {
    if( ! it.controlinfo().hqcDone() ) {
      return;
    }
  }
  onlyHqcData=true;

  //We can now bypass kvQabase with NOT adding this data to the workqueue.
  //But the data must be published on the 'checked' kafka queue. 

  dataToPublish_->insert(newData->begin(), newData->end());
  dataToPublish_->insert(newTextData->begin(), newTextData->end());
}



void DataUpdateTransaction::checkWorkQue(dnmi::db::Connection *con, long observationid)
{
  //Check if we have an unprocessed worque element for this observationid. 
  //It is unprocessed if qa_stop is null. In this case we must ensure that 
  //a new workque element is generated if the data is deleted because of cascading deletes
  //the workque element to.

  ostringstream q;

  q << "SELECT observationid FROM workque WHERE observationid="<< observationid << " AND "
    << "qa_stop IS NULL";
 
  std::unique_ptr<dnmi::db::Result> res;
  res.reset(con->execQuery(q.str()));

  if (res->size() > 0 ) {
    dataToPublish_->clear();
    onlyHqcData = false;  //This ensures that we create a new workque element.
    return;
  }

  //We must ensure that an element in the workque that is processed in kvQabase is added
  //to workstatistik.
  worqueToWorkStatistik(con, observationid);
}

void DataUpdateTransaction::updateWorkQue(dnmi::db::Connection *con, long observationid_, int pri) {
  if ( ! addToWorkQueue || onlyHqcData) {
    observationid=-observationid_;
    duration = pt::microsec_clock::universal_time() - startTime;
    return;
  }

  observationid=observationid_;
  ostringstream q;
  ostringstream sQaId;

  if( qaId < 0 ) {
    sQaId << "NULL";
  } else {
    sQaId << qaId;
  }

  q << "INSERT INTO workque (observationid,priority,process_start,qa_start,qa_stop,service_start,service_stop, qa_id) "
    << "VALUES(" << observationid << "," << pri << ",NULL,NULL,NULL,NULL,NULL," << sQaId.str() << ")";

  con->exec(q.str());
  duration = pt::microsec_clock::universal_time() - startTime;
}



void DataUpdateTransaction::worqueToWorkStatistik(dnmi::db::Connection *con, long observationid) 
{
  //We must ensure that an element in the workque that is processed in kvQabase is added
  //to workstatistik.

  ostringstream q;

  q.str("");
  q << "INSERT INTO workstatistik SELECT "
    << "o.stationid,"
    << "o.obstime," 
    << "o.typeid,"
    << "o.tbtime,"
    << "q.priority,"
    << "q.process_start,"
    << "q.qa_start,"
    << "q.qa_stop,"
    << "q.service_start,"
    << "q.service_stop,"
    << "q.observationid,"
    << "q.qa_id "
    << "FROM workque q, observations o "
    << "WHERE q.observationid=o.observationid AND q.observationid=" << observationid 
    << " AND q.qa_stop IS NOT NULL AND (SELECT count(*) FROM workstatistik s WHERE q.observationid=s.observationid)=0";

  con->exec(q.str());

  q.str("");
  q << "DELETE FROM workque WHERE observationid=" << observationid << " AND qa_stop IS NOT NULL";
  con->exec(q.str());
}


boost::posix_time::ptime 
DataUpdateTransaction
::useTbTime(const std::list<kvalobs::kvData> &data, const std::list<kvalobs::kvTextData> &textData,
            const boost::posix_time::ptime &defaultTbTime)const 
{
  if ( ! tryToUseDataTbTime ) {
    return defaultTbTime;
  }

  boost::posix_time::ptime tbTime;
  
  for( auto &it : data ) {
    if( tbTime.is_special() ) {
      tbTime = it.tbtime();
      continue;
    }
    if( ! it.tbtime().is_special() && it.tbtime() < tbTime ) {
      tbTime = it.tbtime();
    }
  }

  for( auto &it : textData ) {
    if( tbTime.is_special() ) {
     tbTime = it.tbtime();
      continue;
    }
    if( ! it.tbtime().is_special() && it.tbtime() < tbTime ) {
      tbTime = it.tbtime();
    }
  }

  if ( ! tbTime.is_special() ) {
    return tbTime;
  }

  return pt::microsec_clock::universal_time();
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



bool 
DataUpdateTransaction::
partialIsEqual(const std::list<kvalobs::kvData> &oldData, const std::list<kvalobs::kvTextData> &oldTextData, bool replace)const 
{
  bool found=false;

  if( replace ) {
    if ( newData->size() != oldData.size() || newTextData->size() != oldTextData.size()) {
      log << "isEqual: size differ: data " << oldData.size() << " (" << newData->size() << ") " << "- textData " << oldTextData.size() << " (" << newTextData->size()
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


bool 
DataUpdateTransaction::
completeIsEqual(const std::list<kvalobs::kvData> &oldData, const std::list<kvalobs::kvTextData> &oldTextData, bool replace) const
{
  bool found=false;

  if( replace ) {
    if ( newData->size() != oldData.size() || newTextData->size() != oldTextData.size()) {
      log << "doCompleteIsEqual: size differ: data " << oldData.size() << " (" << newData->size() << ") " << "- textData " << oldTextData.size() << " (" << newTextData->size()
          << ")\n";

      return false;
    }
  }

  for (list<kvalobs::kvData>::const_iterator nit = newData->begin(); nit != newData->end(); ++nit) {
    found = false;

    for (list<kvalobs::kvData>::const_iterator oit = oldData.begin(); oit != oldData.end(); ++oit) {
      if (oit->obstime() == nit->obstime() && oit->stationID() == nit->stationID() && oit->typeID() == nit->typeID() && oit->paramID() == nit->paramID()
          && oit->sensor() == nit->sensor() && oit->level() == nit->level()) {
        float norig = nit->original();
        float oorig = oit->original();
        float ncor = nit->corrected();
        float ocor = oit->corrected();


        if ((static_cast<int>((norig + 0.005) * 100) == static_cast<int>((oorig + 0.005) * 100)) &&
            (static_cast<int>((ncor + 0.005) * 100) == static_cast<int>((ocor + 0.005) * 100)) &&
            (nit->controlinfo() == oit->controlinfo()) &&
            (nit->useinfo() == oit->useinfo()) &&
            (nit->cfailed() == oit->cfailed())
         ) {
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
  if( duplicateTestType == Complete ){
    return completeIsEqual(oldData_, oldTextData, insertOrUpdate==DbInsert);
  }

  //if onlyAddOrUpdateData is false, the oldadata is to be replaced by the new data.
  //If true the new data is to be addded to the data that is alleady in the databse.
  if( partialIsEqual(oldData_, oldTextData, insertOrUpdate==DbInsert) ) {
    return true;
  }

  //Remove missing values from the old data and test again for equality  
  list<kvalobs::kvData> oldData(oldData_);

  for (list<kvalobs::kvData>::iterator it = oldData.begin(); it != oldData.end(); ++it) {
    if (it->original() == -32767)
      it = oldData.erase(it);
  }

  return partialIsEqual(oldData, oldTextData, insertOrUpdate==DbInsert);
}


bool DataUpdateTransaction::updateObservation(dnmi::db::Connection *conection, Observation *obs) {
  list<kvalobs::kvData> toUpdateData(*newData);
  list<kvalobs::kvTextData> toUpdateTextData(*newTextData);

  //TODO: Check for only missing data
  bool noOldData=obs->totSize()==0;
  
#if 0
  cerr <<  "OldObs: stationid: #data: " << obs->dataSize() << " #textdata: " << obs->textDataSize() << " noOldData: "<< (noOldData?"true":"false") 
       << " observationid: " << obs->observationid() << "  "<<obs->stationID() <<"/" << obs->typeID() << "/" << obs->obstime() << endl;
  std::cerr << "updateObservation: incomming \n";
  for ( auto &d : toUpdateData )
    std::cerr << "updateObservation (d): new: " << d.obstime() << ", " << d.stationID() << ", " << d.typeID() << ", " << d.paramID() <<  d.original() << "\n";


  for ( auto &d : toUpdateTextData )
    std::cerr << "updateObservation (td): new: " << d.obstime() << ", " << d.stationID() << ", " << d.typeID() << ", " << d.paramID() <<  d.original() << "\n";

  for ( auto &d : obs->data() )
    std::cerr << "updateObservation (d): old: " << d.obstime() << ", " << d.stationID() << ", " << d.typeID() << ", " << d.paramID() <<  d.original() << "\n";


  for ( auto &d : obs->textData() )
    std::cerr << "updateObservation (td): old: " << d.obstime() << ", " << d.stationID() << ", " << d.typeID() << ", " << d.paramID() <<  d.original() << "\n";
  
  std::cerr << "end incomming\n";
#endif

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

  checkWorkQue(conection, obs->observationid());
  ostringstream q;

  q << "DELETE FROM observations WHERE observationid=" << obs->observationid();

  conection->exec(q.str());
  pt::ptime tbTime = useTbTime(toUpdateData, toUpdateTextData, obs->tbtime());

#if 0
  //BEGIN: debug
  if( tbTime.is_not_a_date_time() ) {
      IDLOGERROR(logid, "updateObservation: tbTime is invalid.");
      if( obs->tbtime().is_not_a_date_time() ) {
        IDLOGERROR(logid, "updateObservation: obs->tbTime is invalid.");
      }
      tbTime=pt::microsec_clock::universal_time();
      if ( tbTime.is_not_a_date_time() ) {
          IDLOGERROR(logid, "updateObservation: tbTime still is invalid.");
      }
  }

  if( obstime.is_not_a_date_time() ) {
      IDLOGERROR(logid, "updateObservation: obstime is invalid.");
  }

  if( obs->obstime().is_not_a_date_time() ) {
      IDLOGERROR(logid, "updateObservation: obs->obstime is invalid.");
  }
  //END: debug

  if ( noOldData ) {
    tbTime=pt::microsec_clock::universal_time();
  }
#endif

  Observation newObs(obs->stationID(), obs->typeID(), obs->obstime(), tbTime, toUpdateData, toUpdateTextData);
  newObs.insertIntoDb(conection, false);
  int pri = getPriority(conection, stationid, typeid_, obstime);
  updateWorkQue(conection, newObs.observationid(), pri);
  return true;
} 

bool DataUpdateTransaction::replaceObservation(dnmi::db::Connection *conection, long observationid)
{
  checkWorkQue(conection, observationid);

  ostringstream q;
  q << "DELETE FROM observations WHERE observationid=" << observationid;

  conection->exec(q.str());
  
  pt::ptime tbTime = useTbTime( *newData, *newTextData, pt::microsec_clock::universal_time());

  Observation newObs(stationid, typeid_, obstime, tbTime, *newData, *newTextData);
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

  onlyHqcDataCheck();

  if (!logid.empty()) {
    bool err = false;
    for (std::list<kvalobs::kvData>::const_iterator it = newData->begin(); it != newData->end(); ++it) {
      if (it->obstime().is_not_a_date_time()) {
        err = true;
        mylog << "Invalid obstime: " << it->stationID() << "," << it->typeID() << "," << it->paramID() << "," << it->sensor() << "," << it->level() << ","
              << it->original() << endl;
      } else {
        mylog << pt::to_kvalobs_string(it->obstime()) << "," << it->stationID() << "," << it->typeID() << "," << it->paramID() << "," << it->sensor() << ","
              << it->level() << "," << it->original() << ", " << pt::to_kvalobs_string(it->tbtime()) << endl;
      }
    }

    for (std::list<kvalobs::kvTextData>::const_iterator it = newTextData->begin(); it != newTextData->end(); ++it) {
      if (it->obstime().is_not_a_date_time()) {
        err = true;
        mylog << "Invalid obstime: " << it->stationID() << "," << it->typeID() << "," << it->paramID() << "," << it->original() << endl;
      } else {
        mylog << pt::to_kvalobs_string(it->obstime()) << "," << it->stationID() << "," << it->typeID() << "," << it->paramID() << "," << it->original() 
              << ", '" << pt::to_kvalobs_string(it->tbtime()) << "'" << endl;
      }
    }


    if (err) {
      LOGERROR("NewData: INVALID OBSTIME stationid: " << stationid << " typeid: " << typeid_ << endl << mylog.str());
      return false;
    }

    log << "NewData " << (insertOrUpdate==DbUpdate ? "(update):" : ":") << "stationid: " << stationid << " typeid: " << typeid_ << " obstime: "
        << pt::to_kvalobs_string(obstime) << " onlyHqcData: "<<(onlyHqcData?"true":"false") << " qa_id: " << qaId  << endl << mylog.str() << endl;
  }

  std::unique_ptr<Observation> oldObs(Observation::getFromDb(conection, stationid, typeid_, obstime, false));
  
  if (!oldObs) { //No observation exist
    insertType="INSERT";
    pt::ptime tbTime = useTbTime( *newData, *newTextData, pt::microsec_clock::universal_time());
    Observation newObs(stationid, typeid_, obstime, tbTime, *newData, *newTextData);
    newObs.insertIntoDb(conection, false);
    int pri = getPriority(conection, stationid, typeid_, obstime);
    updateWorkQue(conection, newObs.observationid(), pri);
    return true;
  }

  if (isEqual(oldObs->data(), oldObs->textData())) {
    log << "Data allready exist. stationid: " << stationid << " typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime) << endl;
    IDLOGINFO("duplicates", "DUPLICATE: stationid: " << stationid << " typeid: " << typeid_ << " obstime: " << pt::to_kvalobs_string(obstime));
    dataToPublish_->clear();    
    insertType = "DUPLICATE";
    duration = pt::microsec_clock::universal_time() - startTime;
    observationid=oldObs->observationid();
    return true;
  }

  if (insertOrUpdate==DbUpdate) {
    insertType = "UPDATE";
    return updateObservation(conection, oldObs.get());
  }

  insertType = "REPLACE";
  return replaceObservation(conection, oldObs->observationid());
}

std::string DataUpdateTransaction::transactionLogString() {
  ostringstream s;
  ostringstream sQaId;

  if( qaId>-1 ) {
    sQaId << " qa_id=" << qaId;
  }

  s << "(" << observationid << ": " << stationid << "/" << typeid_<< "/" <<pt::to_kvalobs_string(obstime) << ") duration=" << duration.total_milliseconds() << "ms" << sQaId.str();
  return s.str();
}

void DataUpdateTransaction::onSuccess() {
  ostringstream mylog;
  string prefix(insertType.length(), ' ');
  mylog << insertType << ": " << transactionLogString();
  IDLOGINFO(logid, log.str());
  IDLOGINFO("transaction", mylog.str());
  *ok_ = true;
}

void DataUpdateTransaction::onFailure() {
  ostringstream mylog;
  string prefix(insertType.length(), ' ');
  mylog << insertType << ": Failed: stationid: " << transactionLogString();
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
  IDLOGERROR("transaction", "   FAILED: " << transactionLogString() );
  throw SQLException(lastError, errorCode, mayRecover);
}

}  // namespace decoder
}  // namespace kvalobs
