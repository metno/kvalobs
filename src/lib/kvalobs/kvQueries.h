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

#include <puTools/miTime.h>
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
  std::string selectChecks(const std::list<int> slist,
				const int lan,
				const miutil::miTime& otime);

  /**
   * \brief select all rows from table \em station_param matching
   *  stationid in slist, otime in [fromday - today],
   *  qcx = qcx and valid fromtime
   */
    std::string selectStationParam(const std::list<int> slist,
				      const miutil::miTime& otime,
				      const std::string& qcx);

    /**
     * \brief select all rows from table \em data matching
     *  obstime=otime
     */
  std::string selectData(const miutil::miTime& otime);

  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, paramid=pid and obstime=otime
   */
  std::string selectData(const int sid,
			      const int pid,
			      const miutil::miTime& otime);

  /**
   * \brief Select the \em data record that match this \em kvData,
   * ie. match what is speciefied in the unique index.
   *�(stationid, obstime, paramid, level, sensor and typeid)
   */
  std::string selectData(const kvalobs::kvData &d);

  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, typeid=tid and obstime=otime
   */
  std::string selectDataFromType(const int sid,
				      const int tid,
				      const miutil::miTime& otime);


  /**
   * \brief select all rows from table \em text_data matching
   *  stationid=sid, typeid=tid and obstime=otime.
   * 
   * The result is sorted by paramid.
   */
  std::string  selectTextDataFromType(const int sid,
		  							                const int tid,
	                                        const miutil::miTime& otime);

  
  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, abs(typeid)=abs(tid) and obstime=otime
   */
  std::string selectDataFromAbsType(const int sid,
				      const int tid,
				      const miutil::miTime& otime);


  /**
   * \brief select all rows from table \em data matching
   *  obstime in [stime - etime], order by ob
   */
  std::string selectData(const miutil::miTime& stime,
			      const miutil::miTime& etime,
			      const std::string& ob);

  /**
   * \brief select all rows from table \em data matching
   *  tbtime in [stime - etime], order by ob.
   */
  std::string selectDataByTabletime(const miutil::miTime& stime,
					 const miutil::miTime& etime,
					 const std::string& ob);

  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, and obstime in [stime - etime]
   */
  std::string selectData(const int sid,
			      const miutil::miTime& stime,
			      const miutil::miTime& etime);



  /**
   * \brief select all rows from table \em text_data matching
   *  stationid=sid, and obstime in [stime - etime]
   */
  std::string selectTextData(const int sid,
				  const miutil::miTime& stime,
				  const miutil::miTime& etime);


  /**
   * \brief select all rows from table \em data matching
   *  stationid=sid, and tbtime in [stime - etime]
   */
  std::string selectDataByTbtime(const int sid,
				      const miutil::miTime& stime,
				      const miutil::miTime& etime);

  /**
   * \brief select all rows from table \em data order by ob
   */
  std::string selectData(const std::string& ob);

  /**
   * \briefselect all rows from table \em data matching
   *  obstime in [stime - etime]
   */
  std::string selectData(const miutil::miTime& stime,
			      const miutil::miTime& etime);

  std::string selectReferenceStation(long stationid, long paramsetid);

  /**
   * \brief select a few stations from table \em data matching
   *  obstime in [stime - etime]
   */
  std::string selectDataStat(const miutil::miTime& stime,
				  const miutil::miTime& etime,
				  const std::string& statList);

  /**
   * \brief select all rows from table \em param order by ob
   */
  std::string selectParam(const std::string& ob);

  /**
   * \brief select all rows from table \em model_data matching
   *  stationid=sid, and obstime in [stime - etime]
   */
  std::string selectModelData(const int sid,
				   const miutil::miTime& stime,
				   const miutil::miTime& etime);

  /**
   * \brief Select a station from the \em station table based on stationid.
   */
  std::string selectStationByStationId(long stationid);

  /**
   * \brief Select a station from the \em station table based on wmono.
   */
  std::string selectStationByWmonr(long wmonr);

  /**
   * \brief Select a station from the \em station table based on nationalid.
   */
  std::string selectStationByNationalnr(long nationalnr);

  /**
   * \brief Select a station from the \em station table based on ICAOID.
   */
  std::string selectStationByIcaoId(long icaoid);

  /**
   * \brief Select a station from the \em station table based on call sign.
   */
  std::string selectStationByCall_sign(const std::string &cs);


  std::string selectStationOrdered();
  std::string selectAllStations(const std::string &orderby);
  std::string selectStationsByRange( long from, long to, bool order);

  /**
   * \brief Select all entries from \em obs_pgm matching typeid
   * and time
   */
 	std::string
	selectObsPgmByTypeid(long tid,
	                     const miutil::miTime& otime); 


  /**
   * \brief Select all obs_pgm entries for the given station.
   */
  std::string selectObsPgm( long stationid );
  
  /**
   * \brief Select all entries from \em obs_pgm matching stationid and time
   */
  std::string selectObsPgm(long stationid,
				const miutil::miTime& otime);

  /**
   * \brief Select all entries from \em obs_pgm matching stationid, typeid
   * and time
   */
  std::string selectObsPgm(long stationid,
					   long tid,
					   const miutil::miTime& otime);


  /**
   * \brief Select all entries from \em obs_pgm matching time
   */
  std::string selectObsPgm(const miutil::miTime& otime);

  /**
   * \brief Select all \em KeyValues matching package and key
   */
  std::string selectKeyValues(const std::string& package,
				   const std::string& key);


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
  std::string selectIsGenerated(long stationid, int typeid_);

  /** EGLITIS (for Qc2)
   * \brief select all rows from table \em data matching
   *  stationid in list, paramid=pid and obstime in [stime - etime]
   */
  std::string selectData(const std::list<int> slist,
                              const int pid,
                              const miutil::miTime& stime,
                              const miutil::miTime& etime);
  /** EGLITIS (for Qc2)
   * \brief select all rows from table \em data matching
   *  stationid in list, paramid=pid and obstime in [stime - etime]
   */
  std::string selectData(const std::list<int> slist,
                              const int pid,
                              const int tid,
                              const miutil::miTime& stime,
                              const miutil::miTime& etime);

  /** EGLITIS (for Qc2)
   * \brief select all rows from table \em data matching
   *  paramid=pid, typeid=tid, obstime in [stime - etime]
   *  and controlinfo=controlString
   */

   std::string selectData(const int pid,
                               const int tid, 
                               const miutil::miTime& stime, 
                               const miutil::miTime& etime, 
                               const std::string& controlString);

  /** EGLITIS (for Qc2)
   * \brief select all rows from table \em data matching
   *  a single stationid, paramid=pid and obstime in [stime - etime]
   */
   std::string selectData(const int stid,
                               const int pid, 
                               const int tid, 
                               const miutil::miTime& stime, 
                               const miutil::miTime& etime);
  /** EGLITIS (for Qc2)
   * \brief select all rows from table \em data matching
   *  a single stationid, paramid=pid and obstime in [stime - etime]
   */
   std::string selectData(const int stid,
                               const int pid,
                               const miutil::miTime& stime,
                               const miutil::miTime& etime);

  /** EGLITIS (for Qc2)
   * \brief A query to pick out missing float values especially
   *  for Qc2 tests, for all stations at one particular time
   */
   std::string selectMissingData(const float value,
                                      const int pid, 
                                      const miutil::miTime& Ptime);

  /** EGLITIS (for Qc2)
   * \brief A query to pick out missing float values especially
   *  for Qc2 tests, for all stations at one particular time
   */
   std::string selectMissingData(const float value,
                                      const int pid, 
                                      const int tid, 
                                      const miutil::miTime& Ptime);

  /** @} */
};

#endif
