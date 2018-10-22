/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id:$

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
#include <sstream>
#include <stdexcept>
#include "lib/miutil/timeconvert.h"
#include "lib/kvalobs/observation.h"
#include "lib/kvdb/transactionhelper.h"

using namespace std;
using namespace dnmi;
namespace pt = boost::posix_time;


namespace {
  string dbTime(const pt::ptime &t, bool castToTimestamp=false) {
    if (!castToTimestamp)
      return "'"+pt::to_kvalobs_string(t)+"'";
    else
      return "'"+pt::to_kvalobs_string(t)+"'::timestamp";
  }

}



namespace kvalobs {
/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table data in the kvalobs database.
 */

Observation::Observation():observationid_(0), stationid_(0), typeid_(0) {
}

Observation::Observation(const Observation &d)
  :observationid_(d.observationid_), stationid_(d.stationid_), typeid_(d.typeid_),
   obstime_(d.obstime_), tbtime_(d.tbtime_), data_(d.data_), textData_(d.textData_) {
}
  
bool Observation::set(int stationId, int typeId, 
  const boost::posix_time::ptime & obt,
  const boost::posix_time::ptime & tbt, 
  const std::list<kvalobs::kvData> &data,
  const std::list<kvalobs::kvTextData> &textData) {
    hasObservationid_=false;
    observationid_=0;
    stationid_=stationId;
    typeid_=typeId;
    obstime_=obt;
    tbtime_=tbt;
    data_=data;
    textData_=textData;
}
 
bool Observation::set(const dnmi::db::Result &res ){
  hasObservationid_= false;
  stationid_=0;
  typeid_=0;
  obstime_=pt::ptime();
  tbtime_=pt::ptime();

  if ( !res.hasNext() )
    return false;
 
  db::DRow &r = const_cast<db::Result&>(res).next();
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();
  
  for (; it != names.end(); it++) {
    buf = r[*it];
    if (*it == "observationid" ) {
      hasObservationid_=true;
      observationid_ = atol(buf.c_str());
    } else if (*it == "stationid") {
      stationid_ = atoi(buf.c_str());
    } else if (*it == "typeid") {
      typeid_ = atoi(buf.c_str());
    } else if (*it == "obstime") {
      obstime_ = pt::time_from_string_nothrow(buf);
    } else if (*it == "tbtime") {
      tbtime_ = pt::time_from_string_nothrow(buf);
    } else {
      CERR("Observation::set .. unknown entry:" << *it << std::endl);
    }
  } 

  return hasObservationid_ && stationid_ != 0 && typeid_ != 0 && !obstime_.is_special() && !tbtime_.is_special();  
}

kvData Observation::getKvData(const dnmi::db::DRow &r_, long *obsid_) {
  db::DRow &r = const_cast<db::DRow&>(r_);
  string buf;
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();

  float original;
  int paramid;
  int sensor;
  int level;
  float corrected;
  kvControlInfo controlinfo;
  kvUseInfo useinfo;
  std::string cfailed;
  boost::posix_time::ptime obstime;
  long obsid=0;

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];

      if (*it == "observationid" ) {
        obsid = atol(buf.c_str());
      } else  if (*it == "original") {
        sscanf(buf.c_str(), "%f", &original);
      } else if (*it == "paramid") {
        paramid = atoi(buf.c_str());
      } else if (*it == "sensor") {
        sensor = atoi(buf.c_str());
      } else if (*it == "level") {
        level = atoi(buf.c_str());
      } else if (*it == "corrected") {
        sscanf(buf.c_str(), "%f", &corrected);
      } else if (*it == "controlinfo") {
        controlinfo = kvControlInfo(buf);
      } else if (*it == "useinfo") {
        useinfo = kvUseInfo(buf);
      } else if (*it == "cfailed") {
        cfailed = buf;
      } else if (*it == "obstime") {
        obstime_ = pt::time_from_string_nothrow(buf);
      } else {
        CERR("Observation::getKvData .. unknown entry:" << *it << std::endl);
      }
    } catch (...) {
      CERR("Observation::getKvData: unexpected exception ..... \n");
    }
  }

  if ( obsid != observationid_) {
    CERR("Observation::getKvData:  observationid differ this obsdata '" << obsid << "' observations '" << observationid_ << "'..... \n");
  }
  
  if(  obsid_ ) {
    *obsid_ = obsid;
  }

  if (obstime.is_special() ) {
      obstime = obstime_;
  }

  return kvData(stationid_, obstime, original, paramid,
         tbtime_, typeid_, sensor, level, corrected, controlinfo,  useinfo,cfailed);
  
}

kvTextData Observation::getKvTextData(const dnmi::db::DRow &r_, long *obsid_){
  dnmi::db::DRow & r = const_cast<dnmi::db::DRow&>(r_);
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();
  std::string buf;
  std::string original;
  int paramid;
  boost::posix_time::ptime obstime;
  long obsid;

  for (; it != names.end(); it++) {
    try {
      buf = r[*it];
       if (*it == "observationid" ) {
        obsid = atol(buf.c_str());
      } else if (*it == "obstime") {
        obstime = pt::time_from_string_nothrow(buf);
      } else if (*it == "original") {
        original = buf;
      } else if (*it == "paramid") {
        paramid = atoi(buf.c_str());
      } else {
        CERR("Observation::getKvTextData .. unknown entry:" << *it << std::endl);
      }
    } catch (...) {
      CERR("getKvTextData: exception ..... \n");
    }
  }

  if(  obsid_ ) {
    *obsid_ = obsid;
  }

  if (obstime.is_special() ) {
      obstime = obstime_;
  }

  return kvTextData(stationid_, obstime, original, paramid, tbtime_, typeid_);

}


void Observation::setData(const dnmi::db::Result &res){
  long obsid;
  while (res.hasNext()) {
    dnmi::db::DRow & row = const_cast<db::Result&>(res).next();
    data_.push_back(getKvData(row,&obsid));
    if (observationid_ != obsid) { 
      // Remove it again if it was not for this observationid (Should never happend)
      data_.pop_back(); 
    }
  }
}

void Observation::setTextData(const dnmi::db::Result &res){
  long obsid;
  while (res.hasNext()) {
    dnmi::db::DRow & row = const_cast<db::Result&>(res).next();
    textData_.push_back(getKvTextData(row,&obsid));
    if (observationid_ != obsid) { 
      // Remove it again if it was not for this observationid (Should never happend)
      data_.pop_back(); 
    }
  }

}
  
void Observation::cleanData() {
  textData_.clear();
  data_.clear();
}


Observation&  Observation::operator=(const Observation &rhs) {
  if ( &rhs != this ) {
    observationid_ = rhs.observationid_;
    stationid_ = rhs.stationid_;
    typeid_ = rhs.typeid_;
    obstime_ = rhs.obstime_;
    tbtime_ = rhs.tbtime_;
    data_= rhs.data_;
    textData_ = rhs.textData_;
  }
  return *this;
}

Observation&  Observation::operator=(const std::list<kvalobs::kvData> &rhs){
  data_= rhs;
  return *this;
}

Observation&  Observation::operator=(const std::list<kvalobs::kvTextData> &rhs){
  textData_ = rhs;
  return *this;
}

Observation *Observation::getFromDb(
  dnmi::db::Connection *con, 
  long stationID, 
  long typeID, 
  const boost::posix_time::ptime &obsTime,
  bool useTransaction
  )
{
  db::TransactionBlock tran(con, db::Connection::REPEATABLE_READ, false, !useTransaction);
  try {
    ostringstream q;
    q << "SELECT * FROM observations WHERE stationid=" << stationID << " AND typeid=" << typeID 
      << " AND obstime=" << dbTime(obsTime) ; 

    std::unique_ptr<dnmi::db::Result> res;
    res.reset(con->execQuery(q.str()));
 
    if (res->size() == 0 ) {
      return nullptr;
    }

    Observation *obs=new Observation();

    if (!obs->set(*res.get())) {
      q.str("");
      q << "UNEXPECTED EXCEPTION: Observation::getFromDb: observationid: " << obs->observationid() << " sid: " << obs->stationID() << " tid: "
        << obs->typeID() << " obst: " << pt::to_kvalobs_string(obs->obstime()) << " tbt: " << pt::to_kvalobs_string(obs->tbtime());
      throw std::logic_error(q.str());
    }

    obs->getDataForObservationid(con, obs->observationid());
  
    return obs;
  } 
  catch( ... ) {
    tran.abort();
    throw;
  }

}

Observation *Observation::getFromDb(dnmi::db::Connection *con, long observationid, bool useTransaction)
{
  db::TransactionBlock tran(con, db::Connection::REPEATABLE_READ, false, !useTransaction);
  try {
    ostringstream q;
    q << "SELECT * FROM observations WHERE observationid=" << observationid; 

    std::unique_ptr<dnmi::db::Result> res;
    res.reset(con->execQuery(q.str()));
 
    if (res->size() == 0 ) {
      return nullptr;
    }

    Observation *obs=new Observation();

    if (!obs->set(*res.get())) {
      q.str("");
      q << "UNEXPECTED EXCEPTION: Observation::getFromDb: observationid: " << obs->observationid() << " sid: " << obs->stationID() << " tid: "
        << obs->typeID() << " obst: " << pt::to_kvalobs_string(obs->obstime()) << " tbt: " << pt::to_kvalobs_string(obs->tbtime());
      throw std::logic_error(q.str());
    }

    obs->getDataForObservationid(con, obs->observationid());
  
    return obs;
  } 
  catch( ... ) {
    tran.abort();
    throw;
  }
}



void Observation::getDataForObservationid(dnmi::db::Connection *con, long obsid) {
  std::unique_ptr<dnmi::db::Result> res;
  ostringstream q;
  q << "SELECT * FROM obsdata WHERE observationid=" << obsid; 

  res.reset(con->execQuery(q.str()));
  setData(*res.get());

  q.clear();

  q << "SELECT * FROM obstextdata WHERE observationid=" << obsid; 

  res.reset(con->execQuery(q.str()));
  setTextData(*res.get());
}


void Observation::insertIntoDb(dnmi::db::Connection *con, bool useTransaction) 
{
  db::TransactionBlock tran(con, db::Connection::READ_COMMITTED, false, !useTransaction);
  try {
    ostringstream q;
    string tbtime;

    if ( tbtime_.is_special() ) {
      tbtime=dbTime(pt::second_clock::universal_time());
    } else {
      tbtime=dbTime(tbtime_);
    }

    q << "INSERT INTO observations (stationid, typeid, obstime,tbtime) VALUES(" 
      << stationid_ << "," << typeid_ << "," << dbTime(obstime_) << "," << tbtime << ")"; 
  
    con->exec(q.str());
  
    observationid_ = currentObservationid(con);

    q.clear();
    for(auto &d : data_) {
      q << "INSERT INTO obsdata (observationid,obs_offset,original,paramid,sensor,level,corrected,controlinfo,useinfo,cfailed) VALUES("
        << observationid_ << ","<< dbTime(d.obstime(),true) << "-" << dbTime(obstime_,true) <<"," << d.original() << ","
        << d.paramID() << "," << d.sensor() << "," <<d.level() << "," << d.corrected() <<"," 
        << d.controlinfo().flagstring() <<","<< d.useinfo().flagstring() << "," << "'" << d.cfailed() << "');\n";
    }

    for(auto &d : textData_) {
      q << "INSERT INTO obstextdata (observationid,obs_offset,original,paramid) VALUES("
        << observationid_ << ","<< dbTime(d.obstime(),true) << "-" << dbTime(obstime_,true) <<",'" << con->esc(d.original()) << "',"
        << d.paramID() <<");\n";
    }
    con->exec(q.str());
  } 
  catch( ... ) {
    tran.abort();
    throw;
  }
}


long Observation::currentObservationid(dnmi::db::Connection *con)
{
  std::unique_ptr<dnmi::db::Result> res;
  string q("SELECT currval('observations_observationid_seq')");

  res.reset(con->execQuery(q));
  if (res.get() != nullptr && res->hasNext()) {
    dnmi::db::DRow & row = res->next();
    return atol(row[0].c_str());
  }
  throw std::logic_error("Observation::currentObservationid: Unexpected no value");
}

std::ostream& operator<<(std::ostream& output,
                        const kvalobs::Observation &d) {
  return output;
}

}