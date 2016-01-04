/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 Copyright (C) 2010 met.no

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

#include "databaseResultFilter.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <set>
#include <vector>

namespace db {
namespace resultfilter {
struct caseInsensitiveEq {
  std::string wantedLower;
  caseInsensitiveEq(const std::string & wanted)
      : wantedLower(boost::algorithm::to_lower_copy(wanted)) {
  }

  bool operator ()(const std::string & s) const {
    return s == wantedLower or boost::algorithm::to_lower_copy(s) == wantedLower;
  }
};

float parseStationParam(const std::string & metadataToParse,
                        const std::string & wanted) {
  std::vector<std::string> params;
  boost::split(params, metadataToParse, boost::algorithm::is_any_of(";\n"));

  std::vector<std::string>::const_iterator find = std::find_if(
      params.begin(), params.end(), caseInsensitiveEq(wanted));

  if (find == params.end())
    throw std::runtime_error(wanted + ": No such station_param");
  std::advance(find, params.size() / 2);

  float val = boost::lexical_cast<float>(*find);
  return val;
}

namespace {
template<class DList>
void filterTypeId_(DList & data, int preferredTypeId) {
  std::map<int, DList> typeSortedData;
  for (typename DList::const_iterator it = data.begin(); it != data.end(); ++it)
    typeSortedData[it->typeID()].push_back(*it);
  if (typeSortedData.size() > 1) {
    typename std::map<int, DList>::iterator preferred = typeSortedData.find(
        preferredTypeId);
    if (preferred != typeSortedData.end())
      data.swap(preferred->second);
    else
      data.swap(typeSortedData.rbegin()->second);
  }
}

template<class T, class F>
void filter_(db::DatabaseAccess::DataList & data, F function) {
  std::set<T> values;
  for (db::DatabaseAccess::DataList::const_iterator it = data.begin();
      it != data.end(); ++it)
    values.insert(function(*it));

  if (values.size() > 1) {
    const T wantedValue = *values.begin();  // default: choose lowest value

    db::DatabaseAccess::DataList selectedData;
    for (db::DatabaseAccess::DataList::const_iterator it = data.begin();
        it != data.end(); ++it)
      if (function(*it) == wantedValue)
        selectedData.push_back(*it);
    data.swap(selectedData);
  }
}
}

void filter(db::DatabaseAccess::DataList & data, int preferredTypeId) {
  filterTypeId_(data, preferredTypeId);
  filter_<int>(data, std::mem_fun_ref(&kvalobs::kvData::level));
  filter_<char>(data, std::mem_fun_ref(&kvalobs::kvData::sensor));
}

void filter(db::DatabaseAccess::TextDataList & data, int preferredTypeId) {
  filterTypeId_(data, preferredTypeId);
}

}
}
