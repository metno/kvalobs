/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseMetadata.h,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef _kvQABaseMetadata_h
#define _kvQABaseMetadata_h


/*
  Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
  at Wed May  8 08:13:34 2002
*/

/**
   \brief QABase metadata manager

   - get metadata from kvalobs db
   - output: as valid perl-text (perl-variables)

*/

#include <string>
#include <map>
#include <puTools/miTime>
#include "kvQABaseDBConnection.h"
#include "kvQABaseScriptManager.h"

class kvQABaseMetadata {
private:
  kvQABaseDBConnection& dbcon_; ///< Database connection

public:
  kvQABaseMetadata(kvQABaseDBConnection& dbcon);

  /// return metadata as Perl code variables
  bool data_asPerl(const int sid,                    // station-id
		   const std::string ctype,          // check-type
		   const miutil::miTime& otime,      // observationtime
		   const kvQABaseScriptManager& sman,// script-manager
		   std::string& data);               // return data here

#ifdef USE_PYTHON
  /// return metadata as raw data, language independent
  bool data_asRaw(const int sid,                    // station-id
		   const std::string ctype,          // check-type
		   const miutil::miTime& otime,      // observationtime
		   const kvQABaseScriptManager& sman,// script-manager
		   std::list<kvQABase::script_var>& data);               // return data here
#endif

};

#endif

