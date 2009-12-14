/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseMeteodata.h,v 1.1.2.4 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef _kvQABaseMeteodata_h
#define _kvQABaseMeteodata_h

/*
  Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
  at Wed May  8 08:13:02 2002
*/

#include <string>
#include <map>
#include <puTools/miTime>
#include "kvQABaseScriptManager.h"
#include "kvQABaseDBConnection.h"
#include "kvalobscache.h"
#include "kvQABaseTypes.h"
#include <kvalobs/kvStationInfo.h>

/**
 * \brief QABase observation/model-data manager
 *
 * - get meteorological data from kvalobs db
 * - output: as valid perl-text (perl-variables)
 * - input: changes to data and/or flags
 */
class kvQABaseMeteodata
{
    kvQABaseDBConnection & dbcon_; ///< Database connection

    /// checks performed for:
    const kvalobs::kvStationInfo& stationinfo_;

    /// current check-variables
    kvQABase::script_var obs_vars, refobs_vars, model_vars;

    // Allows access to obsdata, so method shouldProcess may cache the data it needs to look up:
//    friend class CheckRunner;
    
    /// observations by station-id and time
public:
//    struct cached_kvData_w_time
//    {
//      typedef std::vector<kvalobs::kvData> Container;
//      Container data;
//    };
    typedef std::vector<kvalobs::kvData> obs_data;
    typedef std::map<miutil::miTime, obs_data> DataFromTime;
private:
    typedef std::map<int, DataFromTime> DataFromStTime;
    DataFromStTime obsdata;
    /// text_data by station-id and time
    typedef std::map<miutil::miTime, kvQABaseDBConnection::text_data> TDataFromTime;
    typedef std::map<int, TDataFromTime> TDataFromStTime;
    TDataFromStTime textdata;
    /// modeldata by station-id and time
    typedef std::map<miutil::miTime, kvQABaseDBConnection::model_data> MDataFromTime;
    typedef std::map<int, MDataFromTime> MDataFromStTime;
    MDataFromStTime modeldata;

    /// requested data max/min times
    typedef std::map<int, miutil::miTime> TimeFromStation;
    TimeFromStation  minObsTime_;
    TimeFromStation  maxObsTime_;
    TimeFromStation  minTextTime_;
    TimeFromStation  maxTextTime_;
    TimeFromStation  minModelTime_;
    TimeFromStation  maxModelTime_;

    /// load observation data for one station, several timesteps
    bool loadObsData( const int sid, const int tstart, const int tstop );
    /// load text data for one station, several timesteps
    bool loadTextData( const int sid, const int tstart, const int tstop );
    /// load model data for one station, several timesteps
    bool loadModelData( const int sid, const int tstart, const int tstop );

    /** fill observation var-structure for one source */
    void fillObsVariables( kvQABase::script_var& vars );
    /** fill model var-structure for one source */
    void fillModelVariables( kvQABase::script_var& vars );

  public:
    kvQABaseMeteodata( kvQABaseDBConnection& dbcon,
                       const kvalobs::kvStationInfo& stationinfo );

    /**
     * Thrown by data_asPerl to signify that a check should not be run
     */
    struct SkipCheck : std::exception {};
    
    /**
     * PERL output
     *
     * @throws SkipCheck If the function decides that checks should not be run 
     * on this data.
     * @throws std::exception on error in data lookup 
     * @return data for one parameter-check
    */
    std::string data_asPerl( const kvQABaseScriptManager& sman,     // script manager
                      const kvObsPgmList & oplist );      // obs_pgm

#ifdef USE_PYTHON
	/**
     * RAW output
     *
     * @throws SkipCheck If the function decides that checks should not be run 
     * on this data.
     * @throws std::exception on error in data lookup 
     * @return data for one parameter-check in kvQABase::script_var format
    */
	
	bool data_asRaw( const kvQABaseScriptManager& sman,                    // active Script-Manager
                                const kvObsPgmList & oplist,
								std::list<kvQABase::script_var>& data);                     // obs_pgm
#endif

    typedef std::map<std::string, double> ScriptReturnType;

    const DataFromTime & getData()
    {
      return preloadData();
    }
    
    void resetFlags(const kvalobs::kvStationInfo & si);
    
    /**
      - Update parameters with return-variables from check.
      - Finally save parameters to DB
    */
    bool updateParameters( const ScriptReturnType & params, const kvalobs::kvChecks & check );

	void flushCache();

  private:
    DataFromTime & preloadData(const kvalobs::kvStationInfo & si);
    DataFromTime & preloadData() {
      return preloadData(stationinfo_);
    }
    
    /**
     * @param param a name-value pair from check.
     * @param newdata write changes here.
     */
    void setFlag( ScriptReturnType::const_iterator param, const ScriptReturnType & scriptRet,  kvalobs::kvData & newdata, const kvalobs::kvChecks & check );
    void setMissing( ScriptReturnType::const_iterator param, kvalobs::kvData & newdata );
    void setCorrected( float newVal, kvalobs::kvData & data );

    void updateUseInfo( kvalobs::kvData & d );

    /// helper-struct for updateParameters()
    struct obs_keys
    {
      int stationID;       // station-id
      miutil::miTime time; // obstime
      int typeID;          // type-id
      int paridx;          // parameter-index
    };
    typedef std::map<std::string, obs_keys> ObsKeys;

    kvalobs::kvData & getModifiedData( ObsKeys & updated_obs, ScriptReturnType::const_iterator pp, const std::string & type );
    void updateSingleParam( ObsKeys & updated_obs, const ScriptReturnType::const_iterator pp, const ScriptReturnType & params, const kvalobs::kvChecks & check );

    void saveInDb( const ObsKeys & k );

    KvalobsCache saveCache_;

    static const miutil::miString type_flag;
    static const miutil::miString type_corrected;
    static const miutil::miString type_missing;
    static const miutil::miString type_subcheck;
};

#endif
