/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: MissingObsCheck.h,v 1.1.2.2 2007/09/27 09:02:34 paule Exp $                                                       

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
#ifndef __MISSINGOBSCHECK_H__
#define __MISSINGOBSCHECK_H__

#include <boost/function.hpp>
#include <dnmithread/CommandQue.h>
#include <kvdb/kvdb.h>
#include <kvalobs/kvStationInfoCommand.h>
#include "GenCacheElem.h"

typedef boost::function<bool()> ShutdownHook;

class MissingObsCheck {
  dnmi::thread::CommandQue &outputQue;
  dnmi::db::Connection *con;
  GenCache &genCache;
  ShutdownHook shutdown;

  void postCommandToQue(kvalobs::StationInfoCommand *cmd);

  /**
   * \brief Find the obstime we shall consider as missing.
   *
   * Every typeid has lateobs value who tells how long we shall
   * wait before we consider an observation of this typeid as 
   * missing.
   * 
   * A obstime and a checktime is computed from the current
   * time (runTime) and  the lateobs value.
   *  
   * The funnction returns true if we shall search for missing
   * observations for this obstime and any obstimes that is earlyer 
   * than this obstime. If the function return false start to seach
   * for missing observations from obstime.hour(-1), ie one hour before
   * this obstime. 
   * 
   * \param[in]  runTime The time the missing rutine is run.
   * \param[out] obstime The computed obstime.
   * \param[in]  lateobsInMinute The lateobs value for an typeid.
   * \return true if we shall check for missing observations for the
   *              computed obstime and false otherwise.  
   */
  bool checkObstime(const boost::posix_time::ptime &runTime,
                    boost::posix_time::ptime &obstime,
                    const long lateobsInMinute);
 public:
  MissingObsCheck(dnmi::db::Connection &con,
                  dnmi::thread::CommandQue &outputQue, GenCache *genCache,
                  ShutdownHook shutdown = 0);

  ~MissingObsCheck();

  void findMissingData(const boost::posix_time::ptime& runtime,
                       const boost::posix_time::ptime& lastSearchForMissing);

};

#endif /*__MISSINGOBSCHECK_H__*/
