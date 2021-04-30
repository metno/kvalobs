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
#ifndef __lib_kvalobs_observation_h__
#define __lib_kvalobs_observation_h__

#include <list>
#include <iosfwd>
#include <tuple>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>


namespace kvalobs {
/**
 * \addtogroup  dbinterface
 *
 * @{
 */

/**
 * \brief Interface to the table data in the kvalobs database.
 */

class Observation  {
public:
  Observation();
  Observation(const Observation &d);
  
  Observation(int stationId, int typeId, const boost::posix_time::ptime & obt,
    const boost::posix_time::ptime & tbt, 
    const std::list<kvalobs::kvData> &data=std::list<kvalobs::kvData>(),
    const std::list<kvalobs::kvTextData> &textData=std::list<kvalobs::kvTextData>(),
    const std::string &logid="")
    : hasObservationid_(false),observationid_(0), logid_(logid) {
    set(stationId, typeId, obt, tbt, data, textData);
  }

  void setLogid(const std::string &logid) {
    logid_=logid;
  }
  
  bool set(int stationId, int typeId, 
    const boost::posix_time::ptime & obt,
    const boost::posix_time::ptime & tbt, 
    const std::list<kvalobs::kvData> &data=std::list<kvalobs::kvData>(),
    const std::list<kvalobs::kvTextData> &textData=std::list<kvalobs::kvTextData>() );
 
  
  Observation&  operator=(const Observation &rhs);
  Observation&  operator=(const std::list<kvalobs::kvData> &rhs);
  Observation&  operator=(const std::list<kvalobs::kvTextData> &rhs);

  void cleanData();

  std::list<kvData> data() { return data_;}
  std::list<kvTextData> textData(){ return textData_;}

  long observationid()const{ return observationid_;}
  
  int stationID() const {
    return stationid_;
  }
  
  const boost::posix_time::ptime & obstime() const {
    return obstime_;
  }
  
  const boost::posix_time::ptime & tbtime() const {
    return tbtime_;
  }
  
  int typeID() const {
    return typeid_;
  }
  
  void tbtime(const boost::posix_time::ptime &tbtime) {
    tbtime_ = tbtime;
  }
  
  void typeID(int t) {
    typeid_ = t;
  }
  
  size_t totSize() const;
  size_t dataSize() const;
  size_t textDataSize() const;


  /**
   * @return a pointer observation if found and nullptr if not. The caller must delete the pointer.
   * @throws SQLException on db error.
   * @throws std::logic_error if some unexpected data was found.
   */
  static Observation *getFromDb(dnmi::db::Connection *con, long stationID, long typeID, const boost::posix_time::ptime &obsTime, bool useTransaction=true, const std::string &logid="");

  /**
   * @return a pointer observation if found and nullptr if not. The caller must delete the pointer.
   * @throws SQLException on db error.
   * @throws std::logic_error if some unexpected data was found.
   */
  static Observation *getFromDb(dnmi::db::Connection *con, long observationid, bool useTransaction=true, const std::string &logid="");

  
  
  /**
   * @return a pointer observation if found and nullptr if not. The caller must delete the pointer.
   * @throws SQLException on db error.
   * @throws std::logic_error if some unexpected data was found.
   */
  void insertIntoDb(dnmi::db::Connection *con, bool useTransaction=true, const std::string &logid="");


  friend std::ostream& operator<<(std::ostream& output,
                                  const kvalobs::Observation &d);

protected:
  long currentObservationid(dnmi::db::Connection *con);
  //void getDataForObservationid(dnmi::db::Connection *con, long observationid, const boost::posix_time::ptime &obsTime ); 
  bool set(const dnmi::db::Result &res);
  void setData(const dnmi::db::Result &res);
  void setTextData(const dnmi::db::Result &res);
  void setObservationid(long observationid);
  kvData getKvData(const dnmi::db::DRow &row, long *obsid);
  kvTextData getKvTextData(const dnmi::db::DRow &row, long *obsid);
  static std::tuple<long, boost::posix_time::ptime, bool> getObservationid(dnmi::db::Connection *con, long stationID, long typeID, const boost::posix_time::ptime &obsTime, const std::string &logid);


private:
  bool hasObservationid_;
  long observationid_;
  int stationid_;
  int typeid_;
  boost::posix_time::ptime obstime_;
  boost::posix_time::ptime tbtime_;
  std::list<kvData> data_;
  std::list<kvTextData> textData_;
  std::string logid_;
};
}
#endif