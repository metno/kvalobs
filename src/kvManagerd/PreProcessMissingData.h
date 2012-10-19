/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: PreProcessMissingData.h,v 1.2.2.2 2007/09/27 09:02:35 paule Exp $

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
#ifndef _PreProcessMissingData_h
#define _PreProcessMissingData_h

#include <list>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvData.h>
#include <puTools/miTime.h>
#include "PreProcessJob.h"

/**
 * PreProcessMissing data search for missing observation and "wild observations".
 * An "wild observation" is defined as data that is not defined in the observation
 * program as defined by the table obs_pgm.
 *
 * In the search for missing observation the obs_pgm table and
 * lateobs from the types table is used.
 *
 * There is two cases for missing observation.
 *   1. The complete message defined by obstime, stationid and typeid
 *      is missing.
 *   2. Some observation in the message are missing.
 *
 * In case 1 we mark the observation as missing when the lateobs time
 * is passed. The run for missing message is run every hour at hh:30.
 *
 * In case 2 the missing parameters is marked as missing regardless of lateobs.
 * The search for missing observations in a message is run when the message is
 * received.
 *
 * For "wild observations" only the obs_pgm is used.
 */
class PreProcessMissingData : public PreProcessJob {
protected:
   miutil::miTime nextDbCheck;
   std::list<kvalobs::kvTypes> typeList;
   std::list<kvalobs::kvParam> paramList;
   dnmi::db::Connection        *con_;

   /**
    * Cache the 'types' and 'param' table into
    * typeList and paramList. The tables is reread every
    * hour at hh:00. This to catch up with changes is these
    * tables.
    *
    * @return true if the loading of the tables succeeded and false
    * otherwise.
    */
   bool loadParamsAndTypes();

   /**
    * Is this an param that is defined to be received
    * with minute resolution. We use the name of the param
    * to decided if it is an param with minute resolution.
    *
    * All params with minute resolution has the following form.
    * P_0N, where P is the paramname and N is minute resolution specifier.
    *
    * ex.
    *    RR_01, precipitation with minute resolution.
    *
    * @param typeid_ The typeid.
    * @param paramid The paramid.
    * @return True if the paramid and typeid has minute
    * resolution.
    */
   bool paramIsMinute( int typeid_, int paramid );

   bool isMissingMessage( const std::list<kvalobs::kvData> &datalist )const;
   void removeFromList( const kvalobs::kvData &data, std::list<kvalobs::kvData> &dataList )const;



public:
  PreProcessMissingData();
  ~PreProcessMissingData();

  void flagDataNotInObsPgm(  const std::list<kvalobs::kvObsPgm> &obsPgm,
                             std::list<kvalobs::kvData> &data,
                             std::list<kvalobs::kvData> &wildObs );

  const char* jobName() const {return "MissingData";}

  void doJob(long                 stationId,
	     long                 typeId,
	     const miutil::miTime &obstime,
	     dnmi::db::Connection &con);

};

#endif
