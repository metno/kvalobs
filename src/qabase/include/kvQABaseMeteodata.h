/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseMeteodata.h,v 1.15.2.2 2007/09/27 09:02:38 paule Exp $                                                       

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
#include <kvalobs/kvStationInfo.h>

/**
  \brief QABase observation/model-data manager

  - get meteorological data from kvalobs db
  - output: as valid perl-text (perl-variables)
  - input: changes to data and/or flags

*/

class kvQABaseMeteodata {
protected:
  
  /// helper-struct for updateParameters()
  struct obs_keys{
    int stationID;       // station-id
    miutil::miTime time; // obstime
    int typeID;          // type-id
    int paridx;          // parameter-index
  };

  kvQABaseDBConnection& dbcon_; ///< Database connection

  /// checks performed for:
  kvalobs::kvStationInfo& stationinfo_;

  // current check has attributes:
  std::string qcx_;            ///< check-id
  std::string medium_qcx_;     ///< medium classification of check
  int language_;               ///< algorithm-type
  
  /// current check-variables
  kvQABase::script_var obs_vars, refobs_vars, model_vars;

  /// observations by station-id and time
  std::map<int, std::map<miutil::miTime,
    kvQABaseDBConnection::obs_data> > obsdata;
  /// text_data by station-id and time
  std::map<int, std::map<miutil::miTime,
    kvQABaseDBConnection::text_data> > textdata;
  /// modeldata by station-id and time
  std::map<int, std::map<miutil::miTime,
    kvQABaseDBConnection::model_data> > modeldata;
  
  /// requested data max/min times
  std::map<int, miutil::miTime> minObsTime_;
  std::map<int, miutil::miTime> maxObsTime_;
  std::map<int, miutil::miTime> minTextTime_;
  std::map<int, miutil::miTime> maxTextTime_;
  std::map<int, miutil::miTime> minModelTime_;
  std::map<int, miutil::miTime> maxModelTime_;

  /// load observation data for one station, several timesteps
  bool loadObsData(const int sid, const int tstart, const int tstop);
  /// load text data for one station, several timesteps
  bool loadTextData(const int sid, const int tstart, const int tstop);
  /// load model data for one station, several timesteps
  bool loadModelData(const int sid, const int tstart, const int tstop);

  /** fill observation var-structure for one source */
  bool fillObsVariables(kvQABase::script_var& vars);
  /** fill model var-structure for one source */
  bool fillModelVariables(kvQABase::script_var& vars);

public:
  kvQABaseMeteodata(kvQABaseDBConnection& dbcon,
		    kvalobs::kvStationInfo& stationinfo);

  /**
    PERL output
    return data for one parameter-check
  */
  bool data_asPerl(const std::string qcx,            // check-id
		   const std::string medium_qcx,     // medium class. of check
		   const int language,               // algorithm type
		   const kvQABaseScriptManager& sman,// script manager
		   const std::list<kvalobs::kvObsPgm>& oplist, // obs_pgm
		   bool& skipcheck,                  // skip this check (ret)
		   std::string& data);               // return perl-data

  /**
    - Update parameters with return-variables from check.
    - Finally save parameters to DB
  */
  bool updateParameters(std::map<std::string, double>& params);

  /// clear data
  void clear();
};

#endif
