/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvalobsdata.cc,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $

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
#include "kvalobsdata.h"
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvTextDataOperations.h>
#include <kvalobs/kvTextData.h>
#include <miutil/timeconvert.h>
#include <boost/any.hpp>
#include <stack>

using namespace std;
using boost::posix_time::ptime;
using boost::posix_time::special_values;
using boost::posix_time::time_from_string_nothrow;

namespace kvalobs {
namespace serialize {

using internal::Observations;
using internal::StationID;
using internal::TypeID;
using internal::ObsTime;
using internal::TbTime;
using internal::Sensor;
using internal::Level;
using internal::TextDataItem;
using internal::Container;

KvalobsData::KvalobsData()
    : overwrite_(false) {
}

KvalobsData::KvalobsData(const std::list<kvData> & data,
                         const std::list<kvTextData> & tdata)
    : overwrite_(false) {
  for (list<kvData>::const_iterator it = data.begin(); it != data.end(); ++it)
    insert(*it);
  for (list<kvTextData>::const_iterator it = tdata.begin(); it != tdata.end();
      ++it)
    insert(*it);
}

KvalobsData::~KvalobsData() {
}

bool KvalobsData::empty() const {
  return obs_.count() == 0;
}

size_t KvalobsData::size() const {
  return obs_.count();
}
namespace {
void removeExisting(internal::Observations & obs, const kvData & d) {
  int sensor = d.sensor();
  if (sensor >= '0')
    sensor -= '0';

  ObsTime & intermediateKey = obs[d.stationID()][d.typeID()][d.obstime()];
  for (auto & tb : intermediateKey) {
    Level & leaf = const_cast<Level&>(tb[sensor][d.level()]);
    leaf.erase(d.paramID());
  }
}

void removeExisting(internal::Observations & obs, const kvTextData & d) {
  ObsTime & intermediateKey = obs[d.stationID()][d.typeID()][d.obstime()];
  for (auto & tb : intermediateKey) {
    TbTime & leaf = const_cast<TbTime&>(tb);
    leaf.erase(d.paramID());
  }
}
}  // namespace

void KvalobsData::insert(const kvData & d) {
  removeExisting(obs_, d);

  int sensor = d.sensor();
  if (sensor >= '0')
    sensor -= '0';

  ptime tbtime =
      d.tbtime().is_special() ? ptime(special_values::neg_infin) : d.tbtime();

  obs_[d.stationID()][d.typeID()][d.obstime()][tbtime][sensor][d.level()][d
      .paramID()].content() = d;
}

void KvalobsData::insert(const kvTextData & d) {
  removeExisting(obs_, d);

  ptime tbtime =
      d.tbtime().is_special() ? ptime(special_values::neg_infin) : d.tbtime();
  obs_[d.stationID()][d.typeID()][d.obstime()][tbtime].textData[d.paramID()]
      .content() = d;
}

void KvalobsData::setMessageCorrectsThisRejection(
    const kvalobs::kvRejectdecode & previouslyRejectedMessage) {
  correctedMessages_.push_back(previouslyRejectedMessage);
}

void KvalobsData::getData(std::list<kvalobs::kvData> & out,
                          const boost::posix_time::ptime & tbtime) const {
  data(out, true, tbtime);
}

/**
 * Get all text data from object, with the given tbtime
 */
void KvalobsData::getData(std::list<kvalobs::kvTextData> & out,
                          const boost::posix_time::ptime & tbtime) const {
  data(out, true, tbtime);
}

void KvalobsData::getData(std::list<kvalobs::kvData> & out1,
                          std::list<kvalobs::kvTextData> & out2,
                          const boost::posix_time::ptime & tbtime) const {
  data(out1, out2, true, tbtime);
}

void KvalobsData::data(list<kvData> & out, bool setTbtime,
                       const boost::posix_time::ptime & tbtime) const {

  for (Observations::const_iterator s = obs_.begin(); s != obs_.end(); ++s) {
    for (StationID::const_iterator t = s->begin(); t != s->end(); ++t) {
      for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o) {
        for (ObsTime::const_iterator tbt = o->begin(); tbt != o->end(); ++tbt) {
          for (TbTime::const_iterator sensor = tbt->begin();
              sensor != tbt->end(); ++sensor) {
            for (Sensor::const_iterator level = sensor->begin();
                level != sensor->end(); ++level) {
              for (Level::const_iterator param = level->begin();
                  param != level->end(); ++param) {

                const internal::DataContent & c = param->content();
                kvData d(s->get(), o->get(), c.original, param->paramID(),
                         setTbtime ? tbtime : tbt->get(), t->get(),
                         sensor->get(), level->get(), c.corrected,
                         c.controlinfo, c.useinfo, c.cfailed);
                out.push_back(d);
              }
            }
          }
        }
      }
    }
  }

  //Sort the return data, ignoring tbtime, so it is compatible with previous versions.
  out.sort(kvalobs::compare::lt_kvData());
}

void KvalobsData::data(list<kvTextData> & out, bool setTbtime,
                       const boost::posix_time::ptime & tbtime) const {
  for (Observations::const_iterator s = obs_.begin(); s != obs_.end(); ++s) {
    for (StationID::const_iterator t = s->begin(); t != s->end(); ++t) {
      for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o) {
        for (ObsTime::const_iterator tbt = o->begin(); tbt != o->end(); ++tbt) {
          for (Container<TextDataItem>::const_iterator param = tbt->textData
              .begin(); param != tbt->textData.end(); ++param) {

            kvTextData d(s->get(), o->get(), param->content().original,
                         param->paramID(), setTbtime ? tbtime : tbt->get(),
                         t->get());
            out.push_back(d);
          }
        }
      }
    }
  }

  //Sort the return data, ignoring tbtime, so it is compatible with previous versions.
  out.sort(kvalobs::compare::lt_kvTextData());
}

void KvalobsData::data(std::list<kvalobs::kvData> & out1,
                       std::list<kvalobs::kvTextData> & out2, bool setTbtime,
                       const boost::posix_time::ptime & tbtime) const {
  data(out1, setTbtime, tbtime);
  data(out2, setTbtime, tbtime);
}

void KvalobsData::invalidate(bool doit, int station, int typeID,
                             const boost::posix_time::ptime & obstime) {
  obs_[station][typeID][obstime].invalidate(doit);
}

bool KvalobsData::isInvalidate(int station, int typeID,
                               const boost::posix_time::ptime & obstime) const {
  return obs_[station][typeID][obstime].invalidate();
}

void KvalobsData::getInvalidate(std::list<InvalidateSpec> & invSpec) {
  using namespace internal;
  for (Observations::const_iterator s = obs_.begin(); s != obs_.end(); ++s) {
    for (StationID::const_iterator t = s->begin(); t != s->end(); ++t) {
      for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o) {
        if (o->invalidate())
          invSpec.push_back(InvalidateSpec(s->get(), t->get(), o->get()));
      }
    }
  }
}

boost::posix_time::ptime KvalobsData::created()const{
  return created_;
}

  ///Mostly for test and internal use. It is set by the constructor or when it is created
  ///by the deserializer.
void KvalobsData::created(const boost::posix_time::ptime &time) {
  created_=time;
}

void KvalobsData::created(const std::string &isotimestamp ){ /// Can be an empty string -> is_spesial
  if( isotimestamp.empty())
    created_=ptime();
  else
    created_=time_from_string_nothrow(isotimestamp);
}



std::set<kvalobs::kvStationInfo> KvalobsData::summary() const {
  std::set<kvalobs::kvStationInfo> ret;

  using namespace internal;
  for (Observations::const_iterator s = obs_.begin(); s != obs_.end(); ++s)
    for (StationID::const_iterator t = s->begin(); t != s->end(); ++t)
      for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o)
        ret.insert(kvalobs::kvStationInfo(s->get(), o->get(), t->get()));

  return ret;
}

std::ostream& operator<<(std::ostream &ost, const KvalobsData &data) {
  for (Observations::const_iterator s = data.obs_.begin(); s != data.obs_.end(); ++s) {
    for (StationID::const_iterator t = s->begin(); t != s->end(); ++t) {
      for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o) {
        for (ObsTime::const_iterator tbt = o->begin(); tbt != o->end(); ++tbt) {
          for (TbTime::const_iterator sensor = tbt->begin(); sensor != tbt->end(); ++sensor) {
            for (Sensor::const_iterator level = sensor->begin(); level != sensor->end(); ++level) {
              for (Level::const_iterator param = level->begin(); param != level->end(); ++param) {
                const internal::DataContent & c = param->content();
                kvData d(s->get(), o->get(), c.original, param->paramID(), tbt->get(), t->get(), sensor->get(), level->get(), c.corrected, c.controlinfo,
                         c.useinfo, c.cfailed);
                ost << d << endl;
              }
            }
          }
        }
      }
    }
  }

  for (Observations::const_iterator s = data.obs_.begin(); s != data.obs_.end(); ++s) {
    for (StationID::const_iterator t = s->begin(); t != s->end(); ++t) {
      for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o) {
        for (ObsTime::const_iterator tbt = o->begin(); tbt != o->end(); ++tbt) {
          for (Container<TextDataItem>::const_iterator param = tbt->textData
              .begin(); param != tbt->textData.end(); ++param) {
            kvTextData d(s->get(), o->get(), param->content().original,
                         param->paramID(), tbt->get(),
                         t->get());
            ost << d << endl;
          }
        }
      }
    }
  }

  return ost;
}




}

}
