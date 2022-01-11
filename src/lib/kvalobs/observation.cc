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
#include "lib/milog/milog.h"
#include "lib/miutil/timeconvert.h"
#include "lib/kvalobs/observation.h"
#include "lib/kvdb/transactionhelper.h"

using namespace std;
using namespace dnmi;
using std::get;
namespace pt = boost::posix_time;


namespace {
  string dbTime(const pt::ptime &t, bool castToTimestamp=false) {
    if (!castToTimestamp)
      return "'"+pt::to_kvalobs_string(t)+"'";
    else
      return "'"+pt::to_kvalobs_string(t)+"'::timestamp";
  }

  string Q(const std::string &s) {
    if ( s.length() >= 2) {
      if ( s[0]=='\'' && s[s.length()-1]=='\'')
        return s;
    }

    return '\''+s+'\'';
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

Observation::Observation():hasObservationid_(false),observationid_(0), stationid_(0), typeid_(0) {
}

Observation::Observation(const Observation &d)
  :hasObservationid_(d.hasObservationid_), observationid_(d.observationid_), stationid_(d.stationid_), typeid_(d.typeid_),
   obstime_(d.obstime_), tbtime_(d.tbtime_), data_(d.data_), textData_(d.textData_) {
}

 void Observation::setObservationid(long observationid){
   observationid_=observationid;
   hasObservationid_=true;
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
    return true;
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

  long obsid;
  int stationid;
  int myTypeid;
  pt::ptime obstime;
  pt::ptime tbtime;
  float original;
  int paramid;
  int sensor;
  int level;
  float corrected;
  kvControlInfo controlinfo;
  kvUseInfo useinfo;
  std::string cfailed;
  
  *obsid_ = 0;
  for (; it != names.end(); it++) {
    try {
      buf = r[*it];

      if (*it == "observationid" ) {
        obsid = atol(buf.c_str());
        if( ! hasObservationid_ ){
          observationid_=obsid;
          hasObservationid_=true;
        }

        if( *obsid_ == 0 ) {
          *obsid_ = obsid;
        } else if( obsid != *obsid_ ) {
          CERR("Observation::getKvData .. EXPECTING all observations has the same observationid\n");
        }
      } else if ( *it == "stationid" ) {
        stationid = atoi(buf.c_str());
        if( stationid_ == 0){
          stationid_=stationid;
        } 
      } else if (*it == "typeid") {
        myTypeid = atoi(buf.c_str());
        if( typeid_ == 0 ) {
          typeid_ = myTypeid;
        }
      }else if (*it == "obstime") {
        obstime = pt::time_from_string_nothrow(buf);
        if( obstime_.is_special() ) {
          obstime_ = obstime;
        }
      }else if (*it == "tbtime") {
        tbtime = pt::time_from_string_nothrow(buf);
        if( tbtime_.is_special() ) {
          tbtime_ = tbtime;
        }
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
      } else {
        CERR("Observation::getKvData .. unknown entry:" << *it << std::endl);
      }
    } catch (...) {
      CERR("Observation::getKvData: unexpected exception ..... \n");
    }
  }

/*
  if(  obsid_ ) {
    *obsid_ = obsid;
  }

*/
  
  return kvData(stationid, obstime, original, paramid,
         tbtime, myTypeid, sensor, level, corrected, controlinfo, useinfo, cfailed);
  
}

kvTextData Observation::getKvTextData(const dnmi::db::DRow &r_, long *obsid_){
  dnmi::db::DRow & r = const_cast<dnmi::db::DRow&>(r_);
  list<string> names = r.getFieldNames();
  list<string>::iterator it = names.begin();
  std::string buf;
  long obsid;
  int stationid;
  int myTypeid;
  pt::ptime obstime;
  pt::ptime tbtime;
  std::string original;
  int paramid;
  
  *obsid_ = 0;
  for (; it != names.end(); it++) {
    try {
       buf = r[*it];
       if (*it == "observationid" ) {
        obsid = atol(buf.c_str());
        if( ! hasObservationid_ ){
          observationid_=obsid;
          hasObservationid_=true;
        }

        if (*obsid_ == 0) {
          *obsid_ = obsid;
        } else if ( *obsid_ != obsid ) {
            CERR("Observation::getKvTextData .. EXPECTING all observations has the same observationid\n");
        }
      } else if ( *it == "stationid" ) {
        stationid = atoi(buf.c_str());
        if( stationid_ == 0){
          stationid_=stationid;
        } 
      } else if (*it == "typeid") {
        myTypeid = atoi(buf.c_str());
        if( typeid_ == 0 ) {
          typeid_ = myTypeid;
        }
      }else if (*it == "obstime") {
        obstime = pt::time_from_string_nothrow(buf);
        if( obstime_.is_special() ) {
          obstime_ = obstime;
        }
      }else if (*it == "tbtime") {
        tbtime = pt::time_from_string_nothrow(buf);
        if( tbtime_.is_special() ) {
          tbtime_ = tbtime;
        }
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
/*
  if(  obsid_ ) {
    *obsid_ = obsid;
  }
*/
  if (obstime.is_special() ) {
      obstime = obstime_;
  }

  return kvTextData(stationid, obstime, original, paramid, tbtime, myTypeid);

}


void Observation::setData(const dnmi::db::Result &res){
  long obsid;
  while (res.hasNext()) {
    dnmi::db::DRow & row = const_cast<db::Result&>(res).next();
    data_.push_back(getKvData(row,&obsid));
    if (observationid_ != obsid) { 
      
      // Remove it again if it was not for this observationid
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
      textData_.pop_back(); 
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

size_t Observation::totSize() const { 
  return data_.size()+textData_.size();
 }
size_t Observation::dataSize() const {
   return data_.size();
}

size_t Observation::textDataSize() const { 
  return textData_.size();
}


Observation&  Observation::operator=(const std::list<kvalobs::kvData> &rhs){
  data_= rhs;
  return *this;
}

Observation&  Observation::operator=(const std::list<kvalobs::kvTextData> &rhs){
  textData_ = rhs;
  return *this;
}

//observationid, tbtime, exist
std::tuple<long, pt::ptime, bool>
Observation::getObservationid(dnmi::db::Connection *con, long stationID, long typeID, const boost::posix_time::ptime &obsTime, const std::string &logid) 
{
  ostringstream q;
  try{
    q << "SELECT observationid, tbtime "
      << "FROM observations "
      << "WHERE stationid=" << stationID << " AND typeid=" << typeID
      << " AND obstime=" << dbTime(obsTime) << ";";
  
    std::unique_ptr<dnmi::db::Result> res;
    res.reset(con->execQuery(q.str()));

    if( res.get()==nullptr || !res->hasNext() ) {
      return std::make_tuple(0, pt::microsec_clock::universal_time(), false);
    }

    dnmi::db::DRow & row = res->next();
    auto tbt=pt::time_from_string_nothrow(row[1].c_str());
    if( tbt.is_not_a_date_time() ) {
      IDLOGWARN(logid,"Observation::getObservationid: '" << row[1] << "' not_a_date_time, using now.");
      tbt=pt::microsec_clock::universal_time();
    }
    return std::make_tuple(atol(row[0].c_str()), tbt, true);
  }
  catch(const dnmi::db::SQLSerializeError &e) {
    throw;
  }
  catch(const dnmi::db::SQLException &e) {
    if ( !logid.empty() ) {
      IDLOGERROR(logid, "DBException: Observation::getObservationid: " << string(e.what()) <<"\n" << q.str());
    } else {
      LOGERROR("DBException: Observation::getObservationid: " << string(e.what()) <<"\n" << q.str());
    }
    throw;
  }
  catch(...){
    throw;
  }
}


Observation *Observation::getFromDb(
  dnmi::db::Connection *con, 
  long stationID, 
  long typeID, 
  const boost::posix_time::ptime &obsTime,
  bool useTransaction,
  const std::string &logid
  )
{
  std::unique_ptr<Observation> obs=std::unique_ptr<Observation>(new Observation());
  obs->setLogid(logid);

  //db::TransactionBlock tran(con, db::Connection::REPEATABLE_READ, false, !useTransaction);
  db::TransactionBlock tran(con, db::Connection::READ_COMMITTED, false, !useTransaction);
  ostringstream q;
  try {
    q << "SELECT o.observationid, o.stationid, o.typeid, o.obstime, o.tbtime, d.original,d.paramid,d.sensor,d.level,d.corrected, d.controlinfo, d.useinfo,d.cfailed "
      << "FROM observations o RIGHT JOIN  obsdata d "
      << "ON o.observationid = d.observationid "
      << "WHERE o.stationid=" << stationID << " AND o.typeid=" << typeID 
      << " AND o.obstime=" << dbTime(obsTime) << ";";

    std::unique_ptr<dnmi::db::Result> res;
    res.reset(con->execQuery(q.str()));
 
    if (res->size() == 0 ) {
      q.str("");
      q << "SELECT o.observationid, o.stationid, o.typeid, o.obstime, o.tbtime, d.original,d.paramid "
        << "FROM observations o RIGHT JOIN  obstextdata d "
        << "ON o.observationid = d.observationid "
        << "WHERE o.stationid=" << stationID << " AND o.typeid=" << typeID 
        << " AND o.obstime=" << dbTime(obsTime) << ";";  

      res.reset(con->execQuery(q.str()));

      if (res->size() == 0 ) {
        auto id = getObservationid(con, stationID, typeID, obsTime, logid);
  
        if (get<2>(id)){
          obs->setObservationid(get<0>(id));
          obs->stationid_=stationID;
          obs->typeid_=typeID;
          obs->obstime_=obsTime;
          obs->tbtime_=get<1>(id);

          return obs.release();
        }
        return nullptr;
      } 
   
      obs->setTextData(*res.get());
      return obs.release();    
    }

    obs->setData(*res.get());

    if ( obs->observationid() == 0 ){
      string tmp=q.str();
      q.str("");

      q << "EXCEPTION: Missing observationid. query: " << tmp; 
      IDLOGERROR(logid,"Missing observationid. query: " << tmp << "\n" << "#obsdata: " 
        << obs->dataSize() << " #textdata: " << obs->textDataSize() << " #tot: " << obs->totSize() 
        << " hasObsID: " << (obs->hasObservationid_?"true":"false"));
      
      throw logic_error(q.str());
    }

    q.str("");
    q << "SELECT o.observationid, o.stationid, o.typeid, o.obstime, o.tbtime, d.original,d.paramid "
      << "FROM observations o RIGHT JOIN  obstextdata d "
      << "ON o.observationid = d.observationid "
      << "WHERE d.observationid=" << obs->observationid() << ";";

    res.reset(con->execQuery(q.str()));

    if (res->size() == 0 ) {
      return obs.release();
    }

    obs->setTextData(*res.get());

    return obs.release();
  } 
  catch(const dnmi::db::SQLSerializeError &e) {
    tran.abort();
    throw;
  }
  catch(const dnmi::db::SQLException &e) {
    if ( !logid.empty() ) {
      IDLOGERROR(logid, "DBException: Observation::getFromDb: " << string(e.what()) <<"\n" << q.str());
    } else {
      LOGERROR("DBException: Observation::getFromDb: " << string(e.what()) <<"\n" << q.str());
    }
    tran.abort();
    throw;
  }
  catch(...){
    tran.abort();
    throw;
  }
  
}

Observation *Observation::getFromDb(dnmi::db::Connection *con, long observationid, bool useTransaction, const std::string &logid)
{
  //db::TransactionBlock tran(con, db::Connection::REPEATABLE_READ, false, !useTransaction);
  db::TransactionBlock tran(con, db::Connection::READ_COMMITTED, false, !useTransaction);
  ostringstream q;
  try {
    std::unique_ptr<Observation> obs=std::unique_ptr<Observation>(new Observation());
    obs->setLogid(logid);
    q << "SELECT o.observationid, o.stationid, o.typeid, o.obstime, o.tbtime, d.original,d.paramid,d.sensor,d.level,d.corrected, d.controlinfo, d.useinfo,d.cfailed "
      << "FROM observations o LEFT JOIN  obsdata d "
      << "ON o.observationid = d.observationid "
      << "WHERE d.observationid=" << observationid << ";";

    std::unique_ptr<dnmi::db::Result> res;
    res.reset(con->execQuery(q.str()));

    if (res->size() > 0 ) {
      obs->setData(*res.get());
    } else {     
      q.str("");
      q << "SELECT observationid, stationid, typeid, obstime, tbtime "
        << "FROM observations "
        << "WHERE observationid=" << observationid << ";";
      res.reset(con->execQuery(q.str()));
      if (! obs->set(*res.get()) ) {
        return nullptr; 
      } 
    } 

    q.str("");
    q << "SELECT o.observationid, o.stationid, o.typeid, o.obstime, o.tbtime, d.original,d.paramid "
      << "FROM observations o LEFT JOIN  obstextdata d "
      << "ON o.observationid = d.observationid "
      << "WHERE d.observationid=" << observationid << ";";

    res.reset(con->execQuery(q.str()));

    if (res->size() == 0 ) {
      return obs.release();
    }

    obs->setTextData(*res.get());

    return obs.release();
  } 
  catch(const dnmi::db::SQLSerializeError &e) {
    tran.abort();
    throw;
  }
  catch(const dnmi::db::SQLException &e) {
    if ( !logid.empty() ) {
      IDLOGERROR(logid, "DBException: Observation::getFromDb: " << string(e.what()) <<"\n" << q.str());
    } else {
      LOGERROR("DBException: Observation::getFromDb: " << string(e.what()) <<"\n" << q.str());
    }

    tran.abort();
    throw;
  }
  catch(...){
    tran.abort();
    throw;
  }
  
}



void Observation::insertIntoDb(dnmi::db::Connection *con, bool useTransaction, const std::string &logid) 
{
  //db::TransactionBlock tran(con, db::Connection::REPEATABLE_READ, false, !useTransaction);
  db::TransactionBlock tran(con, db::Connection::READ_COMMITTED, false, !useTransaction);
  ostringstream q;

  try {
    string tbtime;

    if ( tbtime_.is_special() ) {
      tbtime=dbTime(pt::microsec_clock::universal_time());
    } else {
      tbtime=dbTime(tbtime_);
    }

    q << "INSERT INTO observations (stationid, typeid, obstime,tbtime) VALUES(" 
      << stationid_ << "," << typeid_ << "," << dbTime(obstime_) << "," << tbtime << ")"; 
  
    con->exec(q.str());
  
    observationid_ = currentObservationid(con);

    q.str("");
    for(auto &d : data_) {
      q << "INSERT INTO obsdata (observationid,original,paramid,sensor,level,corrected,controlinfo,useinfo,cfailed) VALUES("
        << observationid_ <<"," << d.original() << ","
        << d.paramID() << "," << d.sensor() << "," <<d.level() << "," << d.corrected() <<"," 
        << Q(d.controlinfo().flagstring()) <<","<< Q(d.useinfo().flagstring()) << "," << Q(d.cfailed()) << ");\n";
    }

    for(auto &d : textData_) {
      q << "INSERT INTO obstextdata (observationid,original,paramid) VALUES("
        << observationid_ <<",'" << con->esc(d.original()) << "',"
        << d.paramID() <<");\n";
    }
    con->exec(q.str());
  } 
  catch(const dnmi::db::SQLSerializeError &e) {
    tran.abort();
    throw;
  }
  catch(const dnmi::db::SQLException &e) {
    if ( !logid.empty() ) {
      IDLOGERROR(logid, "DBException: Observation::insertIntoDb: " << string(e.what()) <<"\n" << q.str());
    } else {
      LOGERROR("DBException: Observation::insertIntoDb: " << string(e.what()) <<"\n" << q.str());
    }

    tran.abort();
    throw;
  }
  catch(...){
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
