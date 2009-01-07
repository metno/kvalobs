#ifndef _kvQABaseTypes_h
#define _kvQABaseTypes_h


/* Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
   at Wed Jun  5 12:48:03 2002 */

#include <kvalobs/kvDataFlag.h>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <kvalobs/kvObsPgm.h>

namespace kvQABase
{

  const std::string missing_value = "-32767";

  /**
    Sources for parameters in checks
  */
  enum data_source {
    obs_data = 0,
    refobs_data = 1,
    model_data = 2,
    meta_data = 3
  };


  /*
    Structures for meteorological and meta-data needed for
    one algorithm/check. Data ordered by:
    source (obs,refobs,model,meta)
           \
    parameter (TA, etc)
             \
       position
               \
          time/values
  */


  /**
    \brief parametervalue, -missingstatus and -controlflag
  */
  struct par_values
  {
    par_values() : value( missing_value ), status( kvQCFlagTypes::status_orig_and_corr_missing ) {}
    par_values( const std::string & val_, int status_, kvalobs::kvControlInfo & ci )
    : value( val_ ), status( status_ ), cinfo( ci ) {}
    std::string value;            ///< data-value
    int status;                   ///< kvQCFlagTypes::missing_status
    kvalobs::kvControlInfo cinfo; ///< control data-flag
  };

  /**
    \brief script parameter
  */
  struct script_par
  {
    std::string signature;        ///< varname in script
    std::string name;             ///< official parametername
    int paramid;                  ///< official parameterid
    int sensor;                   ///< sensor
    int level;                    ///< level
    int typeID;                   ///< typeID
    bool normal;                  ///< normal parameter (data or model_data)
    /** timeseries (timeoffset/value hash) hashed by stationid */
    std::map<int, std::map<int, par_values> > values;
  };

  /**
    \brief script variables from one source
  */
  struct script_var
  {
    int dsource;                  ///< data_source type
    std::string source;           ///< sourcename of data (obs,refobs,model,meta)
    int timestart;                ///< starttime in minutes from obstime
    int timestop;                 ///< stoptime in minutes from obstime
    bool missing_data;            ///< any missing data for this source
    bool allnormal;               ///< only normal variables in pars
    std::vector<int> allpos;      ///< all station-ids
    std::vector<int> alltimes;    ///< all timeoffsets for source
    std::vector<script_par> pars; ///< each parameter

    void clear()
    {
      allpos.clear();
      alltimes.clear();
      pars.clear();
    }
  };
};

struct lt_kvObsPgm
{
  bool operator() ( const kvalobs::kvObsPgm & a, const kvalobs::kvObsPgm & b ) const
  {
    if ( a.stationID() != b.stationID() )
      return a.stationID() < b.stationID();
    if ( a.paramID() != b.paramID() )
      return a.paramID() < b.paramID();
    if ( a.level() != b.level() )
      return a.level() < b.level();
    return a.typeID() < b.typeID();
  }
};

typedef std::set
  <kvalobs::kvObsPgm, lt_kvObsPgm> kvObsPgmList;


#endif
