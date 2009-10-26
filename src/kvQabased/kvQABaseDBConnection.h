/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseDBConnection.h,v 1.1.2.4 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef _kvQABaseDBConnection_h
#define _kvQABaseDBConnection_h

/*
  Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
   at Thu Jun 20 14:16:12 2002
*/



#include <string>
#include <list>
#include <map>
#include <puTools/miTime>

#include "kvMetadataTable.h"
#include "kvQABaseTypes.h"
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvAlgorithms.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvChecks.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvDbGate.h>

#include <kvalobs/kvDataOperations.h>
#include <set>

#include <kvdb/dbdrivermgr.h>

/**
 * \brief QABase Database Connection Gate
 */
class kvQABaseDBConnection
{
  public:
    /// all obs_data parameters from one timestep
//    typedef cached_data_w_time<kvalobs::kvData> obs_data; // moved to meteodata
    /// all text_data parameters from one timestep
    typedef std::vector<kvalobs::kvTextData> text_data;
    /// all model_data parameters from one timestep
    typedef std::vector<kvalobs::kvModelData> model_data;

  private:
    bool connection_ok;
    kvalobs::kvDbGate dbGate;
    static miutil::miTime active_timestamp;

    static std::map<std::string, kvalobs::kvAlgorithms> algolist;
    static long int algo_timestamp;      ///< last update of list
    bool updateAlgoList();

    static std::map<std::string, kvalobs::kvParam> parameterlist;
    static long int parameter_timestamp; ///< last update of list
    bool updateParameterList();

    static long int qcxinfo_timestamp; ///< last update of list
    bool updateQcxList();

#ifdef USE_PYTHON
	static int script_language;
#endif

  public:
#ifdef USE_PYTHON
    kvQABaseDBConnection( dnmi::db::Connection *con, const int & script_language_ );
#else
    kvQABaseDBConnection( dnmi::db::Connection *con );
#endif
    bool dbOk() const
    {
      return connection_ok;
    }

    /**
      reread static database-tables
      currently: algorithms and param
    */
    void updateStatics();

    /// return algorithm by name
    bool getAlgo( const std::string name, kvalobs::kvAlgorithms& algo );

    /// return parameter-struct by name or id
    bool getParameter( const std::string& name, kvalobs::kvParam& p );
    bool getParameter( const int id, kvalobs::kvParam& p );
    /// return parameter-id by name
    bool getParameterId( const std::string& name, int& id );


    /// get observation program for one station
    bool getObsPgm( const int sid,
                    const miutil::miTime& otime,
                    //                     std::list<kvalobs::kvObsPgm>& oplist );
                    kvObsPgmList & oplist );

    /// get all checks from db matching stationid and obstime
    bool getChecks( const int sid,
                    const miutil::miTime& otime,
                    std::list<kvalobs::kvChecks>& checklist );

    /// get all metadata from db matching stationid, parameterid and obstime
    bool getMetadata( const int sid,
                      const miutil::miTime& otime,
                      const std::string& qcx,
                      std::list<kvMetadataTable>& tables );

    /// get observation from db matching stationid, parameterid and obstime
//    bool getObservation( const int sid,                   // station
//                         const miutil::miTime& otime,     // obs-time
//                         const int pid,                   // parameter
//                         kvalobs::kvData& param );

    template <typename Iterator>
    bool setObservation( Iterator begin, Iterator end )
    {
    try
    {
      connection_->beginTransaction();
      for ( ; begin != end; ++ begin )
        if ( ! setObservation( * begin ) )
        {
          connection_->rollBack();
          return false;
        }
      connection_->endTransaction();
      return true;
    }
    catch( std::exception & e )
    {
      std::cout << e.what() << std::endl;
      throw;
    }
    }

    /// write observation to db
    bool setObservation( const kvalobs::kvData& param );

    /** get all observations from db matching stationid and obstime
        between stime and etime
    */
    bool getObservations( const int sid,                  // station
                          const miutil::miTime& stime,    // start-time
                          const miutil::miTime& etime,    // end-time
                          std::list<kvalobs::kvData> & data );

    /** get all text_data from db matching stationid and obstime
        between stime and etime
    */
    bool getTextData( const int sid,                  // station
                      const miutil::miTime& stime,    // start-time
                      const miutil::miTime& etime,    // end-time
                      std::map<miutil::miTime, text_data>& data );

    /** get all modeldata from db matching stationid and validtime
        between stime and etime
    */
    bool getModelData( const int sid,                  // station
                       const miutil::miTime& stime,    // start-time
                       const miutil::miTime& etime,    // end-time
                       std::map<miutil::miTime, model_data>& data );

    /// Set timestamp in Key/Value table
    bool saveTimestamp( const miutil::miTime& time );

  private:
    dnmi::db::Connection * connection_;
};

#endif
