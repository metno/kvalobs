/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: GenCacheElem.cc,v 1.1.2.2 2007/09/27 09:02:34 paule Exp $                                                       

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
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvGeneratedTypes.h>
#include "GenCacheElem.h"

using namespace std;
using namespace kvalobs;

bool GenCache::isGenerated(long stationid, long typeid_,
                           dnmi::db::Connection *con) {
  if (typeid_ < 0)
    return true;

  boost::mutex::scoped_lock l(genCacheMutex);

  if (genCacheFlush.is_not_a_date_time()) {
    genCacheFlush = boost::posix_time::microsec_clock::universal_time();
  } else if (boost::posix_time::microsec_clock::universal_time() - genCacheFlush
      > boost::posix_time::hours(24)) {
    genCacheFlush = boost::posix_time::microsec_clock::universal_time();
    genCachElem.clear();
  }

  for (list<GenCachElem>::iterator it = genCachElem.begin();
      it != genCachElem.end(); it++) {
    if (it->stationID() == stationid && it->typeID() == typeid_) {
      return it->generated();
    }
  }

  kvDbGate gate(con);
  list<kvGeneratedTypes> stList;

  if (!gate.select(stList, kvQueries::selectIsGenerated(stationid, typeid_))) {
    LOGDEBUG("isGenerated: DBERROR: " << gate.getErrorStr());
    throw dnmi::db::SQLException(gate.getErrorStr());
  }

  genCachElem.push_back(GenCachElem(stationid, typeid_, !stList.empty()));

  return !stList.empty();
}
