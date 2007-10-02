/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvMetadataTable.h,v 1.1.2.2 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef _kvMetadataTable_h
#define _kvMetadataTable_h

/*
   Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
   at Fri Aug 30 16:07:08 2002
 */

/**
   \brief Metadata-structure 

   Metadata for one variable ordered as string-tables
   
   <pre>
   -------------------------
   <var_>
   X1;  X2;  X3; ....  XN
   V11; V12; V13; .... V1N
   V21; V22; V23; .... V2N
   .......................
   VM1; VM2; VM3; .... VMN
   -------------------------
   </pre>
*/

#include <string>
#include <vector>
#include <list>
#include <map>

class kvMetadataTable {
private:
  std::string var_; ///< variable
  std::map< std::string, std::vector<std::string> > value_; ///< data

  /**
     \brief metadata argument string 

     name/value pair found in argument to 'findEntry'
  */
  struct argus {
    std::string name;
    std::string value;
    argus();
    argus(std::string n, std::string v)
      : name(n), value(v) {}
  };


public:
  kvMetadataTable() {}
  
  /**
    return name of variable
  */
  std::string var() const {return var_;}
  
  /**
     Try to find metadata-entry matching name (and optional arguments)
     Examples:
              findEntry("MAX",data);
              findEntry("MIN(level=0)",data);

     if successful: return value in data
     return false if not found
  */
  bool findEntry(const std::string& name, std::string& data);
  
  /**
    Static memberfunction to produce lists of kvMetadataTable
    - input: name of parameter and metadata-string (typically from database)
    - identify variable-sections, and make one kvMetadataTable
      for each
    - pack each kvMetadataTable
    
    return false if bad source
  */
  static bool processString(const std::string& param,
			    const std::string& source,
			    std::list<kvMetadataTable>& lmt);
};

#endif
