/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvalobsdataserializer.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $

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
#ifndef KVALOBS_SERIALIZEKVALOBSDATASERIALIZER_H
#define KVALOBS_SERIALIZEKVALOBSDATASERIALIZER_H

#include "kvalobsdata.h"
#include <string>

namespace kvalobs {

namespace serialize {

/**
 * Serializes a \c KvalobsData object into an XML string, or an XML string
 * into a \c KvalobsData object.
 *
 * @author Vegard Bønes
 */
class KvalobsDataSerializer {
 public:
  KvalobsDataSerializer();
  KvalobsDataSerializer(const KvalobsData & d, const std::string &producer);
  KvalobsDataSerializer(const KvalobsData & d);
  KvalobsDataSerializer(const std::string & s);

  ~KvalobsDataSerializer();
  std::string producer()const;

  static std::string serialize(const KvalobsData & d,const std::string &producer);
  static std::string serialize(const KvalobsData & d);

  const KvalobsData & toData() const;

  KvalobsData & toData();

  std::string toString() const;
  std::string toString(const boost::posix_time::ptime &created) const;

  
 private:
  KvalobsData data_;
};




}
;

}
;

#endif
