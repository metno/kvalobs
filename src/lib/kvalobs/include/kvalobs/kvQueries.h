/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQueries.h,v 1.1.2.4 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef _kvQueries_h
#define _kvQueries_h

#include <puTools/miString>
#include <puTools/miTime>
#include <sstream>
#include <kvalobs/kvData.h>
#include <list>

/* Created by DNMI/FoU/PU: j.schulze@dnmi.no
   at Mon Aug 26 13:00:05 2002 */


/**
 * \brief This namespace contains common queries
 *        that is used at many places in kvalobs.
 */
namespace kvQueries {

  /**
   * \addtogroup kvqueries
   *
   * @{
   */

  /**
   * \brief select all rows from table \em checks matching
   *  stationid in slist, language=lan and valid fromtime
   */
  miutil::miString selectChecks(const std::list<int> slist,
				const int lan,
				const miutil::miTime& otime);

  /**
   * \brief select all rows from table \em station_param matching
   *  stationid in slist, otime in [fromday - today],
   *  qcx = qcx and valid fromtime
   */
    miutil::miString selectStationParam(const std::list<int> slist,
				      const miutil::miTime& otime,
				      const std::string& qcx);

    /**
     * \brief select all rows from table \em data matching
     *  obstime=otime
     */
  miutil::miString selectData(const miutil::miTime& otime);

  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, paramid=pid and obstime=otime
   */
  miutil::miString selectData(const int sid,
			      const int pid,
			      const miutil::miTime& otime);

  /**
   * \brief Select the \em data record that match this \em kvData,
   * ie. match what is speciefied in the unique index.
   *�(stationid, obstime, paramid, level, sensor and typeid)
   */
  miutil::miString selectData(const kvalobs::kvData &d);

  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, typeid=tid and obstime=otime
   */
  miutil::miString selectDataFromType(const int sid,
				      const int tid,
				      const miutil::miTime& otime);


  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, abs(typeid)=abs(tid) and obstime=otime
   */
  miutil::miString selectDataFromAbsType(const int sid,
				      const int tid,
				      const miutil::miTime& otime);


  /**
   * \brief select all rows from table \em data matching
   *  obstime in [stime - etime], order by ob
   */
  miutil::miString selectData(const miutil::miTime& stime,
			      const miutil::miTime& etime,
			      const miutil::miString& ob);

  /**
   * \brief select all rows from table \em data matching
   *  tbtime in [stime - etime], order by ob.
   */
  miutil::miString selectDataByTabletime(const miutil::miTime& stime,
					 const miutil::miTime& etime,
					 const miutil::miString& ob);

  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, and obstime in [stime - etime]
   */
  miutil::miString selectData(const int sid,
			      const miutil::miTime& stime,
			      const miutil::miTime& etime);



  /**
   * \brief select all rows from table \em text_data matching
   *  stationid=sid, and obstime in [stime - etime]
   */
  miutil::miString selectTextData(const int sid,
				  const miutil::miTime& stime,
				  const miutil::miTime& etime);


  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, and tbtime in [stime - etime]
   */
  miutil::miString selectDataByTbtime(const int sid,
				      const miutil::miTime& stime,
				      const miutil::miTime& etime);

  /**
   * \brief select all rows from table \em data order by ob
   */
  miutil::miString selectData(const miutil::miString& ob);

  /**
   * \briefselect all rows from table \em data matching
   *  obstime in [stime - etime]
   */
  miutil::miString selectData(const miutil::miTime& stime,
			      const miutil::miTime& etime);

  miutil::miString selectReferenceStation(long stationid, long paramsetid);

  /**
   * \brief select a few stations from table \em data matching
   *  obstime in [stime - etime]
   */
  miutil::miString selectDataStat(const miutil::miTime& stime,
				  const miutil::miTime& etime,
				  const miutil::miString& statList);

  /**
   * \brief select all rows from table \em param order by ob
   */
  miutil::miString selectParam(const miutil::miString& ob);

  /**
   * \brief select all rows from table \em model_data matching
   *  stationid=sid, and obstime in [stime - etime]
   */
  miutil::miString selectModelData(const int sid,
				   const miutil::miTime& stime,
				   const miutil::miTime& etime);

  /**
   * \brief Select a station from the \em station table based on stationid.
   */
  miutil::miString selectStationByStationId(long stationid);

  /**
   * \brief Select a station from the \em station table based on wmono.
   */
  miutil::miString selectStationByWmonr(long wmonr);

  /**
   * \brief Select a station from the \em station table based on nationalid.
   */
  miutil::miString selectStationByNationalnr(long nationalnr);

  /**
   * \brief Select a station from the \em station table based on ICAOID.
   */
  miutil::miString selectStationByIcaoId(long icaoid);

  /**
   * \brief Select a station from the \em station table based on call sign.
   */
  miutil::miString selectStationByCall_sign(const std::string &cs);


  miutil::miString selectStationOrdered();
  miutil::miString selectAllStations(const miutil::miString &orderby);

  /**
   * \brief Select all entries from \em obs_pgm matching typeid
   * and time
   */
 	miutil::miString
	selectObsPgmByTypeid(long tid,
	                     const miutil::miTime& otime); 


  /**
   * \brief Select all obs_pgm entries for the given station.
   */
  miutil::miString selectObsPgm( long stationid );
  
  /**
   * \brief Select all entries from \em obs_pgm matching stationid and time
   */
  miutil::miString selectObsPgm(long stationid,
				const miutil::miTime& otime);

  /**
   * \brief Select all entries from \em obs_pgm matching stationid, typeid
   * and time
   */
  miutil::miString selectObsPgm(long stationid,
					   long tid,
					   const miutil::miTime& otime);


  /**
   * \brief Select all entries from \em obs_pgm matching time
   */
  miutil::miString selectObsPgm(const miutil::miTime& otime);

  /**
   * \brief Select all \em KeyValues matching package and key
   */
  miutil::miString selectKeyValues(const miutil::miString& package,
				   const miutil::miString& key);


  /**
   * \brief Look up stationid and typeid in the table \em generated_types.
   *
   * The table \em generated_types contains observations that is generated
   * from other observations in the database. This may be SYNOP that is
   * generated from automatic wheater stations data.
   *
   * \param stationid The stations id.
   * \param typeid_   The typeid to test
   * \return A query to be used in gate::select.
   */
  miutil::miString selectIsGenerated(long stationid, int typeid_);

  /** @} */
};

#endif
