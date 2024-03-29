/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvalobsdata.h,v 1.1.2.3 2007/09/27 09:02:27 paule Exp $

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
#ifndef KVALOBSDATA_H
#define KVALOBSDATA_H

#include "sorteddata.h"
#include <string>
#include <map>
#include <list>
#include <set>
#include <utility>
#include <kvalobs/kvDataFlag.h>
#include <kvalobs/kvRejectdecode.h>
#include <kvalobs/kvStationInfo.h>

namespace kvalobs {
class kvData;
class kvTextData;

namespace serialize {

/**
 * The content of a serialized message.
 *
 * \see KvalobsDataSerializer
 *
 * @author Vegard B�nes
 */
class KvalobsData {
 public:
  KvalobsData();
  KvalobsData(const KvalobsData &d);

  KvalobsData(const std::list<kvData> & data,
              const std::list<kvTextData> & tdata = std::list<kvTextData>());

  ~KvalobsData();

  std::string producer()const;

  void producer(const std::string &prod );

  std::string msgid()const;
  void msgid(const std::string &id);


  /**
   * True if no data is contained in this object.
   */
  bool empty() const;

  /**
   * Number of observations (not parameters) in this object.
   */
  size_t size() const;

  /**
   * Add data to object
   */
  void insert(const kvalobs::kvData & d);

  /**
   * Add text data to object
   */
  void insert(const kvalobs::kvTextData & d);

  /**
   * Add data to object, from an iterator range.
   */
  template<typename InputIterator>
  void insert(InputIterator begin, InputIterator end) {
    for (; begin != end; ++begin)
      insert(*begin);
  }

  void setMessageCorrectsThisRejection(
      const kvalobs::kvRejectdecode & previouslyRejectedMessage);

  /**
   * Get all data from object, with the given tbtime
   */
  void getData(std::list<kvalobs::kvData> & out,
               const boost::posix_time::ptime & tbtime =
                   boost::posix_time::ptime()) const;
  /**
   * Get all text data from object, with the given tbtime
   */
  void getData(std::list<kvalobs::kvTextData> & out,
               const boost::posix_time::ptime & tbtime =
                   boost::posix_time::ptime()) const;
  /**
   * Get all data and text datafrom object, with the given tbtime
   */
  void getData(std::list<kvalobs::kvData> & out1,
               std::list<kvalobs::kvTextData> & out2,
               const boost::posix_time::ptime & tbtime =
                   boost::posix_time::ptime()) const;

  /**
   * Get all data from object.
   *
   * @param setTbtime if true set the \em tbtime to the value of the input
   * parameter tbtime.
   * @param tbtime The value to set tbtime to.
   */
  void data(std::list<kvalobs::kvData> & out, bool setTbtime = false,
            const boost::posix_time::ptime & tbtime =
                boost::posix_time::ptime()) const;

  /**
   * Get all text data from object.
   *
   * @param setTbtime if true set the \em tbtime to the value of the input
   * parameter tbtime.
   * @param tbtime The value to set tbtime to.
   */
  void data(std::list<kvalobs::kvTextData> & out, bool setTbtime = false,
            const boost::posix_time::ptime & tbtime =
                boost::posix_time::ptime()) const;

  /**
   * Get all data and text data from object.
   *
   * @param setTbtime if true set the \em tbtime to the value of the input
   * parameter tbtime.
   * @param tbtime The value to set tbtime to.
   */
  void data(std::list<kvalobs::kvData> & out1,
            std::list<kvalobs::kvTextData> & out2, bool setTbtime = false,
            const boost::posix_time::ptime & tbtime =
                boost::posix_time::ptime()) const;

  typedef std::vector<kvalobs::kvRejectdecode> RejectList;
  void getRejectedCorrections(RejectList & out) const {
    out = correctedMessages_;
  }

  /**
   * Set overwrite specification
   *
   * Shall the kvalobs decoder ignore and overwrite any values in the database?
   */
  void overwrite(bool doit) {
    overwrite_ = doit;
  }

  /**
   * Get overwrite specification
   *
   * Shall the kvalobs decoder ignore and overwrite any values in the database?
   */
  bool overwrite() const {
    return overwrite_;
  }

  /**
   * Remove all data and reset the overwrite flag to false.
   *
   */
  void clear() {
    overwrite_ = false;
    obs_.clear();
  }

  /**
   * Set invalidate specification.
   *
   * If invalidate is true, all parametes which forms a specific observation
   * will be rejected, before the new data is inserted.
   *
   * If overwrite() is true as well, all data will be deleted before
   * inserting the new values.
   */
  void invalidate(bool doit, int station, int typeID,
                  const boost::posix_time::ptime & obstime);

  /**
   * Query invalidate specification. Shall the given station, typeId, and
   * obstime be invalidated?
   *
   * @see invalidate
   */
  bool
  isInvalidate(int station, int typeID,
               const boost::posix_time::ptime & obstime) const;

  /**
   * Specification for what observations will be invalidated
   *
   * @see invalidate
   */
  struct InvalidateSpec {
    int station;
    int typeID;
    boost::posix_time::ptime obstime;
    InvalidateSpec(int st, int ty, boost::posix_time::ptime ot)
        : station(st),
          typeID(ty),
          obstime(ot) {
    }
  };

  /**
   * Get a complete list of observations to be invalidated.
   *
   * @see invalidate
   */
  void getInvalidate(std::list<InvalidateSpec> & invSpec);

  /**
   * Const access to data holder
   */
  const internal::Observations & obs() const {
    return obs_;
  }

  /**
   * Get the set of station/typeid/obstime contained here. Does not include rejected data
   */
  std::set<kvalobs::kvStationInfo> summary() const;

  ///Can return is_spesial if the xml-document it is created from does not have the attribute 'created'.
  boost::posix_time::ptime created()const;

  ///Mostly for test and internal use. It is set when the data is deserialized.
  ///It will set the attribute
  void created(const boost::posix_time::ptime &time);
  void created(const std::string &isotimestamp ); /// Can be an empty string -> is_spesial

  friend std::ostream& operator<<(std::ostream &o, const KvalobsData &d);
 private:
  bool overwrite_;
  internal::Observations obs_;

  RejectList correctedMessages_;
  boost::posix_time::ptime created_;
};

std::ostream& operator<<(std::ostream &o, const KvalobsData &d);
}

}

#endif
