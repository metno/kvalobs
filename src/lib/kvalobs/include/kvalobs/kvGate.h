/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvGate.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef _kvGate_h
#define _kvGate_h

#include <kvalobs/kvAlgorithms.h>
#include <kvalobs/kvChecks.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvDataFlag.h>
#include <kvalobs/kvDbBase.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvNotData.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvRejectdecode.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvTimecontrol.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/kvVarStation.h>

#include <kvalobs/kvQueries.h>
#include <db/db.h>

#include <list>

/* Created by met.no/FoU/PU: j.schulze@met.no
   at Wed Aug 28 13:49:02 2002 */

namespace kvalobs {
  
  class kvGate  {
  private:
    
    dnmi::db::Connection &con;
    
  public:
    kvGate() {}
    
    template<class T>
    bool select(std::list<T>& li , miutils::miString q="")
    {
      li.clear();
      std::string query = T::selectAllQuery() + q;
      
      Result *res = 0;
      
      try {
	res = con.execQuery(query);
	
	while (res->hasNext()) {
	  DRow & row = res->next();
	  li.push_back(T(row));
	}
	delete res;
	return true;
      }
      catch(SQLException & ex) {
	delete res;
	CERR("EXCEPTION: select: " << ex.what() << endl);
      }
      catch(...) {
	delete res;
	CERR("EXCEPTION: select: Unknown exception!\n");
      }
  
      return false;
    }
    
    
  };

}

#endif







