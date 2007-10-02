/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvModelData.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef __kvModelData_h__
#define __kvModelData_h__

#include <kvalobs/kvDbBase.h>

/*
 * Created by DNMI/IT: borge.moe@met.no
 * at Tue Aug 28 07:53:16 2002 
 */

namespace kvalobs{

  /**
   * \addtogroup  dbinterface
   *
   * @{
   */  


  /**
   * \brief Interface to the table model_data in the kvalobs database.
   */

class kvModelData : public kvDbBase {
 private:
  int              stationid_;
  miutil::miTime   obstime_;
  int              paramid_;
  int              level_;
  int              modelid_;
  float            original_;
  

 public:
  kvModelData() {}
  kvModelData(const dnmi::db::DRow &r){ set(r);} 
  kvModelData(int stationid, 
	      const miutil::miTime &obstime,    
	      int   paramid,    
	      int   level,
	      int   modelid,
	      float original)
 
    {
      set(stationid, obstime, paramid, level, modelid, original);
    }

  bool set(int stationid, 
	   const miutil::miTime &obstime,    
	   int   paramid,    
	   int   level,
           int   modelid,     
	   float original);


  bool set(const dnmi::db::DRow&);
  char* tableName() const {return "model_data";}
  miutil::miString toSend()   const;
  miutil::miString uniqueKey()const;

  int              stationID() const { return stationid_;} 
  miutil::miTime   obstime()   const { return obstime_;}
  int              paramID()   const { return paramid_;}
  int              level()     const { return level_;}
  int              modelID()   const { return modelid_;}
  float            original()  const { return original_;}
};


/** @} */
};
#endif
