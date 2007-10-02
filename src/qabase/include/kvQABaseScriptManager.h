/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseScriptManager.h,v 1.5.2.2 2007/09/27 09:02:38 paule Exp $                                                       

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
#ifndef _kvQABaseScriptManager_h
#define _kvQABaseScriptManager_h


/*
  Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
  at Mon Jun 17 16:12:50 2002
*/

#include "kvQABaseDBConnection.h"
#include "kvQABaseTypes.h"
#include <kvalobs/kvAlgorithms.h>
#include <string>
#include <vector>
#include <map>

/**
   \brief QABase perlscript-manager

   - maintains list of perlscripts with name and signature
*/

class kvQABaseScriptManager {
private:
  bool algo_selected;                ///< script selected
  kvalobs::kvAlgorithms algo;        ///< selected algorithm
  kvQABase::script_var variables[4]; ///< for each datasource
  kvQABaseDBConnection& dbcon_;      ///< Database connection

public:
  kvQABaseScriptManager(kvQABaseDBConnection& dbcon);
  ~kvQABaseScriptManager();

  /** Prepare a check algorithm (perl script)
      from checkname, arguments and stationid */
  bool findAlgo(const std::string name,
		const std::string argu,
		const int sid, bool& sig_ok);

  /// return prepared script
  bool getScript(std::string& s) const;
  /// name of prepared algorithm
  bool getAlgoName(std::string& name) const;
  /// return prepared script_var variable for one data_source
  bool getVariables(const kvQABase::data_source source,
		    kvQABase::script_var& vars) const;
  /// make Perl code from script_var variables
  bool makePerlVariables(kvQABase::script_var& vars,
			 std::string& varstring) const;
  
  void clear();
};

#endif
