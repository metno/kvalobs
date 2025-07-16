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

#ifndef DATABASERESULTFILTER_H_
#define DATABASERESULTFILTER_H_

#include <db/DatabaseAccess.h>

namespace db {
namespace resultfilter {
/**
 * Extract data from station_param.
 *
 * @param metadataToParse The full station_param string
 * @param wanted Wanted key from the string.
 * @return The parsed value
 *
 * \ingroup group_db
 */
float parseStationParam(const std::string & metadataToParse,
                        const std::string & wanted);

/**
 * Select correct data from the given list, based on typeid. If we have
 * several candidates for representing the same piece of data, with respect to
 * everything except typeid, we prefer either the preferredTypeId, or the
 * highest typeid.
 *
 * Also, if multiple levels or sensors are present, the lowest will be preferred.resultfilter
 *
 * @param data The data to filter
 * @param preferredTypeId The given typeid will be preferred to any other typeids
 *
 * \ingroup group_db
 */

/**
 * @brief Filters a DataList to select the most appropriate data based on typeid, level, and sensor.
 *
 * Select correct data from the given list, based on typeid. If we have
 * several candidates for representing the same piece of data, with respect to
 * everything except typeid, we prefer either the preferredTypeId, or the
 * highest typeid.
 * 
 * If multiple levels or sensors are present, the lowest value
 * for each is preferred based on filterByLevel and filterBySensor. 
 *
 * @param data The data to filter.
 * @param preferredTypeId The typeid to prefer over others.
 * @param filterByLevel If true, filter by level (default: true).
 * @param filterBySensor If true, filter by sensor (default: true).
 *
 * \ingroup group_db
 */
  void filter(db::DatabaseAccess::DataList & data, int preferredTypeId,
            bool filterByLevel=true, bool filterBySensor=true);
 
/**
 * Select correct data from the given list, based on typeid. If we have
 * several candidates for representing the same piece of data, with respect to
 * everything except typeid, we prefer either the preferredTypeId, or the
 * highest typeid.
 *
 * @param data The data to filter
 * @param preferredTypeId The given typeid will be preferred to any other typeids
 *
 * \ingroup group_db
 */
void filter(db::DatabaseAccess::TextDataList & data, int preferredTypeId);

}
}

#endif /* DATABASERESULTFILTER_H_ */
