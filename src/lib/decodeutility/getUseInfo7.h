/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: decodeutility.h,v 1.1.2.3 2007/09/27 09:02:27 paule Exp $

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

#ifndef __GETUSEINFO7_H__
#define __GETUSEINFO7_H__

#include <list>
#include <boost/date_time/posix_time/ptime.hpp>
#include <kvalobs/kvTypes.h>

namespace decodeutility {

/**
 * getUseinfo7Code computes the useinfo(7) flag that
 * says if the observation was received to late or to early.
 *
 * It use the earlyObs and lateObs from the 'type' table
 * set the flag.
 *
 * The flag codes returned:
 *
 *   - 4, the observation was received to late.
 *   - 3, the observation was received to early.
 *   - 0, the observation was received on time.
 *
 * @param typeId The typeid to the observation.
 * @param receivedTime The time the observation was received.
 * @param obstime The time that the observation is valid for.
 * @param typeList The type list of from the type table.
 * @return -1 if the type was not recognized and the flag code otherwise.
 */

int
getUseinfo7Code(int typeId, const boost::posix_time::ptime &referencetime,
                const boost::posix_time::ptime &obtime,
                const std::list<kvalobs::kvTypes> &typeList);
}

#endif
