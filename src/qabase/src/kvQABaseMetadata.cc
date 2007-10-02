/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseMetadata.cc,v 1.18.6.2 2007/09/27 09:02:38 paule Exp $                                                       

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

#include "../include/kvQABaseMetadata.h"
#include "../include/kvQABaseTypes.h"
#include "../include/kvQABaseDBConnection.h"
#include <kvalobs/kvQCFlagTypes.h>

#include <puTools/miString>
#include <list>

#include <milog/milog.h>

kvQABaseMetadata::kvQABaseMetadata(kvQABaseDBConnection& dbcon)
  : dbcon_(dbcon)
{
}

// return metadata for a check
bool
kvQABaseMetadata::data_asPerl(const int                    sid,  // station-id
			      const std::string            ctype,// check-type
			      const miutil::miTime&        otime,// observationtime
			      const kvQABaseScriptManager& sman, // script-manager
			      std::string&                 data) // return data here
{
  data.clear();   // clear final perl-string

  bool result;
  kvQABase::script_var vars;

  // get requirements for script
  result= sman.getVariables(kvQABase::meta_data, vars);
  if (!result) {
    IDLOGERROR("html",
	       "kvQABaseMetadata::data_asPerl ERROR sman.getVariables failed."
	       << std::endl);
    return false;
  }

  if (vars.pars.size() == 0) return true; // no requirements, return

  vars.alltimes.clear();
  vars.alltimes.push_back(0); // only one timestep for meta-data

  std::list<kvMetadataTable> tables; // metadata-tables

  // NBNBNB should we get a list of times to get metadata for?????

  int npos= vars.allpos.size();

  for (int i=0; i<npos; i++){
    tables.clear(); // clear metadata-tables
    int ipos= vars.allpos[i];
    
    // fetch metadata from DB
    result= dbcon_.getMetadata(ipos,otime,ctype,tables);
    if (!result) {
      IDLOGERROR("html",
		 "kvQABaseMetadata::data_asPerl ERROR dbcon_.getMetadata failed."
		 << std::endl);
      return false;
    }
      
    // put metadata into script_var structure
    int n= vars.pars.size();

    for (int j=0; j<n; j++){
      std::string vname= vars.pars[j].name;
      std::string value= kvQABase::missing_value;
      int status= kvQCFlagTypes::status_ok;
      bool found = false;

      std::list<kvMetadataTable>::iterator mp= tables.begin();
      for (; mp != tables.end(); mp++){
	if (mp->findEntry(vname, value)){
	  found= true;	    
	  break;
	}
      }

      if (!found){
	IDLOGERROR("html", "Missing METADATA parameter:" << vname
		   << " for station:" << ipos << std::endl);
	status= kvQCFlagTypes::status_original_missing;
	vars.missing_data= true;
	// should we really....
	return false;
      }

      // add data value
      vars.pars[j].values[ipos][0].value=  value;
      vars.pars[j].values[ipos][0].status= status;
    }
  }

  // convert var_structure into Perl-code
  std::string varstring;

  result= sman.makePerlVariables(vars, varstring);

  if (!result) {
    IDLOGERROR("html",
	       "kvQABaseMetadata::data_asPerl ERROR sman.makePerlVariables failed."
	       << std::endl);
    return false;
  }

  // Pretty heading
  data+="\n";
  data+="#==========================================\n";
  data+="# MetaData\n";
  data+="#\n";

  data+= varstring;
  
  return true;
}
