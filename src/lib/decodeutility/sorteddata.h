/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: sorteddata.h,v 1.1.2.3 2007/09/27 09:02:27 paule Exp $

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
#ifndef SORTEDDATA_H
#define SORTEDDATA_H

#include <set>
#include <utility>
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>

namespace kvalobs {
namespace serialize {
namespace internal {

/**
 * A simplifying layer over std::set, allowing two features:
 * 1. Modifying the content of the set
 * 2. Accessing content with operator[] (both const and non-const)
 */
template<typename Next>
class Container {
 protected:
  typedef std::set<Next> NextData;
  mutable NextData next;
 public:
  virtual ~Container() {
  }

  Next & operator[](const Next & n) {
    return const_cast<Next &>(*next.insert(n).first);
  }

  const Next & operator[](const Next & n) const {
    return *next.insert(n).first;
  }

  bool empty() const {
    return next.empty();
  }

  size_t size() const {
    return next.size();
  }

  virtual size_t count() const {
    size_t ret = 0;
    for (const_iterator it = begin(); it != end(); ++it)
      ret += it->count();
    return ret;
  }

  void erase(const Next & n) {
    next.erase(n);
  }

  void clear() {
    next.clear();
  }

  typedef typename NextData::iterator iterator;
  iterator begin() {
    return next.begin();
  }
  iterator end() {
    return next.end();
  }

  typedef typename NextData::const_iterator const_iterator;
  const_iterator begin() const {
    return next.begin();
  }
  const_iterator end() const {
    return next.end();
  }

  typedef typename NextData::value_type value_type;
  iterator find(const value_type &what) {
    return next.find(what);
  }
  const_iterator find(const value_type &what) const {
    return next.find(what);
  }

};

/**
 * Allows a \c Container to be the content of another \c Container, using the key \c Content.
 */
template<typename Content, typename Next>
class SortedData : public Container<Next> {
  Content content_;
 public:
  SortedData(Content c)
      : content_(c) {
  }

  Content & get() {
    return content_;
  }

  const Content & get() const {
    return content_;
  }

  bool operator<(const SortedData<Content, Next> & d) const {
    return content_ < d.get();
  }
  bool operator==(const SortedData<Content, Next> & d) const {
    return content_ == d.get();
  }
};

/**
 * A leaf item for SortedData.
 */
template<typename Content>
class LeafItem {
  int paramID_;
  Content content_;
 public:
  LeafItem(int param, const Content & content = Content())
      : paramID_(param),
        content_(content) {
  }
  size_t count() const {
    return 1;
  }
  bool operator<(const LeafItem<Content> & d) const {
    return paramID_ < d.paramID();
  }

  int paramID() const {
    return paramID_;
  }
  const Content & content() const {
    return content_;
  }
  Content & content() {
    return content_;
  }
};

/**
 * Data for a \c kvData object that is not part of the key for that object.
 */
struct DataContent {
  DataContent(float orig, float corr, const kvalobs::kvControlInfo ci,
              const kvalobs::kvUseInfo & ui, const std::string & fail)
      : original(orig),
        corrected(corr),
        controlinfo(ci),
        useinfo(ui),
        cfailed(fail) {
  }
  DataContent(const kvData & d)
      : original(d.original()),
        corrected(d.corrected()),
        controlinfo(d.controlinfo()),
        useinfo(d.useinfo()),
        cfailed(d.cfailed()) {
  }
  DataContent()
      : original(std::numeric_limits<float>::max()),
        corrected(std::numeric_limits<float>::max()) {
  }

  float original;
  float corrected;
  kvalobs::kvControlInfo controlinfo;
  kvalobs::kvUseInfo useinfo;
  std::string cfailed;
};
/**
 * Data for a \c kvTextData object that is not part of the key for that object.
 */
struct TextDataContent {
  explicit TextDataContent(const std::string & orig)
      : original(orig) {
  }
  TextDataContent(const kvTextData & d)
      : original(d.original()) {
  }
  TextDataContent() {
  }
  std::string original;
  boost::posix_time::ptime tbtime;
};

typedef LeafItem<DataContent> DataItem;
typedef LeafItem<TextDataContent> TextDataItem;

typedef SortedData<int, DataItem> Level;
typedef SortedData<int, Level> Sensor;

/**
 * \see Observations
 */
class TbTime : public SortedData<boost::posix_time::ptime, Sensor> {
 public:
  TbTime(boost::posix_time::ptime t)
      : SortedData<boost::posix_time::ptime, Sensor>(t) {
  }

  Container<TextDataItem> textData;

  virtual size_t count() const {
    return SortedData<boost::posix_time::ptime, Sensor>::count()
        + textData.count();
  }
};

/**
 * \see Observations
 */
class ObsTime : public SortedData<boost::posix_time::ptime, TbTime> {
  bool invalidate_;
 public:
  ObsTime(boost::posix_time::ptime t)
      : SortedData<boost::posix_time::ptime, TbTime>(t),
        invalidate_(false) {
  }

  void invalidate(bool doit) {
    invalidate_ = doit;
  }

  bool invalidate() const {
    return invalidate_;
  }
};

typedef SortedData<int, ObsTime> TypeID;
typedef SortedData<int, TypeID> StationID;

/**
 * The one thing you need to use from here. A collection of data, ordered by
 * station, type, obstime, sensor, level and paramid, or in the case of \c
 * kvTextData, station, type, obstime and paramID.
 *
 * \c kvData leaf information may be accessed in the following way:
 * <code>obs[station][type][obstime][sensor][level][param]</code>
 *
 * \c kvTextData leaf information may be accessed like this:
 * <code>obs[station][type][obstime][param]</code>
 *
 * There is no direct way to get \c kvData or \c kvTextData object from this.
 * you will have to do this manually.
 */
typedef Container<StationID> Observations;

}
}
}

#endif
